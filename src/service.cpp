#include "service.h"

#include <TelepathyQt/BaseConnectionManager>
#include <TelepathyQt/BaseProtocol>
#include <TelepathyQt/Constants>
#include <TelepathyQt/Debug>
#include <TelepathyQt/Types>

#include "protocol.h"

namespace SimpleCM {

class ServicePrivate
{
public:
    Tp::BaseProtocolPtr baseProtocol;
    Tp::BaseConnectionManagerPtr baseCm;

    bool running = false;
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
    return d->running;
}

QString Service::selfContactIdentifier() const
{
    Q_D(const Service);
    return d->selfContactId;
}

bool Service::start()
{
    Q_D(Service);
    if (d->running) {
        return false;
    }

    d->running = true;

    d->baseProtocol = Tp::BaseProtocol::create<SimpleProtocol>(QDBusConnection::sessionBus(), d->protocolName);
    d->baseCm = Tp::BaseConnectionManager::create(QDBusConnection::sessionBus(), d->cmName);

    d->protocol = static_cast<SimpleProtocol*>(d->baseProtocol.data());
    d->protocol->setConnectionManagerName(d->cmName);
    // d->protocol->setEnglishName(ui->displayNameEdit->text());
    // d->protocol->setIconName(ui->iconEdit->text());
    // d->protocol->setVCardField(ui->vcardField->text());

    connect(d->protocol, &SimpleProtocol::clientSendMessage,
            this, [this](const QString &targetId, const QString &message) {

        Message clientToServiceMessage;
        clientToServiceMessage.to = Peer::fromContactId(targetId);
        clientToServiceMessage.text = message;

        emit messageSent(clientToServiceMessage);
    });

    d->baseCm->addProtocol(d->baseProtocol);

    return d->baseCm->registerObject();
}

bool Service::stop()
{
    Q_D(Service);
    if (!d->running) {
        return false;
    }

    d->baseCm.reset();
    d->baseProtocol.reset();
    d->protocol = nullptr;

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
