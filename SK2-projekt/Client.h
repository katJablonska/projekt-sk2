#ifndef SK2_PROJEKT_CLIENT_H
#define SK2_PROJEKT_CLIENT_H

#include <set>

class Client {
public:
    int fd;
    std::set<int> subscriptions;

    Client() = default;
    Client(int fd);

    void addSub(int channelId);
    void removeSub(int channelId);
    const std::set<int>& getSubs() const;
    bool hasSubscribed(int channelId) const;
};

#endif //SK2_PROJEKT_CLIENT_H
