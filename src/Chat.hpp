#ifndef SIMPLE_CHAT_HPP
#define SIMPLE_CHAT_HPP

#include <QString>
#include "simplecm_export.h"

namespace SimpleCM {

class SIMPLECM_EXPORT Chat
{
public:
    enum Type {
        Invalid,
        Contact,
        Room,
    };

    Chat() = default;
    Chat(const QString &id, Type t);

    bool operator==(const Chat &p) const;

    static Chat fromContactId(const QString &id);
    static Chat fromRoomId(const QString &id);

    Type type = Type::Invalid;
    QString identifier;
};

} // SimpleCM

#endif // SIMPLE_CHAT_HPP
