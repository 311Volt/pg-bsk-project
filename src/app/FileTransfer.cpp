#include "FileTransfer.hpp"

#include "../crypto/Random.hpp"
#include <filesystem>
#include <chrono>
#include <type_traits>

#include "AppState.hpp"

#include <fmt/format.h>

constexpr size_t DEFAULT_BLOCK_SIZE = 65536;
constexpr size_t DEFAULT_NUM_WORKERS = 24;

namespace stdfs = std::filesystem;

FileTransfer::FileTransfer(size_t blockSize, AppState* app)
	: blockSize(blockSize),
	  fileSize(0),
	  awaitingAccept(false),
	  app(app)
{
	Random::Fill(&transferId, sizeof(transferId));
}

FileTransfer::~FileTransfer()
{

}

void FileTransfer::setOnAcceptFn(CallbackT fn)
{
	onAccept = fn;
}
void FileTransfer::setOnUpdateFn(CallbackT fn)
{
	onUpdate = fn;
}

bool FileTransfer::isDone()
{
	std::lock_guard<std::mutex> lock(mtx);
	return bytesCompleted() >= bytesTotal();
}

void FileTransfer::invoke(CallbackT cb)
{
	std::lock_guard<std::mutex> lock(mtx);
	if(cb) {
		cb();
	}
}

size_t FileTransfer::bytesTotal() const
{
	return fileSize;
}

size_t FileTransfer::getNumBlocks() const
{
	if(fileSize + blockSize == 0) {
		return 0;
	}
	return (fileSize + blockSize - 1) / blockSize;
}


FileTransferSend::FileTransferSend(const std::string_view filename, AppState* app)
	: FileTransfer(DEFAULT_BLOCK_SIZE, app),
	  exitFlag(false)
{
	awaitingAccept = true;
	inputPath = filename;
	inFile = std::ifstream(inputPath.c_str(), std::ios::binary);

	if(!inFile.good()) {
		throw std::runtime_error("cannot open " + inputPath);
	}

	fileSize = stdfs::file_size(inputPath);

	unsentBlocks.resize(getNumBlocks());
	for(size_t i=0; i<unsentBlocks.size(); i++) {
		unsentBlocks[i] = i;
	}
}

void FileTransferSend::workerLoop()
{
	using namespace std::chrono_literals;
	while(1) {
		auto blk = popBlock();
		if(!blk) {
			break;
		}
		fmt::print("sending block {}\n",blk.value());
		bool succ = false;
		while(!succ) {
			succ = sendBlock(blk.value()) == 0;
			if(!succ) {
				std::this_thread::sleep_for(500ms);
			}
		}
		invoke(onUpdate);
	}
}

std::optional<size_t> FileTransferSend::popBlock()
{
	std::lock_guard<std::mutex> lock(mtx);
	
	if(unsentBlocks.size()) {
		auto ret = unsentBlocks.front();
		unsentBlocks.pop_front();
		return ret;
	}
	return std::nullopt;
}

FileTransferSend::~FileTransferSend()
{
	for(auto& worker: workers) {
		if(worker.joinable()) {
			worker.join();
		}
	}
}

int FileTransferSend::start()
{
	workers.resize(DEFAULT_NUM_WORKERS);

	for(auto& worker: workers) {
		worker = std::thread([this](){workerLoop();});
	}
	return 0;
}

int FileTransferSend::sendMeta()
{
	try {
		MsgFileMeta meta;
		meta.blockSize = blockSize;
		meta.size = fileSize;
		meta.filename = stdfs::path(inputPath).filename();

		auto hash = digest::sha256();

		inFile = std::ifstream(inputPath, std::ios::binary);
		hash.absorb_istream(inFile);
		inFile = std::ifstream(inputPath, std::ios::binary);

		hash.finalize(meta.sha256.data());

		auto metaRaw = meta.getRaw();
		app->SendEncryptedPacket<int>("FileMeta", FILE_META, metaRaw.data(), metaRaw.size());
		return 0;
	} catch(std::exception& e) {
		return -1;
	}

}

size_t FileTransferSend::bytesCompleted() const
{
	return std::min(
		fileSize,
		(getNumBlocks() - unsentBlocks.size()) * blockSize
	);
}

std::string FileTransferSend::fileName() const
{
	return inputPath;
}


int FileTransferSend::sendBlock(size_t num)
{
	std::vector<uint8_t> outDat(blockSize);

	MsgFileBlock block;
	block.offset = num * blockSize;
	block.blockId = num;

	{
		std::lock_guard<std::mutex> lock(mtx);
		inFile.clear();
		inFile.seekg(block.offset, std::ios::beg);
		inFile.read((char*)outDat.data(), outDat.size());
		outDat.resize(inFile.gcount());
	}

	digest::sha256().absorb(outDat.data(), outDat.size()).finalize(&block.checksum);
	block.data = outDat;

	auto blockRaw = block.getRaw();
	auto f = app->SendEncryptedPacket<int>("FileBlock", FILE_BLOCK, blockRaw.data(), blockRaw.size());


	if(isDone() && app->fileFinishCallback) {
		inFile.close();
		app->fileFinishCallback(app);
	}

	return f.Get();
}

int FileTransferSend::ackAccept()
{
	awaitingAccept = false;
	invoke(onAccept);
	return start();
}



FileTransferRecv::FileTransferRecv(const MsgFileMeta& meta, AppState* app)
	: FileTransfer(DEFAULT_BLOCK_SIZE, app)
{
	outputPath = "recv_files/" + meta.filename;
	fileSize = meta.size;
	blockSize = meta.blockSize;
}

size_t FileTransferRecv::bytesCompleted() const
{
	return std::min(
		fileSize,
		blockSize * receivedBlocks.size()
	);
}

std::string FileTransferRecv::fileName() const
{
	return outputPath;
}


int FileTransferRecv::acceptTransfer()
{
	try {
		awaitingAccept = false;
		stdfs::create_directory("recv_files");
		outFile = std::ofstream(outputPath, std::ios::binary);

		size_t bytesLeft = fileSize;
		while(bytesLeft) {
			std::array<char, 4096> nul;
			nul.fill(0);
			size_t n = std::min(bytesLeft, nul.size());
			outFile.write(nul.data(), n);
			bytesLeft -= n;
		}

		outFile.seekp(0);
		invoke(onAccept);

		int xd = 0;
		app->SendEncryptedPacket<int>("FileTransferAccept", FILE_ACCEPT, &xd, sizeof(xd));
	} catch(std::exception&) {
		return -1;
	}

	return 0;
}

int FileTransferRecv::receiveBlock(const MsgFileBlock& block)
{
	if(awaitingAccept) {
		return -1;
	}

	Array32 sha256;
	digest::sha256().absorb(block.data.data(), block.data.size()).finalize(&sha256);

	if(sha256 != block.checksum) {
		return -2;
	}

	{
		std::lock_guard<std::mutex> lock(mtx);
		outFile.seekp(block.offset);
		outFile.write((char*)block.data.data(), block.data.size());
		receivedBlocks.insert(block.blockId);
	}
	invoke(onUpdate);


	if(isDone() && app->fileFinishCallback) {
		outFile.close();
		app->fileFinishCallback(app);
	}
	return 0;
}