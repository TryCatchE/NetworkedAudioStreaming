#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#pragma comment(lib, "Ws2_32.lib")

void audioDataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = static_cast<ma_decoder*>(pDevice->pUserData);
    if (pDecoder == NULL) {
        memset(pOutput, 0, frameCount * ma_get_bytes_per_frame(pDevice->playback.format, pDevice->playback.channels));
        return;
    }

    // Read frames from the decoder
    ma_uint64 framesRead;
    ma_result result = ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, &framesRead);

    // Check the result and handle any errors
    if (result != MA_SUCCESS) {
        // Error handling here
        memset(pOutput, 0, frameCount * ma_get_bytes_per_frame(pDevice->playback.format, pDevice->playback.channels));
        return;
    }

    // If we didn't read enough frames, fill the rest of the buffer with silence
    if (framesRead < frameCount) {
        size_t bytesPerFrame = ma_get_bytes_per_frame(pDevice->playback.format, pDevice->playback.channels);
        memset((char*)pOutput + framesRead * bytesPerFrame, 0, (frameCount - framesRead) * bytesPerFrame);
    }
}


int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server_address;
    const int bufferSize = 1024;
    char buffer[bufferSize];

    // Initialize Winsock
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Create a TCP/IP socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // Define the server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(10000);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    // Connect the socket to the server's address
    connect(sock, (struct sockaddr*)&server_address, sizeof(server_address));

    // Send username and password
    std::string username;
    std::string password;
    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    std::cout << "Enter password: ";
    std::getline(std::cin, password);
    std::string credentials = username + "," + password;
    send(sock, credentials.c_str(), credentials.size(), 0);

    // Receive login response
    ZeroMemory(buffer, bufferSize);
    recv(sock, buffer, bufferSize, 0);
    std::string login_response(buffer);
    if (login_response == "Login failed") {
        std::cout << "Login failed. Exiting..." << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    } else {
        std::cout << "Login successful!" << std::endl;
    }

    // Receive and print the list of songs
    ZeroMemory(buffer, bufferSize);
    recv(sock, buffer, bufferSize, 0);
    std::string song_list(buffer);
    std::cout << "Available songs:" << std::endl << song_list << std::endl;

    // Request a specific song
    std::string song_number;
    std::cout << "Enter the song number to play: ";
    std::getline(std::cin, song_number);
    send(sock, song_number.c_str(), song_number.size(), 0);

    // Buffer to store the received song data
    std::vector<char> song_buffer;
    int bytesReceived = 0;
    do {
        ZeroMemory(buffer, bufferSize);
        bytesReceived = recv(sock, buffer, bufferSize, 0);
        if (bytesReceived > 0) {
            song_buffer.insert(song_buffer.end(), buffer, buffer + bytesReceived);
        }
    } while (bytesReceived > 0);

    // Cleanup Winsock
    closesocket(sock);
    WSACleanup();

    // Initialize Miniaudio
    ma_result result;
    ma_context context;
    ma_device device;
    ma_decoder decoder;
    result = ma_context_init(NULL, 0, NULL, &context);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize miniaudio context" << std::endl;
        return 1;
    }

    // Initialize the decoder with the received song data
    result = ma_decoder_init_memory(song_buffer.data(), song_buffer.size(), NULL, &decoder);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize decoder." << std::endl;
        ma_context_uninit(&context);
        return 1;
    }

    // Initialize the audio device
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = decoder.outputFormat;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate       = decoder.outputSampleRate;
    deviceConfig.dataCallback     = audioDataCallback;
    deviceConfig.pUserData        = &decoder; // Pass decoder to the callback

    result = ma_device_init(&context, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio device." << std::endl;
        ma_decoder_uninit(&decoder);
        ma_context_uninit(&context);
        return 1;
    }

    // Start the audio playback
    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to start audio device." << std::endl;
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        ma_context_uninit(&context);
        return 1;
    }

    std::cout << "Press Enter to stop playing..." << std::endl;
    std::cin.get(); // Wait for the user to press Enter

    // Stop the audio playback
    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);
    ma_context_uninit(&context);

    return 0;
}
