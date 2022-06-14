#ifndef SRC_APP_FILETRANSFER
#define SRC_APP_FILETRANSFER

#include <functional>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <optional>
#include <fstream>

#include "FileMessages.hpp"
#include "Codes.hpp"

class AppState;

using CallbackT = std::function<void(void)>;

class FileTransfer {
protected:
	AppState* app;
	uint64_t transferId;
	bool awaitingAccept;
	size_t fileSize, blockSize;
	CallbackT onAccept, onUpdate;
	std::mutex mtx;
	void invoke(CallbackT cb);
public:
	FileTransfer(size_t blockSize, AppState* app);
	virtual ~FileTransfer();

	virtual size_t bytesCompleted() const = 0;
	virtual std::string fileName() const = 0;
	bool isDone();
	size_t bytesTotal() const;

	size_t getNumBlocks() const;

	void setOnAcceptFn(CallbackT fn);
	void setOnUpdateFn(CallbackT fn);	
};

class FileTransferSend: public FileTransfer {
	std::string inputPath;
	std::ifstream inFile;
	std::vector<int> blockStatus;
	std::deque<size_t> unsentBlocks;

	std::thread dispatcher;
	std::vector<std::thread> workers;
	std::vector<size_t> freeWorkers;

	bool exitFlag;

	void workerLoop();
	std::optional<size_t> popBlock();
public:
	FileTransferSend(const std::string_view filename, AppState* app);
	~FileTransferSend();

	int start();
	int sendMeta();

	virtual size_t bytesCompleted() const override;
	virtual std::string fileName() const override;
	
	int sendBlock(size_t num);
	int ackAccept();
};

class FileTransferRecv: public FileTransfer {
	std::string outputPath;
	std::ofstream outFile;
	std::array<uint8_t, 32> sha256;
	size_t blockSize;
	std::unordered_set<size_t> receivedBlocks;
public:
	FileTransferRecv(const MsgFileMeta& meta, AppState* app);
	
	virtual size_t bytesCompleted() const override;
	virtual std::string fileName() const override;

	int acceptTransfer();
	int receiveBlock(const MsgFileBlock& block);
};


#endif /* SRC_APP_FILETRANSFER */
