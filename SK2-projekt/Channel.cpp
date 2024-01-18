#include "Channel.h"

Channel::Channel(int channelId, const std::string& channelName) : id(channelId), name(channelName) {}

int Channel::getId() const {
    return id;
}

const std::string& Channel::getName() const {
    return name;
}
