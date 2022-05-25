CCP=g++
client_src = client
server_src = server
client_bin = robots-client
server_bin = robots-server

all: compile_client compile_server
clean: clean_all

buffer: buffer.cpp buffer.h
	$(CCP) -Wall -Wextra -std=c++20 -O2 -pthread -o buffer buffer.cpp

compile_client: $(client_src).cpp $(client_src).h
	$(CCP) -Wall -Wextra -std=c++20 -O2 -pthread -o $(client_bin) $(client_src).cpp buffer.cpp game.cpp player.cpp -lboost_program_options \
	&& ./robots-client -p 2137 -s students.mimuw.edu.pl:10100 -d 0.0.0.0:10022 -n essa


compile_server: $(server_src).cpp
	$(CCP) -Wall -Wextra -std=c++20 -O2 -o $(server_bin) $(server_src).cpp

clean_binaries:
	rm *.o

clean_all:
	rm *.o robots-*