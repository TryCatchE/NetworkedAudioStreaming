#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")


void initializeWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        exit(1);
    }
}

std::map<std::string, std::string> valid_credentials = {
    {"user1", "password1"},
    {"user2", "password2"},
    {"user3", "password3"}
};
std::vector<std::string> songs = {"song1.wav", "song2.wav", "song3.wav"};



void handleClient(SOCKET clientSocket) {
    char buffer[1024];

    // Receive username and password
    memset(buffer, 0, 1024);
    int bytesReceived = recv(clientSocket, buffer, 1024, 0);
    if (bytesReceived == SOCKET_ERROR) {
        closesocket(clientSocket);
        return;
    }

    std::string credentials(buffer);
    auto separator = credentials.find(',');
    std::string username = credentials.substr(0, separator);
    std::string password = credentials.substr(separator + 1);

    // Validate credentials
    if (valid_credentials.find(username) != valid_credentials.end() && valid_credentials[username] == password) {
        send(clientSocket, "Login successful", 16, 0);
    } else {
        send(clientSocket, "Login failed", 12, 0);
        closesocket(clientSocket);
        return;
    }

    // Send song list
    std::string songList;
    for (int i = 0; i < songs.size(); ++i) {
        songList += std::to_string(i) + ". " + songs[i] + "\n";
    }
    send(clientSocket, songList.c_str(), songList.size(), 0);

    // Receive song number from the client
    memset(buffer, 0, 1024);
    int songNumberSize = recv(clientSocket, buffer, 1024, 0);
    if (songNumberSize == SOCKET_ERROR) {
        closesocket(clientSocket);
        return;
    }

    int selectedSongIndex = std::stoi(buffer);

    // Check if the selected song index is valid
    if (selectedSongIndex < 0 || selectedSongIndex >= songs.size()) {
        send(clientSocket, "Invalid song selection", 21, 0);
        closesocket(clientSocket);
        return;
    }

    std::string selectedSong = songs[selectedSongIndex];

    // Open and send the selected song file to the client
    std::string songFilePath = "songs_folder/" + selectedSong;
    FILE* songFile = fopen(songFilePath.c_str(), "rb");
    if (!songFile) {
        send(clientSocket, "Song not found", 14, 0);
        closesocket(clientSocket);
        return;
    }

    char songBuffer[1024];
    while (true) {
        size_t bytesRead = fread(songBuffer, 1, sizeof(songBuffer), songFile);
        if (bytesRead <= 0) {
            break;
        }
        send(clientSocket, songBuffer, bytesRead, 0);
    }

    // Close the song file
    fclose(songFile);

    closesocket(clientSocket);
}



int main() {
    initializeWinsock();

    SOCKET server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        perror("socket failed");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(10000);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        perror("bind failed");
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        perror("listen");
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    while (true) {
        SOCKET new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket == INVALID_SOCKET) {
            perror("accept");
            closesocket(server_fd);
            WSACleanup();
            exit(EXIT_FAILURE);
        }
        std::thread clientThread(handleClient, new_socket);
        clientThread.detach();
    }

    // Clean up
    closesocket(server_fd);
    WSACleanup();
    return 0;
}
