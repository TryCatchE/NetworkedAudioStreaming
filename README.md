Networked Audio Streaming Application
This project consists of two code snippets that together create a simple networked audio streaming application. One snippet represents the client-side application responsible for selecting and playing songs, while the other represents the server-side application responsible for serving songs to clients.

Client Application (client.cpp)
The client application connects to the server and allows users to log in, select a song from the available list, and stream it for playback.

Requirements:
Windows operating system
Winsock library
Miniaudio library
Usage:
Compile the code with a C++ compiler.
Run the compiled executable.
Enter your username and password when prompted.
Choose a song from the available list.
Press Enter to start playing the selected song.
Press Enter again to stop playback and exit the application.
Server Application (server.cpp)
The server application listens for incoming connections from clients, authenticates users, and streams requested songs to clients.

Requirements:
Windows operating system
Winsock library
Usage:
Compile the code with a C++ compiler.
Run the compiled executable.
Ensure that the "songs_folder" directory contains the audio files you want to serve.
Clients can connect to the server using its IP address and port number (default port: 10000).
Clients must provide valid credentials (username and password) to log in.
Clients can then select a song from the list provided by the server and start streaming it.
Note:
Both client and server applications use the Winsock library for network communication.
The server-side application authenticates users against a predefined set of credentials.
The client-side application uses the Miniaudio library for audio playback.
