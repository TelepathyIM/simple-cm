#include "ServiceLowLevel_p.h"

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
