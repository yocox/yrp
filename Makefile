tree:
	g++ -g -std=c++11 ./example/tree.cc -I./include
	./a.out

test:
	g++ -g -std=c++11 test.cc -I./include
	./a.out
