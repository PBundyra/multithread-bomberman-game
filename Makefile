CCP=g++
client_src = src/client/client
server_src = src/server/server
client_bin = robots-client
server_bin = robots-server

all: compile_client compile_server
clean: clean_all

compile_client: $(client_src).cpp
	$(CCP) -Wall -Wextra -std=c++20 -O2 -pthread -o $(client_bin) $(client_src).cpp -lboost_program_options

compile_server: $(server_src).cpp
	$(CCP) -Wall -Wextra -std=c++20 -O2 -o $(server_bin) $(server_src).cpp

clean_binaries:
	rm *.o

clean_all:
	rm *.o robots-*