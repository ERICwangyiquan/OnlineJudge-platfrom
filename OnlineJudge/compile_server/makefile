compile_server:compile_server.cc
	g++ -o $@ $^ -L/usr/lib64 -std=c++11 -ljsoncpp -lpthread

.PHONY:clean
clean:
	rm -f compile_server