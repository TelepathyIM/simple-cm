#include "service.h"

#include <TelepathyQt/BaseConnectionManager>
#include <TelepathyQt/BaseProtocol>
#include <TelepathyQt/Constants>
#include <TelepathyQt/Debug>
#include <TelepathyQt/Types>

#include "protocol.h"

enum class ServiceState {
    Initial,
    Prepared,
    Running,
};

namespace SimpleCM {

class ServicePrivate
{
public:
    Tp::BaseProtocolPtr baseProtocol;
    Tp::BaseConnectionManagerPtr baseCm;

    ServiceState state = ServiceState::Initial;
    QString selfContactId;
    QString cmName;
    QString protocolName;
    SimpleProtocol *protocol = nullptr;
};

Service::Service(QObject *parent)
    : QObject(parent)
    , m_d(new ServicePrivate)
{
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

    Tp::BaseProtocolPtr &baseProtocol = m_d->baseProtocol;
    Tp::BaseConnectionManagerPtr &connectionManager = m_d->baseCm;
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

    // d->protocol->setEnglishName(ui->displayNameEdit->text());
    // d->protocol->setIconName(ui->iconEdit->text());
    // d->protocol->setVCardField(ui->vcardField->text());

    connect(m_d->protocol, &SimpleProtocol::clientSendMessage,
            this, [this](const QString &targetId, const QString &message) {

        Message clientToServiceMessage;
        clientToServiceMessage.to = Peer::fromContactId(targetId);
        clientToServiceMessage.text = message;

        emit messageSent(clientToServiceMessage);
    });


    return m_d->baseCm->registerObject();
}

bool Service::stop()
{
    Q_D(Service);
    if (d->state != ServiceState::Running) {
        return false;
    }

    d->baseCm.reset();
    d->baseProtocol.reset();
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
