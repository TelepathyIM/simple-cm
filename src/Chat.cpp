#include "Chat.hpp"

namespace SimpleCM {

Chat::Chat(const QString &id, Type t)
    : type(t)
    , identifier(id)
{
}

bool Chat::operator==(const Chat &p) const
{
    return (p.type == type) && (p.identifier == identifier);
}

Chat Chat::fromContactId(const QString &id)
{
    return Chat(id, Type::Contact);
}

Chat Chat::fromRoomId(const QString &id)
{
    return Chat(id, Type::Room);
}

} // SimpleCM
