
#ifndef PG_BSK_PROJECT_FILESTATE_HPP
#define PG_BSK_PROJECT_FILESTATE_HPP

#include <exception>
#include <fstream>
#include <future>
#include <vector>
#include <string>
#include <atomic>
#include <array>
#include <queue>
#include <mutex>

#include "../crypto/Crypto.hpp"

#include <rpc/client.h>
#include <rpc/server.h>
#include <rpc/rpc_error.h>

#include <fmt/format.h>

#include <stdexcept>

#include "Codes.hpp"
#include "FuturePromise.hpp"
#include "rpc/msgpack/v1/object.hpp"


struct MsgFileMeta {
	std::string filename;
	std::array<uint8_t, 32> sha256;
	uint64_t size;
	uint64_t blockSize;

	MSGPACK_DEFINE_ARRAY(filename, sha256, size, blockSize);


};

struct MsgFileBlock {
	uint64_t offset;
	uint64_t checksum;
	std::vector<uint8_t> data;

	MSGPACK_DEFINE_ARRAY(offset, checksum, data);
};


class FileTransfer {
	uint64_t transferId;
	bool awaitingAccept;
	size_t blockSize;
public:
	FileTransfer();

	virtual size_t bytesCompleted() = 0;
	virtual size_t bytesTotal() = 0;
};

class FileTransferSend: public FileTransfer {
	std::string inputPath;
	std::ifstream inFile;
	std::vector<int> blockStatus;
public:
	FileTransferSend(const std::string_view filename);

	int sendBlock();
};

class FileTransferRecv: public FileTransfer {
	std::string outputPath;
	std::ofstream outFile;
	std::array<uint8_t, 32> sha256;
	size_t blockSize;
public:
	FileTransferRecv(const MsgFileMeta& meta);

	bool acceptTransfer();

	int receiveBlock(const MsgFileBlock& block);
};


class Filestate {
public:
	Filestate(void* hash, std::string fileName, size_t size);
	Filestate(std::string fileName);
	~Filestate();
	
	int32_t UpdateReceive(const void* data, size_t bytes);
	
	inline bool Valid() const { return file; }
	inline size_t Size() const { return size; }
	
	Future<int32_t> UpdateSend();
	Future<uint32_t> SendMeta();
	
	void FileDigest(void* digest);
	
	float GetProgress();
	
	std::shared_ptr<Filestate> self;
	
private:
	
	inline const static size_t BLOCK_SIZE = 4096;
	
	size_t size;
	std::atomic<size_t> progress;
	
	FILE* file;
	std::string fileName;
	std::array<uint8_t, 32> hash;
	digest::sha256 sha;
};

#endif

