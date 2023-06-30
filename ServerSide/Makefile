
CC = g++

all: windows_server 
windows_server: server_main.cpp
	$(CC) server_main.cpp -lws2_32 -o windows_server -g

run_server: windows_server
	windows_server $(ip) $(port)

clean:
	del /Q *.o *.exe

