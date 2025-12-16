# C++ Socket-Based Chat System
A multi-threaded, client-server chat system built from scratch in C++ using TCP/IP sockets. It's designed to handle multiple concurrent users for real-time messaging, showcasing fundamental low-level network programming and concurrency concepts.
### Features
Multi-Threaded Server: Utilizes a thread-per-client model to handle multiple concurrent users in parallel.

Real-time Messaging: Supports both public broadcast messages and private one-to-one messages.

User Nicknames: Allows users to set and change their nicknames with the /nick command.

Online User List: Users can request a list of all currently connected clients with the /list command.

Graceful Shutdown: The server can be shut down cleanly using a SHUTDOWN command in its console, notifying all clients.

External Configuration: The server port is configured via an external server.conf file, so no recompilation is needed to change it.

Asynchronous Client: The client uses select() to handle both user input and network messages simultaneously, ensuring notifications are never missed.

Object-Oriented Design: The server code is refactored into a clean, encapsulated Server class.

## Building the Project
#### Prerequisites
- A C++ compiler that supports C++17 (e.g., g++ or clang++)
- make

### Running the Application
You will need at least two separate terminal windows.

Start the Server: In your first terminal, run the server executable:

./server.exe

The server will start and listen on the port specified in server.conf (default is 8080).

Start the Client(s): In one or more other terminals, run the client executable:

./client.exe

The client will connect to the server. You can now start chatting!

Server Administration:

To change the port, edit the PORT value in the server.conf file and restart the server.
To shut down the server gracefully, type /shutdown in the server's terminal window and press Enter.

## Chat Usage

Once connected, you can type messages to send them publicly. The following special commands are available:

/nick <new_name> - Changes your nickname.

Example: /nick Alice

/msg <username> <message> - Sends a private message to a specific user.

Example: /msg Bob Hello there!

/list - Shows a list of all users currently online.

/exit - Disconnects you from the server.
