/*
    Copyright (C) 2014 Alexandr Akulich <akulichalexander@gmail.com>

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "protocol.h"
#include "connection.h"

#include <TelepathyQt/BaseConnection>
#include <TelepathyQt/Constants>
#include <TelepathyQt/RequestableChannelClassSpec>
#include <TelepathyQt/RequestableChannelClassSpecList>
#include <TelepathyQt/Types>

#include <QLatin1String>
#include <QVariantMap>

SimpleProtocol::SimpleProtocol(const QDBusConnection &dbusConnection, const QString &name)
    : BaseProtocol(dbusConnection, name)
{
    setParameters(Tp::ProtocolParameterList()
                  << Tp::ProtocolParameter(QLatin1String("self_id"), QLatin1String("s"), Tp::ConnMgrParamFlagRequired));

    setRequestableChannelClasses(Tp::RequestableChannelClassSpecList() << Tp::RequestableChannelClassSpec::textChat());

    // callbacks
    setCreateConnectionCallback(memFun(this, &SimpleProtocol::createConnection));
    setIdentifyAccountCallback(memFun(this, &SimpleProtocol::identifyAccount));
    setNormalizeContactCallback(memFun(this, &SimpleProtocol::normalizeContact));

    addrIface = Tp::BaseProtocolAddressingInterface::create();
    addrIface->setAddressableVCardFields(QStringList() << QLatin1String("x-example-vcard-field"));
    addrIface->setAddressableUriSchemes(QStringList() << QLatin1String("example-uri-scheme"));
    addrIface->setNormalizeVCardAddressCallback(memFun(this, &SimpleProtocol::normalizeVCardAddress));
    addrIface->setNormalizeContactUriCallback(memFun(this, &SimpleProtocol::normalizeContactUri));
    plugInterface(Tp::AbstractProtocolInterfacePtr::dynamicCast(addrIface));

    presenceIface = Tp::BaseProtocolPresenceInterface::create();
    presenceIface->setStatuses(Tp::PresenceSpecList(SimpleConnection::getSimpleStatusSpecMap()));
    plugInterface(Tp::AbstractProtocolInterfacePtr::dynamicCast(presenceIface));
}

SimpleProtocol::~SimpleProtocol()
{
}

QString SimpleProtocol::connectionManagerName() const
{
    return m_connectionManagerName;
}

void SimpleProtocol::setConnectionManagerName(const QString &newName)
{
    m_connectionManagerName = newName;
}

void SimpleProtocol::addMessage(QString sender, QString message)
{
    emit receiveMessage(sender, message);
}

void SimpleProtocol::addContact(const QString &contact)
{
    emit addContactRequested(contact);
}

void SimpleProtocol::setContactList(QStringList list)
{
    emit contactsListChanged(list);
}

void SimpleProtocol::setContactPresence(const QString &identifier, const QString &presence)
{
    emit contactPresenceChanged(identifier, presence);
}

void SimpleProtocol::connectionCreatedEvent(SimpleConnection *newConnection)
{
    connect(this, &SimpleProtocol::receiveMessage,
            newConnection, &SimpleConnection::receiveMessage);
    connect(this, &SimpleProtocol::contactsListChanged,
            newConnection, &SimpleConnection::setContactList);
    connect(this, &SimpleProtocol::addContactRequested ,
            newConnection, &SimpleConnection::addContact);
    connect(this, &SimpleProtocol::contactPresenceChanged,
            newConnection, &SimpleConnection::setContactPresence);

    connect(newConnection, &SimpleConnection::sendMessage,
            this, &SimpleProtocol::clientSendMessage);
}

Tp::BaseConnectionPtr SimpleProtocol::createConnection(const QVariantMap &parameters, Tp::DBusError *error)
{
    Q_UNUSED(error)

    Tp::BaseConnectionPtr newConnection = Tp::BaseConnection::create<SimpleConnection>(m_connectionManagerName, this->name(), parameters);

    SimpleConnection *newSimpleConnection = (SimpleConnection *) newConnection.data();

    connectionCreatedEvent(newSimpleConnection);

    return newConnection;
}

QString SimpleProtocol::identifyAccount(const QVariantMap &parameters, Tp::DBusError *error)
{
    qDebug() << Q_FUNC_INFO << parameters;
    error->set(QLatin1String("IdentifyAccount.Error.NotImplemented"), QLatin1String(""));
    return QString();
}

QString SimpleProtocol::normalizeContact(const QString &contactId, Tp::DBusError *error)
{
    qDebug() << Q_FUNC_INFO << contactId;
    error->set(QLatin1String("NormalizeContact.Error.NotImplemented"), QLatin1String(""));
    return QString();
}

QString SimpleProtocol::normalizeVCardAddress(const QString &vcardField, const QString vcardAddress,
        Tp::DBusError *error)
{
    qDebug() << Q_FUNC_INFO << vcardField << vcardAddress;
    error->set(QLatin1String("NormalizeVCardAddress.Error.NotImplemented"), QLatin1String(""));
    return QString();
}

QString SimpleProtocol::normalizeContactUri(const QString &uri, Tp::DBusError *error)
{
    qDebug() << Q_FUNC_INFO << uri;
    error->set(QLatin1String("NormalizeContactUri.Error.NotImplemented"), QLatin1String(""));
    return QString();
}
