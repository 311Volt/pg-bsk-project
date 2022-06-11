
#include "Random.hpp"
#include <stdio.h>

#include <memory>
#include <stdexcept>

#ifdef __unix__

struct FileCloser{void operator()(FILE* f){fclose(f);}};

void Random::Fill(void* ptr, size_t size) 
{
	std::unique_ptr<FILE, FileCloser> f(fopen("/dev/urandom", "rb"));
	if(!f) {
		throw std::runtime_error("cannot open /dev/urandom");
	}
	fread(ptr, 1, size, f.get());
}


#elif defined(WIN32)

//****NOT TESTED****

#include <windows.h>
#include <Wincrypt.h>

struct RandomContext {
	HCRYPTPROV hCryptProv;

	RandomContext()
	{
		auto res = CryptAcquireContextA(
			&hCryptProv,
			NULL,
			"Microsoft Base Cryptographic Provider v1.0",
			PROV_RSA_FULL,
			CRYPT_VERIFYCONTEXT
		);

		if(!res) {
			throw std::runtime_error("cannot acquire Win32 cryptographic context");
		}
	}

	~RandomContext()
	{
		CryptReleaseContext(hCryptProv, 0);
	}
};

void Random::Fill(void* ptr, size_t size)
{
	static RandomContext ctx;

	CryptGenRandom(ctx.hCryptProv, size, ptr);
}

#else
	#error "sorry, not implemented"
#endif
