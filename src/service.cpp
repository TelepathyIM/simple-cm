#include "service.h"

#include <TelepathyQt/BaseConnectionManager>
#include <TelepathyQt/BaseProtocol>
#include <TelepathyQt/Constants>
#include <TelepathyQt/Debug>
#include <TelepathyQt/Types>

#include "protocol.h"
#include "ServiceLowLevel_p.h"

enum class ServiceState {
    Initial,
    Prepared,
    Running,
};

namespace SimpleCM {

class ServicePrivate
{
public:
    ServiceState state = ServiceState::Initial;
    QString selfContactId;
    QString cmName;
    QString protocolName;
    SimpleProtocol *protocol = nullptr;
    ServiceLowLevel *lowLevel = nullptr;
    ServiceLowLevelPrivate *lowLevelData = nullptr;
};

Service::Service(QObject *parent)
    : QObject(parent)
{
    m_d = new ServicePrivate();
    m_d->lowLevel = ServiceLowLevelPrivate::createLowLevel(this);
    m_d->lowLevelData = ServiceLowLevelPrivate::get(m_d->lowLevel);

    Tp::registerTypes();
    Tp::enableDebug(true);
    Tp::enableWarnings(true);
}

bool Service::isRunning() const
{
    Q_D(const Service);
    return d->state == ServiceState::Running;
}

QString Service::selfContactIdentifier() const
{
    Q_D(const Service);
    return d->selfContactId;
}

ServiceLowLevel *Service::lowLevel()
{
    return m_d->lowLevel;
}

bool Service::prepare()
{
    if (m_d->state != ServiceState::Initial) {
        return false;
    }
    if (m_d->protocolName.isEmpty()) {
        return false;
    }
    if (m_d->cmName.isEmpty()) {
        return false;
    }

    Tp::BaseProtocolPtr &baseProtocol = m_d->lowLevelData->baseProtocol;
    Tp::BaseConnectionManagerPtr &connectionManager = m_d->lowLevelData->connectionManager;
    baseProtocol = Tp::BaseProtocol::create<SimpleProtocol>(QDBusConnection::sessionBus(), m_d->protocolName);
    connectionManager = Tp::BaseConnectionManager::create(QDBusConnection::sessionBus(), m_d->cmName);

    m_d->protocol = static_cast<SimpleProtocol*>(baseProtocol.data());
    m_d->protocol->setConnectionManagerName(m_d->cmName);
    connectionManager->addProtocol(baseProtocol);

    m_d->state = ServiceState::Prepared;
    return true;
}

bool Service::start()
{
    if (m_d->state == ServiceState::Running) {
        return false;
    }

    if (m_d->state != ServiceState::Prepared) {
        if (!prepare()) {
            return false;
        }
    }

    m_d->state = ServiceState::Running;

    connect(m_d->protocol, &SimpleProtocol::clientSendMessage,
            this, [this](const QString &targetId, const QString &message) {

        Message clientToServiceMessage;
        clientToServiceMessage.to = Chat::fromContactId(targetId);
        clientToServiceMessage.text = message;

        emit messageSent(clientToServiceMessage);
    });

    return m_d->lowLevelData->connectionManager->registerObject();
}

bool Service::stop()
{
    Q_D(Service);
    d->lowLevelData->connectionManager.reset();
    d->lowLevelData->baseProtocol.reset();
    d->protocol = nullptr;
    d->state = ServiceState::Initial;

    return true;
}

void Service::setSelfContactIdentifier(const QString &selfId)
{
    Q_D(Service);
    d->selfContactId = selfId;
}

void Service::setManagerName(const QString &name)
{
    Q_D(Service);
    d->cmName = name;
}

void Service::setProtocolName(const QString &name)
{
    Q_D(Service);
    d->protocolName = name;
}

void Service::addContact(const QString &contact)
{
    Q_D(Service);
    d->protocol->addContact(contact);
}

void Service::setContactList(const QStringList &list)
{
    Q_D(Service);
    d->protocol->setContactList(list);
}

void Service::setContactPresence(const QString &identifier, const QString &presence)
{
    Q_D(Service);
    d->protocol->setContactPresence(identifier, presence);
}

void Service::addMessage(const Message &message)
{
    Q_D(Service);
    d->protocol->addMessage(message.to.identifier, message.text);
}

} // SimpleCM
