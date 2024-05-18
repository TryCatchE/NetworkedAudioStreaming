# Networked Audio Streaming

This project consists of two code snippets: one for the client and one for the server, creating a simple networked audio streaming application.

## Client Application (`client.cpp`):

Connects to the server, logs in, selects a song from the available list, and streams it for playback.

### Usage:
1. Compile the code.
2. Run the executable.
3. Enter username and password.
4. Choose a song.
5. Press Enter to start playback.
6. Press Enter again to stop and exit.

## Server Application (`server.cpp`):

Listens for client connections, authenticates users, and streams requested songs.

### Usage:
1. Compile the code.
2. Run the executable.
3. Ensure "songs_folder" has the audio files.
4. Clients connect using IP and port.
5. Provide credentials to log in.
6. Select a song to stream.

### Notes:
- Both use Winsock for networking.
- Server authenticates against a preset list.
- Client uses Miniaudio for playback.
