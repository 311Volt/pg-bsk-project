
C = gcc
CFLAGS = -lpthread -Ofast -s -m64 -Irpclib/include -Isrc
CXX = g++
CXXFLAGS = -std=c++17 $(CFLAGS)

CRYPTO = bin/Crypto.o bin/Random.o bin/CryptoObjects.o
RPCLIB = rpclib/librpc.a
OBJECTS = $(CRYPTO) $(RPCLIB)

test/test_crypto.exe: bin/test_crypto.o $(CRPYTO)
	$(CXX) $(CXXFLAGS) -o $@ $^
	./$@

test/test_rpclib.exe: bin/test_rpclib.o $(RPCLIB)
	$(CXX) $(CXXFLAGS) -o $@ $^
	./$@


bin/%.o: src/crypto/%.c
	$(CC) -c $(CFLAGS) -o $@ $^

bin/%.o: src/crypto/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $^

bin/%.o: test/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $^

$(RPCLIB):
	(cd rpclib && cmake . && make $(MFLAGS))

.PHONY: clean
clean:
	rm bin/*.o
	rm *.o
	rm *.exe
	rm *.a
	rm *.so

