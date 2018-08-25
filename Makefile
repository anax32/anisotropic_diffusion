CXX=g++
CXXFLAGS=-Wall

bin:
	mkdir bin

clean:
	rm -drf bin

apply: apply_filters.cpp pngreader.h diffusion.h bin
	$(CXX) $(CXXFLAGS) $< -o bin/$@ -lpng

all: apply
