#include "FileMessages.hpp"

#include <sstream>
#include <cstdlib>


template<typename T>
std::vector<uint8_t> ToRaw(const T& obj)
{
	clmdep_msgpack::sbuffer sbuf;
	clmdep_msgpack::pack(sbuf, obj);
	std::vector<uint8_t> ret(sbuf.size());
	std::memcpy(ret.data(), sbuf.data(), sbuf.size());
	return ret;
}

template<typename T>
T FromRaw(const std::vector<uint8_t>& raw)
{
	auto objh = clmdep_msgpack::unpack((const char*)raw.data(), raw.size());
	return objh.get().as<T>();
}


MsgFileMeta::MsgFileMeta(const std::vector<uint8_t>& raw)
{
	*this = FromRaw<MsgFileMeta>(raw);
}

std::vector<uint8_t> MsgFileMeta::getRaw()
{
	return ToRaw<MsgFileMeta>(*this);
}

MsgFileBlock::MsgFileBlock(const std::vector<uint8_t>& raw)
{
	*this = FromRaw<MsgFileBlock>(raw);
}

std::vector<uint8_t> MsgFileBlock::getRaw()
{
	return ToRaw<MsgFileBlock>(*this);
}