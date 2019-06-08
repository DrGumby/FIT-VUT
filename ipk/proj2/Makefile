CXX = g++
CXXFLAGS = -pedantic -Wall -g
CXXLIBS = -lpcap
source1 = ipk-scan.cc 
out1 = ipk-scan

build:  
	${CXX} ${CXXFLAGS} ${source1} -o ${out1} ${CXXLIBS}
run: build
	./${out1}
