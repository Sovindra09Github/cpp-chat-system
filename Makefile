CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
LDFLAGS = -lws2_32 -Wl,-subsystem,console

SERVER_SRC = src/main.cpp src/Server.cpp
CLIENT_SRC = client.cpp

SERVER_EXE = server.exe
CLIENT_EXE = client.exe

all: $(SERVER_EXE) $(CLIENT_EXE)

$(SERVER_EXE):
	$(CXX) $(CXXFLAGS) -o server.exe $(SERVER_SRC) $(LDFLAGS)

$(CLIENT_EXE):
	$(CXX) $(CXXFLAGS) -o client.exe $(CLIENT_SRC) $(LDFLAGS)

clean:
	rm -f *.exe
