#include "ServiceLowLevel_p.h"

#include "Chat.hpp"
#include "connection.h"
#include "JsonUtils.hpp"
#include "protocol.h"
#include "textchannel.h"

#include <TelepathyQt/BaseConnectionManager>
#include <TelepathyQt/BaseProtocol>

namespace SimpleCM {

Tp::BaseProtocolPtr ServiceLowLevel::getProtocol()
{
    return m_d->baseProtocol;
}

Tp::BaseConnectionManagerPtr ServiceLowLevel::getConnectionManager()
{
    return m_d->connectionManager;
}

void ServiceLowLevel::sendJsonMessage(const Chat &target, const QByteArray &json)
{
    SimpleProtocolPtr protocol = SimpleProtocolPtr::dynamicCast(m_d->baseProtocol);
    if (!protocol) {
        return;
    }

    SimpleConnectionPtr connection = protocol->getConnection();
    if (!connection) {
        return;
    }

    SimpleTextChannelPtr textChannel = connection->ensureTextChannel(target);
    if (!textChannel) {
        return;
    }

    Tp::MessagePartList partList = JsonUtils::messageFromJson(json);
    if (partList.isEmpty()) {
        return;
    }

    textChannel->addReceivedMessage(partList);
}

ServiceLowLevel::ServiceLowLevel(QObject *parent)
    : QObject(parent)
    , m_d(new ServiceLowLevelPrivate)
{
}

ServiceLowLevelPrivate::~ServiceLowLevelPrivate()
{
}

ServiceLowLevel *ServiceLowLevelPrivate::createLowLevel(QObject *parent)
{
    return new ServiceLowLevel(parent);
}

ServiceLowLevelPrivate *ServiceLowLevelPrivate::get(ServiceLowLevel *parent)
{
    return parent->m_d;
}

} // SimpleCM
