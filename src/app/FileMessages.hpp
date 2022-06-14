#ifndef SRC_APP_FILEMESSAGES
#define SRC_APP_FILEMESSAGES

#include <rpc/msgpack.hpp>


struct MsgFileMeta {
	MsgFileMeta(){}
	MsgFileMeta(const std::vector<uint8_t>& raw);

	std::string filename;
	std::array<uint8_t, 32> sha256;
	uint64_t size;
	uint64_t blockSize;

	MSGPACK_DEFINE_ARRAY(filename, sha256, size, blockSize);

	std::vector<uint8_t> getRaw();
};

struct MsgFileBlock {
	MsgFileBlock(){}
	MsgFileBlock(const std::vector<uint8_t>& raw);

	uint64_t blockId, offset;
	std::array<uint8_t,32> checksum;
	std::vector<uint8_t> data;

	MSGPACK_DEFINE_ARRAY(blockId, offset, checksum, data);

	std::vector<uint8_t> getRaw();
};


#endif /* SRC_APP_FILEMESSAGES */
