#include "Client.h"
#include "server.h"

Client::Client(int fd) : fd(fd) {}

void Client::addSub(int channelId) {
    subscriptions.insert(channelId);
}

void Client::removeSub(int channelId) {
    subscriptions.erase(channelId);
}

const std::set<int>& Client::getSubs() const {
    return subscriptions;
}

bool Client::hasSubscribed(int channelId) const {
    return subscriptions.find(channelId) != subscriptions.end();
}
