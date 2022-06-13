
#include <cstring>
#include <cmath>
#include <functional>
#include <memory>

#include "AppState.hpp"
#include "Filestate.hpp"
#include "crypto/Crypto.hpp"

#include <fmt/format.h>

Filestate::Filestate(void* hash, std::string fileName, size_t size) {
	DEBUG();
	this->fileName = fileName;
	file = fopen(fileName.c_str(), "wb");
	this->size = size;
	memcpy(this->hash.data(), hash, this->hash.size());
	progress = 0;
	// TODO: event started receiving
	printf(" Filestate receive: %s %lu\n", fileName.c_str(), size);
	printf(" file = %p [%p]\n", file, this);
	DEBUG();
}

Filestate::Filestate(std::string fileName) {
	DEBUG();
	this->fileName = fileName;
	file = fopen(fileName.c_str(), "rb");
	fseek(file, 0, SEEK_END);
	this->size = ftell(file);
	fseek(file, 0, SEEK_SET);
	FileDigest(this->hash.data());
	fseek(file, 0, SEEK_SET);
	progress = 0;
	// TODO: event started sending
	printf(" Filestate send: %s %lu\n", fileName.c_str(), size);
	printf(" file = %p [%p]\n", file, this);
	DEBUG();
}

Filestate::~Filestate() {
	if(file) {
	DEBUG();
		fflush(file);
		fclose(file);
		file = NULL;
	}
}


int32_t Filestate::UpdateReceive(const void* data, size_t bytes) {
	DEBUG();
	uint8_t R;
	Random::Fill(&R, 1);
	fwrite(data, 1, bytes, file);
	this->sha.absorb((const char*)data, bytes);
	progress += bytes;
	// TODO: event update progress receive
	printf(" Receive progress: %lu/%lu\n", progress.load(), size);
	if(progress == size) {
		uint8_t hash[32];
		sha.finalize(hash);
		if(memcmp(hash, this->hash.data(), 32) != 0) {
	DEBUG();
			fflush(file);
			fclose(file);
			file = NULL;
			AppState::singleton->filestate = NULL;
			// TODO: event file receiving failed: retrying
			printf(" file receiving failed: retrying\n");
			fflush(stdout);
			return -3;
		} else {
	DEBUG();
			fflush(file);
			fclose(file);
			file = NULL;
			// TODO: event file receiving success
			printf(" file receiving success\n");
			fflush(stdout);
		}
	}
	return bytes;
}

Future<int32_t> Filestate::UpdateSend() {
	DEBUG();
	size_t bytes = progress-size;
	if(bytes > BLOCK_SIZE) {
		bytes = BLOCK_SIZE;
	}
	if(bytes == 0) {
		Promise<int32_t> p;
		p.SetValue(0);
		return p.GetFuture();
	}
	printf(" file = %p [%p]\n", file, this);
	fseek(file, progress.load(), SEEK_SET);
	uint8_t b[BLOCK_SIZE];
	bytes = fread(b, 1, bytes, file);
	printf(" read bytes from file to send: %lu at progress = %lu/%lu\n", bytes, progress.load(), size);
	return AppState::singleton->SendEncryptedPacket<int32_t>(
			"FileBlock", FILE_BLOCK, b, bytes).Then<int32_t>(
				[this](uint32_t _read)->int32_t{
					int32_t read = _read;
					printf(" returned from FileBlock = %i\n", read);
					DEBUG();
					if(read < 0) {
						this->SendMeta();
						DEBUG();
						return -4;
					}
					DEBUG();
					this->progress += read;
					printf(" Send progress: %lu/%lu (of: %i)\n",
							progress.load(), size, read);
					if(this->progress >= this->size) {
						// todo finished sending file correct
						printf(" finished sending file\n");
						self = NULL;
						return 0;
					}
					this->UpdateSend();
					return read;
				});
}

Future<uint32_t> Filestate::SendMeta() {
	DEBUG();
	printf(" Sending meta\n");
	uint8_t b[4096];
	*(uint64_t*)b = size;
	memcpy(b+8, hash.data(), 32);
	memcpy(b+8+32, fileName.c_str(), fileName.size()+1);
	return AppState::singleton->SendEncryptedPacket<uint32_t>(
			"FileMeta", FILE_META, b, 8+32+fileName.size()+1).Then<uint32_t>(
				[this](uint32_t read)->uint32_t{
					DEBUG();
					if(read != 0) {
						DEBUG();
						this->SendMeta();
						return 1;
					} else {
						DEBUG();
						this->progress = 0;
						this->UpdateSend();
						return 0;
					}
				});
}

void Filestate::FileDigest(void* digest) {
	size_t pos = ftell(file);
	fseek(file, 0, SEEK_SET);
	digest::sha256().absorb_file(file).finalize(digest);
	fseek(file, pos, SEEK_SET);
	printf(" HASH: %lX\n", *(uint64_t*)digest);
	printf(" HASH REQ: %lX\n", *(uint64_t*)this->hash.data());
}

float Filestate::GetProgress() {
	return (float)progress.load()/(float)size;
}

