
#include <cstring>
#include <cmath>
#include <functional>
#include <memory>

#include "AppState.hpp"
#include "Filestate.hpp"
#include "crypto/Crypto.hpp"

#include <fmt/format.h>

Filestate::Filestate(std::string fileName, size_t size) : file(fileName, std::ios::binary|std::ios_base::out) {
	this->fileName = fileName;
	this->size = size;
	progress = 0;
	printf(" Filestate receive: %s %lu\n", fileName.c_str(), size);
}

Filestate::Filestate(std::string fileName) : file(fileName, std::ios::binary|std::ios_base::in) {
	this->fileName = fileName;
	file.seekg(0, std::ios_base::end);
	this->size = file.tellg();
	progress = 0;
	printf(" Filestate send: %s %lu\n", fileName.c_str(), size);
}


int32_t Filestate::UpdateReceive(const void* data, size_t bytes) {
	file.seekp(progress.load());
	file.write((const char*)data, bytes);
	progress += bytes;
	printf(" Receive progress: %lu/%lu\n", progress.load(), size);
	return bytes;
}

Future<int32_t> Filestate::UpdateSend() {
	size_t bytes = progress-size;
	if(bytes >= BLOCK_SIZE) {
		bytes = BLOCK_SIZE;
	}
	if(bytes == 0) {
		Promise<int32_t> p;
		p.SetValue(0);
		return p.GetFuture();
	}
	file.seekg(progress.load());
	uint8_t b[BLOCK_SIZE];
	file.read((char*)b, bytes);
	return AppState::singleton->SendEncryptedPacket<uint32_t>(
			"FileBlock", FILE_BLOCK, b, bytes).Then<int32_t>(
				[this](uint32_t _read)->int32_t{
					int32_t read = _read;
					this->progress += read;
					printf(" Send progress: %lu/%lu (of: %i)\n", progress.load(), size, read);
					if(this->progress >= this->size || read == 0) {
						return 0;
					}
					this->UpdateSend();
					return read;
				});
}

Future<uint32_t> Filestate::SendMeta() {
	uint8_t b[4096];
	*(uint64_t*)b = size;
	memcpy(b+8, fileName.c_str(), fileName.size()+1);
	return AppState::singleton->SendEncryptedPacket<uint32_t>(
			"FileMeta", FILE_META, b, 8+1+fileName.size()+1).Then<uint32_t>(
				[this](uint32_t read)->uint32_t{
					if(read != 0) {
						this->SendMeta();
						return 1;
					}
					this->UpdateSend();
					return 0;
				});
}

float Filestate::GetProgress() {
	return (float)progress.load()/(float)size;
}

