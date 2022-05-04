
.PHONY: run
run: build
	( cd build ; ./pg-bsk-project )

.PHONY: build
build:
	( echo 1 ; ( mkdir build || true ) ; echo 2 ; cd build ; echo 3 ; cmake .. ; echo 4 ; make $(MFLAGS) )

