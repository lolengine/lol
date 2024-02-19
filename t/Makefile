
SRC = audio.cpp

all: test

clean:
	rm -f test test.exe

check: test
	./test

test: $(SRC)
	$(CXX) -I../include $^ -o $@
