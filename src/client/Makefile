CCP=g++
client_src = client
server_src = server
client_bin = robots-client
server_bin = robots-server

all: compile_client
clean: clean_all

compile_client: $(client_src).cpp $(client_src).h
	$(CCP) -Wall -Wextra -std=c++20 -O2 -pthread -DNDEBUG -o $(client_bin) $(client_src).cpp buffer.cpp game.cpp player.cpp events.cpp utils.cpp -lboost_program_options \

run_client:
	./$(client_bin) -p 2137 -s students.mimuw.edu.pl:10111 -d 0.0.0.0:10022 -n essa

compile_server: $(server_src).cpp
	$(CCP) -Wall -Wextra -std=c++20 -O2 -o $(server_bin) $(server_src).cpp

clean_binaries:
	rm *.o

clean_all:
	rm *.o