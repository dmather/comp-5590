CC=g++
CFLAGS=-Wall -g
CXXLIBS=-OpenCL

all: geneticV1_opencl geneticV1

geneticV1: geneticV1.cpp
	$(CC) $(CFLAGS) $^ -o $@

geneticV1_opencl: geneticV1_opencl.cpp
	$(CC) $(CFLAGS) -lOpenCL $^ -o $@
