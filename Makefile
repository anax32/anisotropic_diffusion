CXX=g++
CXXFLAGS=-Wall

all: bin apply

bin:
	mkdir bin

clean:
	rm -drf bin

apply: apply_filters.cpp pngreader.h diffusion.h bin
	$(CXX) $(CXXFLAGS) -O4 $< -o bin/$@ -lpng

test-apply: apply_filters.cpp pngreader.h diffusion.h bin
	$(CXX) $(CXXFLAGS) -g $< -o bin/$@ -lpng \
	&& \
	valgrind \
		--leak-check=yes \
		--log-file=$@.valgrind.log \
		bin/$@ -i data/geo_sm1.png -o test.png -a -n 10 \
        && \
	valgrind \
		--leak-check=yes \
		--log-file=$@.valgrind.log \
		bin/$@ -i data/geo_sm1.png -o test.png -g -n 10

