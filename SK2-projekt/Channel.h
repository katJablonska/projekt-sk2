#ifndef SK2_PROJEKT_CHANNEL_H
#define SK2_PROJEKT_CHANNEL_H

#include <set>
#include <string>

class Channel {
public:
    int id;
    std::string name;

    Channel(int channelId, const std::string& channelName);

    int getId() const;
    const std::string& getName() const;

};

#endif //SK2_PROJEKT_CHANNEL_H
