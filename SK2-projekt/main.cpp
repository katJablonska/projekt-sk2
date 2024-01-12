#include <iostream>
#include "Server.h"
#include "Client.h"
#include "Channel.h"

int main() {
    std::string ip = "127.0.0.1";
    int port = 8080;

    // Creating server
    auto server = Server(ip, port);

    while (true) {
        server.handleEvents();
    }

    return 0;
}
