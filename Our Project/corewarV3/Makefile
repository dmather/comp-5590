CC=g++
CFLAGS=-Wall -g
CXXLIBS=-OpenCL

all: corewarV3_opencl geneticV1_opencl

#corewarV3_opencl: corewarV3_opencl.o redcode_fileio.o 
corewarV3_opencl: corewarV3_opencl.cpp
	$(CC) $(CFLAGS) -lOpenCL $^ -o $@

#corewarV3_opencl.o: corewarV3_opencl.cpp
#	$(CC) $(CFLAGS) -lOpenCL corewarV3_opencl.cpp

#redcode_fileio.o: redcode_fileio.cpp
#	$(CC) $(CFLAGS) -lOpenCL redcode_fileio.cpp

geneticV1: geneticV1.cpp
	$(CC) $(CFLAGS) $^ -o $@

geneticV1_opencl: geneticV1_opencl.cpp
	$(CC) $(CFLAGS) -lOpenCL $^ -o $@
