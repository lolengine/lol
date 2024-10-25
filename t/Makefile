
SRC = \
    test.cpp \
    audio-convert.cpp audio-sadd.cpp \
    base-narray.cpp \
    image-kernel.cpp

all: test

clean:
	rm -f test test.exe

check: test
	./test

test: $(SRC:%.cpp=%.o)
	$(CXX) $^ -o $@

%.o: %.cpp
	$(CXX) -std=c++23 -I../include -c $^ -o $@
