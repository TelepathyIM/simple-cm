#ifndef SIMPLE_JSON_UTILS_HPP
#define SIMPLE_JSON_UTILS_HPP

#include <TelepathyQt/Message>

namespace SimpleCM {

class JsonUtils
{
public:
    static QByteArray messageToJson(const Tp::MessagePartList &message);
    static Tp::MessagePartList messageFromJson(const QByteArray &json);
};

} // SimpleCM

#endif // SIMPLE_JSON_UTILS_HPP
