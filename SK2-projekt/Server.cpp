#include "Server.h"
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <iterator>
#include <string>
#include "json.hpp"
#include "InvalidRequest.h"

const int one = 1;
int nextChannelId = 0;

Server::Server(std::string ip, int port){
    sockaddr_in localAddress{
            .sin_family = AF_INET,
            .sin_port   = htons(port),
            .sin_addr   = {inet_addr(ip.c_str())}
    };

    servSock = ::socket(AF_INET, SOCK_STREAM, 0);
    if(servSock == -1) {
        perror("Failed to create socket (socket)");
        exit(1);
    }

    if(setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
        perror("Error (setsockopt)");
        exit(1);
    }

    if(bind(servSock, (sockaddr*) &localAddress, sizeof(localAddress)) == -1) {
        perror("Failed to assign address (bind)");
        exit(1);
    }

    if(listen(servSock, SOMAXCONN) == -1) {
        perror("Error (listen)");
        exit(1);
    }

    pollfd pfd = {servSock, INPUT_EVENT | ERROR_EVENTS, 0};
    this->pollFds.push_back(pfd);
}

void Server::addClient(int fd) {
    this -> clients.push_back(Client(fd));
    pollfd pfd = {fd, INPUT_EVENT | ERROR_EVENTS, 0};
    this->pollFds.push_back(pfd);
}

std::set<int> Server::getSubscribersOf(int channelID){
    std::set<int> channelSubscribers;
    for (const auto& client : this -> clients) {
        if (client.hasSubscribed(channelID)) {
            channelSubscribers.insert(client.fd);
        }
    }
    return channelSubscribers;
}

Client& Server::getClientByID(int id){
    auto it = std::find_if(clients.begin(), clients.end(),
                               [&id](const Client& c) { return c.fd == id; });
    int idx = std::distance(clients.begin(), it);
    return this->clients[idx];
}

void Server::removeClient(int fd) {
    erase_if(this->clients, [&](Client& c) {return c.fd == fd;});
    // usuwa pollfd, gdzie fd jest równe usuwanemu fd
    erase_if(this->pollFds, [&](pollfd& pfd) { return pfd.fd == fd; });
    shutdown(fd, SHUT_RDWR);
    close(fd);
}

void Server::onConnect() {
    int clientFd = accept(servSock, nullptr, nullptr);
    if (clientFd == -1) {
        perror("Failed to accept connection (accept)");
        exit(1);
    }
    addClient(clientFd);
}

std::string Server::receiveFromSocket(int fd) {
    unsigned int messageSize = 0;
    int bytes = recv(fd, &messageSize, sizeof(messageSize), MSG_WAITALL);

    messageSize = ntohl(messageSize);

    if (bytes != sizeof(messageSize) && bytes != 0) {
        perror("Error (read)");
        exit(1);
    }

    auto buffer = std::string(messageSize, 0);

    int messageBytes = recv(fd, buffer.data(), messageSize, MSG_WAITALL);

    if (messageBytes != (int)messageSize && messageBytes != 0) {
        perror("Error (read)");
        exit(1);
    }

    if (bytes == 0 || messageBytes == 0) {
        perror("Socket already closed");
        exit(1);
    }

    return buffer;
}

void Server::handleCreateChannel(std::string channel, int clientFd) {
    auto it = std::find_if(channels.begin(), channels.end(),
                           [&channel](const Channel& ch) { return ch.name == channel; });

    if (it != channels.end()) {
        print("Channel  " + channel + " already exists.");
        sendToSocket(clientFd, "Failed to create channel. Channel already exists.\n");
    } else {
        channels.push_back(Channel(nextChannelId++, channel));
        print("Channel created: " + channel);
        sendToSocket(clientFd, "Channel created.\n");
    }
}

void Server::handleDeleteChannel(std::string channel, int clientFd) {
    auto it = std::find_if(channels.begin(), channels.end(),
                           [&channel](const Channel& ch) { return ch.name == channel; });

    if (it != channels.end()) {
        channels.erase(it);
        print("Channel deleted: " + channel);
        sendToSocket(clientFd, "Channel deleted.\n");
    } else {
        print("Channel not found: " + channel);
        sendToSocket(clientFd, "Failed to delete channel. Channel not found.\n");
    }
}

