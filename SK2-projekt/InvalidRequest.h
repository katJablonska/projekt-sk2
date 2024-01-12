#ifndef SK2_PROJEKT_INVALIDREQUEST_H
#define SK2_PROJEKT_INVALIDREQUEST_H

#include <string>

class InvalidRequestException : public std::exception {
private:
    std::string message;

public:
    explicit InvalidRequestException(std::string reason) : message(reason) {}
    auto reason() const noexcept -> const char *  {
        return message.c_str();
    }
};

#endif //SK2_PROJEKT_INVALIDREQUEST_H
