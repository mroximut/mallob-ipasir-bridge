
all: libipasirmallob.a ipasir_example

mallob_ipasir.o: src/mallob_ipasir.cpp src/mallob_ipasir.hpp src/timer.cpp
	g++ -c -std=c++17 -O3 src/mallob_ipasir.cpp src/timer.cpp -Isrc

libipasirmallob.a: mallob_ipasir.o
	ar rvs libipasirmallob.a mallob_ipasir.o timer.o

ipasir_example: ipasir_example.cpp libipasirmallob.a
	g++ -std=c++17 -O3 ipasir_example.cpp -o ipasir_example -L. -lipasirmallob -Isrc