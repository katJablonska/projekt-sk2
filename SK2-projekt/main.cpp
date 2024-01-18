#include <iostream>
#include "Server.h"
#include "Client.h"
#include "Channel.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        print("Invalid number of arguments: " + std::to_string(argc));
    }
    std::string ip = argv[1];
    int port = std::atoi(argv[2]);

    // Creating server
    auto server = Server(ip, port);
    print("Listening on " + ip + ":" + std::to_string(port));

    while (true) {
        server.handleEvents();
    }

    return 0;
}
