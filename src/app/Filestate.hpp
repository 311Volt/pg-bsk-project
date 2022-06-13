
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

