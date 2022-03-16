
C = gcc
CFLAGS = -lpthread -Ofast -s -m64 -Irpclib/include -Isrc -I"digestpp"
CXX = g++
CXXFLAGS = -std=c++17 $(CFLAGS)

CRYPTO = bin/Crypto.o bin/Random.o bin/CryptoObjects.o
RPCLIB = rpclib/librpc.a
OBJECTS = $(CRYPTO) $(RPCLIB)

test: tests/test_crypto.exe tests/test_rpclib.exe
	tests/test_crypto.exe
	tests/test_rpclib.exe

tests/%.exe: bin/%.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^


bin/%.o: src/crypto/%.c
	$(CC) -c $(CFLAGS) -o $@ $^

bin/%.o: src/crypto/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $^

bin/%.o: tests/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $^

$(RPCLIB):
	(cd rpclib && cmake . && make $(MFLAGS))

.PHONY: clean
clean:
	rm bin/*.o *.o *.exe tests/*.exe *.a *.so

