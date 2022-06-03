
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
	Filestate(std::string fileName, size_t size);
	Filestate(std::string fileName);
	~Filestate() = default;
	
	int32_t UpdateReceive(const void* data, size_t bytes);
	
	inline bool Valid() const { return file.good(); }
	inline size_t Size() const { return size; }
	
	Future<int32_t> UpdateSend();
	Future<uint32_t> SendMeta();
	
	float GetProgress();
	
private:
	
	inline const static size_t BLOCK_SIZE = 16;
	
	size_t size;
	std::atomic<size_t> progress;
	
	std::fstream file;
	std::string fileName;
};

#endif

