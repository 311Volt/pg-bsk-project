
C = gcc
CFLAGS = -lpthread -Ofast -s -m64
CXX = g++
CXXFLAGS = -std=c++17 $(CFLAGS)

CRYPTO = bin/Crypto.o bin/Random.o bin/CryptoObjects.o

test.exe: bin/test_crypto.o $(CRYPTO)
	$(CXX) $(CXXFLAGS) -o $@ $^
	./test.exe

bin/%.o: src/crypto/%.c
	$(CC) -c $(CFLAGS) -o $@ $^

bin/%.o: src/crypto/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $^

bin/%.o: test/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm bin/*.o
	rm *.o
	rm *.exe
	rm *.a
	rm *.so

