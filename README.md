Networked Audio Streaming

This project consists of two code snippets: one for the client and one for the server, creating a simple networked audio streaming application.

Client Application (client.cpp):

Connects to the server, logs in, selects a song from the available list, and streams it for playback.

Usage:

Compile the code.
Run the executable.
Enter username and password.
Choose a song.
Press Enter to start playback.
Press Enter again to stop and exit.
Server Application (server.cpp):

Listens for client connections, authenticates users, and streams requested songs.

Usage:

Compile the code.
Run the executable.
Ensure "songs_folder" has the audio files.
Clients connect using IP and port.
Provide credentials to log in.
Select a song to stream.
Notes:

Both use Winsock for networking.
Server authenticates against a preset list.
Client uses Miniaudio for playback.
