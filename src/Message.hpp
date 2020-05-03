#ifndef SIMPLE_MESSAGE_HPP
#define SIMPLE_MESSAGE_HPP

#include "Chat.hpp"

namespace SimpleCM {

class SIMPLECM_EXPORT Message
{
public:
    Chat chat;
    QString from;
    QString text;
};

} // SimpleCM

#endif // SIMPLE_MESSAGE_HPP
