#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <sstream>
#include <filesystem>

#pragma comment(lib, "Ws2_32.lib")

void initializeWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        exit(1);
    }
}


namespace fs = std::filesystem;

std::vector<std::string> getSongsFromFolder(const std::string& folderPath) {
    std::vector<std::string> songFiles;
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            songFiles.push_back(entry.path().filename().string());
        }
    }
    return songFiles;
}

std::map<std::string, std::string> valid_credentials = {
    {"user1", "password1"},
    {"user2", "password2"},
    {"user3", "password3"}
};


void sendAudioData(SOCKET clientSocket, const std::string& songFilePath) {
    std::ifstream songFile(songFilePath, std::ios::binary);
    if (!songFile) {
        send(clientSocket, "Song not found", 14, 0);
        return;
    }

    const int audioBufferSize = 4096;  // Adjust as needed
    char audioBuffer[audioBufferSize];

    while (!songFile.eof()) {
        songFile.read(audioBuffer, audioBufferSize);
        int bytesRead = static_cast<int>(songFile.gcount());
        if (bytesRead <= 0) {
            break;
        }
        send(clientSocket, audioBuffer, bytesRead, 0);
    }

    songFile.close();
}

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
    std::stringstream songList;
    std::vector<std::string> songs = getSongsFromFolder("songs_folder");
    for (int i = 0; i < songs.size(); ++i) {
        songList << i << ". " << songs[i] << "\n";
    }
    send(clientSocket, songList.str().c_str(), static_cast<int>(songList.str().size()), 0);

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

    // Send the selected song in binary format
    std::string selectedSong = songs[selectedSongIndex];
    std::string songFilePath = "songs_folder/" + selectedSong;
    sendAudioData(clientSocket, songFilePath);

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
