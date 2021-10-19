.PHONY: all
all: format build

.PHONY: format
format:
	clang-format src/*.cpp -i

.PHONY: build
build:
	mkdir -p build
	cd build && \
	cmake -DCMAKE_CXX_COMPILER=/usr/bin/g++ .. && \
	make


.PHONY: debug   
debug:
	mkdir -p build
	cd build  \
	cmake -DCMAKE_CXX_COMPILER=/usr/bin/g++ .. && \
	make

.PHONY: clang   
clang:
	mkdir -p build
	cd build  \
	cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++-13 -DCMAKE_BUILD_TYPE=Debug .. && \
	make

.PHONY: clean
clean:
	rm -rf build
