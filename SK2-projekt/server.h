#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <vector>
#include <set>
#include <sys/poll.h>
#include <netinet/in.h>
#include "Client.h"
#include "Channel.h"

const int ERROR_EVENTS = POLLERR | POLLNVAL | POLLHUP;
const int INPUT_EVENT = POLLIN;

template <typename T> void print(T obj) { std::cout << obj << std::endl; }

class Server {
public:
    std::vector<pollfd> pollFds;
    std::vector<Client> clients;
    std::vector<Channel> channels;
    int servSock;


    Server(std::string ip, int port);
    Server() = default;
    ~Server() = default;

    void addClient(int fd);
    void removeClient(int fd);
    void onConnect();
    std::string receiveFromSocket(int fd);
    void handleCreateChannel(std::string channel, int clientFd);
    void handleDeleteChannel(std::string channel, int clientFd);
    void handleSubscribeChannel(std::string channel, int clientFd);
    void handleUnsubscribeChannel(std::string channel, int clientFd);
    void handlePublish(std::string channel, std::string message, int clientFd);
    void sendToSocket(int fd, const std::string& message);
    void onInput(int fd);
    void onDisconnect(int fd);
    void handleEvents();
    std::set<int> getSubscribersOf(int channelID);
    Client& getClientByID(int id);
};

#endif // SERVER_H