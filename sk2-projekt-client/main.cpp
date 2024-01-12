#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include "json.hpp"

#define MAX_BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

template <typename T> void print(T obj) { std::cout << obj << std::endl; }

void sendToSocket(int fd, std::string message) {
    unsigned int messageSize = message.size();
    unsigned int netMessageSize = htonl(messageSize);

    if (send(fd, &netMessageSize, sizeof(netMessageSize), 0) == -1) {
        perror("(send)");
    }

    if (send(fd, message.data(), message.size()*sizeof(char), 0) == -1) {
        perror("(send)");
    }
}

nlohmann::json createChannelRequest(std::string name){
    nlohmann::json request;
    request["requestType"] = 1;
    request["channel"] = name;
    return request;
}

nlohmann::json deleteChannelRequest(std::string name){
    nlohmann::json request;
    request["requestType"] = 2;
    request["channel"] = name;
    return request;
}

nlohmann::json subscribeChannelRequest(std::string name){
    nlohmann::json request;
    request["requestType"] = 3;
    request["channel"] = name;
    return request;
}

nlohmann::json unsubscribeChannelRequest(std::string name){
    nlohmann::json request;
    request["requestType"] = 4;
    request["channel"] = name;
    return request;
}

nlohmann::json publishRequest(std::string channel, std::string message){
    nlohmann::json request;
    request["requestType"] = 5;
    request["channel"] = channel;
    request["message"] = message;
    return request;
}

void sendRequest(int fd, nlohmann::json jsonRequest){
    std::string stringRequest;
    stringRequest = nlohmann::to_string(jsonRequest);
    sendToSocket(fd, stringRequest);
}

std::string receiveFromSocket(int fd) {
    unsigned int messageSize = 0;
    int bytes = recv(fd, &messageSize, sizeof(messageSize), MSG_WAITALL);

    messageSize = ntohl(messageSize);

    if (bytes != sizeof(messageSize) && bytes != 0) {
        perror("(read)");
        exit(1);
    }

    auto buffer = std::string(messageSize, 0);

    int messageBytes = recv(fd, buffer.data(), messageSize, MSG_WAITALL);

    if (messageBytes != messageSize && messageBytes != 0) {
        perror("(read)");
        exit(1);
    }

    if (bytes == 0 || messageBytes == 0) {
        perror("Socket already closed");
        exit(1);
    }

    return buffer;
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation error");
        return 1;
    }

    struct sockaddr_in server_addr {
            .sin_family = AF_INET, .sin_port = htons(SERVER_PORT),
    };
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    char buffer[MAX_BUFFER_SIZE];

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
        -1) {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to the server\n");

    while (true) {
        printf("Enter a message: ");
        fgets(buffer, MAX_BUFFER_SIZE, stdin);

        std::string sBuffer = buffer;
        std::istringstream iss(sBuffer);
        std::string requestType, channelName, message;

        std::string word;
        int wordCount = 0;
        while (iss >> word) {
            // Remove newline character if present
            if (!word.empty() && word.back() == '\n') {
                word.pop_back();
            }

            if (wordCount == 0) {
                requestType = word;
            } else if (wordCount == 1) {
                channelName = word;
            } else {
                if (!message.empty()) {
                    message += " ";
                }
                message += word;
            }
            ++wordCount;
        }

        nlohmann::json request;
        if(requestType == "create"){
            request = createChannelRequest(channelName);
        }
        if(requestType == "delete"){
            request = deleteChannelRequest(channelName);
        }
        if(requestType == "subscribe"){
            request = subscribeChannelRequest(channelName);
        }
        if(requestType == "unsubscribe"){
            request = unsubscribeChannelRequest(channelName);
        }
        if(requestType == "publish"){
        request = publishRequest(channelName, message);
        }
        if(requestType == "listen"){
            while(true){
               print(receiveFromSocket(sock));
            }
        }

        sendRequest(sock, request);

        print(receiveFromSocket(sock));
    }

    shutdown(sock, SHUT_WR);
    close(sock);

    return EXIT_SUCCESS;
}