void Server::handleSubscribeChannel(std::string channel, int clientFd) {
    auto it = std::find_if(channels.begin(), channels.end(),
                           [&channel](const Channel& ch) { return ch.name == channel; });

    auto& client = getClientByID(clientFd);
    if (it != channels.end()) {
        client.addSub(it->id);
        print("Client " + std::to_string(clientFd) + " subscribed to channel: " + channel);
        sendToSocket(clientFd, "Channel subscribed.\n");
    } else {
        print("Subscription failed. Channel not found.");
        sendToSocket(clientFd, "Subscribing failed. Channel not found.\n");
    }
}

void Server::handleUnsubscribeChannel(std::string channel, int clientFd) {
    auto it = std::find_if(channels.begin(), channels.end(),
                           [&channel](const Channel& ch) { return ch.name == channel; });

    auto& client = getClientByID(clientFd);

    if (it != channels.end()) {
        client.removeSub(it->id);
        print("Client " + std::to_string(clientFd) + " unsubscribed from channel: " + channel);
        sendToSocket(clientFd, "Channel unsubscribed.\n");
    } else {
        print("Unsubscription failed. Channel or client not found.");
        sendToSocket(clientFd, "Unsubscribing failed. Channel not found.\n");
    }
}

void Server::handlePublish(std::string channel, std::string message, int clientFd) {
    auto it = std::find_if(channels.begin(), channels.end(),
                           [&channel](const Channel& ch) { return ch.name == channel; });

    if (it != channels.end()) {
        print("Message published to channel " + channel + ": " + message);
        sendToSocket(clientFd, "Message published.\n");
        for (int subscriber : getSubscribersOf(it->id)) {
            sendToSocket(subscriber, message);
        }

    } else {
        print("Publishing failed. Channel not found: " + channel);
        sendToSocket(clientFd, "Publish failed. Channel not found.\n");
    }
}

void Server::sendToSocket(int fd, const std::string& message) {
    unsigned int messageSize = htonl(message.size());
    if (send(fd, &messageSize, sizeof(messageSize), 0) == -1) {
        perror("Error sending message size");
        exit(1);
    }

    if (send(fd, message.c_str(), message.size(), 0) == -1) {
        perror("Error sending message");
        exit(1);
    }
}

void Server::onInput(int fd) {
    nlohmann::json request = nlohmann::json::parse(receiveFromSocket(fd), nullptr, false);
    switch (request["requestType"].get<int>()) {
        case 1:
            handleCreateChannel(request["channel"].get<std::string>(), fd);
            break;
        case 2:
            handleDeleteChannel(request["channel"].get<std::string>(), fd);
            break;
        case 3:
            handleSubscribeChannel(request["channel"].get<std::string>(), fd);
            break;
        case 4:
            handleUnsubscribeChannel(request["channel"].get<std::string>(), fd);
            break;
        case 5:
            handlePublish(request["channel"].get<std::string>(), request["message"].get<std::string>(), fd);
            break;
        default:
            print("Invalid request type");
    }
}

void Server::onDisconnect(int fd) {
    removeClient(fd);
}

void Server::handleEvents() {
    int p = poll(this->pollFds.data(), this->pollFds.size(), -1);
    if(p == -1){
        perror("Blad przy funkcji poll");
        exit(1);
    }

    //dla kazdego pollfd w wektorze pollFds sprawdzamy co sie stalo
    for(pollfd &pfd : this -> pollFds){
        //jezeli nic sie nie wydarzylo (revents = 0)
        if (!pfd.revents) {
            continue;
        }

        // czy input event byl na serwerze czy kliencie?
        // laczy sie nowy klient
        // sprawdza czy pfd na ktore wyslano dane to gniazdo serwera
        if((pfd.revents & INPUT_EVENT) && pfd.fd == this->servSock){
            onConnect();
            break;
        }

        if(pfd.revents & ERROR_EVENTS){
            // rozlaczenie klienta
            onDisconnect(pfd.fd);
            print("Client disconected.");
            break;
        }

        if(pfd.revents & INPUT_EVENT){
            //otrzymanie danych od klienta
            try{
                onInput(pfd.fd);
            }
            catch(const InvalidRequestException& e){
                sendToSocket(pfd.fd, "Error. Invalid request." + std::string(e.what()));
                continue;
            }
            catch(const std::exception& e){
                sendToSocket(pfd.fd, "Server error.\n");
                continue;
            }
            break;
        }
    }
}