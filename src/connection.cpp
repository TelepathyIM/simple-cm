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

#include "connection.h"

#include "Chat.hpp"
#include "textchannel.h"

#include <TelepathyQt/Constants>
#include <TelepathyQt/BaseChannel>

#include <QDateTime>

#include <QDebug>

Tp::SimpleStatusSpecMap SimpleConnection::getSimpleStatusSpecMap()
{
    //Presence
    Tp::SimpleStatusSpec spAvailable;
    spAvailable.type = Tp::ConnectionPresenceTypeAvailable;
    spAvailable.maySetOnSelf = false;
    spAvailable.canHaveMessage = true;

    Tp::SimpleStatusSpec spOffline;
    spOffline.type = Tp::ConnectionPresenceTypeOffline;
    spOffline.maySetOnSelf = false;
    spOffline.canHaveMessage = false;

    Tp::SimpleStatusSpec spUnknown;
    spUnknown.type = Tp::ConnectionPresenceTypeUnknown;
    spUnknown.maySetOnSelf = false;
    spUnknown.canHaveMessage = false;

    Tp::SimpleStatusSpecMap specs;
    specs.insert(QLatin1String("available"), spAvailable);
    specs.insert(QLatin1String("offline"), spOffline);
    specs.insert(QLatin1String("unknown"), spUnknown);
    return specs;
}

SimpleConnection::SimpleConnection(const QDBusConnection &dbusConnection, const QString &cmName, const QString &protocolName, const QVariantMap &parameters) :
    Tp::BaseConnection(dbusConnection, cmName, protocolName, parameters)
{
    /* Connection.Interface.Contacts */
    contactsIface = Tp::BaseConnectionContactsInterface::create();
    contactsIface->setGetContactAttributesCallback(Tp::memFun(this, &SimpleConnection::getContactAttributes));
    contactsIface->setContactAttributeInterfaces(QStringList()
                                                 << TP_QT_IFACE_CONNECTION
                                                 << TP_QT_IFACE_CONNECTION_INTERFACE_CONTACT_LIST
                                                 << TP_QT_IFACE_CONNECTION_INTERFACE_SIMPLE_PRESENCE);
    plugInterface(Tp::AbstractConnectionInterfacePtr::dynamicCast(contactsIface));

    /* Connection.Interface.SimplePresence */
    simplePresenceIface = Tp::BaseConnectionSimplePresenceInterface::create();
    simplePresenceIface->setSetPresenceCallback(Tp::memFun(this,&SimpleConnection::setPresence));
    plugInterface(Tp::AbstractConnectionInterfacePtr::dynamicCast(simplePresenceIface));

    /* Connection.Interface.ContactList */
    contactListIface = Tp::BaseConnectionContactListInterface::create();
    contactListIface->setGetContactListAttributesCallback(Tp::memFun(this, &SimpleConnection::getContactListAttributes));
//    contactListIface->setRequestSubscriptionCallback(Tp::memFun(this, &SimpleConnection::requestSubscription));
    plugInterface(Tp::AbstractConnectionInterfacePtr::dynamicCast(contactListIface));

    /* Connection.Interface.Requests */
    requestsIface = Tp::BaseConnectionRequestsInterface::create(this);
    /* Fill requestableChannelClasses */
    Tp::RequestableChannelClass text;
    text.fixedProperties[TP_QT_IFACE_CHANNEL + QLatin1String(".ChannelType")] = TP_QT_IFACE_CHANNEL_TYPE_TEXT;
    text.fixedProperties[TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandleType")]  = Tp::HandleTypeContact;
    text.allowedProperties.append(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandle"));
    text.allowedProperties.append(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetID"));
    requestsIface->requestableChannelClasses << text;
    plugInterface(Tp::AbstractConnectionInterfacePtr::dynamicCast(requestsIface));

    if (parameters.contains(QLatin1String("self_id"))) {
        m_selfId = parameters.value(QLatin1String("self_id")).toString();
    }

    setSelfHandle(addContact(m_selfId));

    setConnectCallback(Tp::memFun(this, &SimpleConnection::connectCallback));
    setInspectHandlesCallback(Tp::memFun(this, &SimpleConnection::inspectHandles));
    setCreateChannelCallback(Tp::memFun(this, &SimpleConnection::createChannel));
    setRequestHandlesCallback(Tp::memFun(this, &SimpleConnection::requestHandles));
    connect(this, &Tp::BaseConnection::disconnected,
            this, &SimpleConnection::onDisconnectRequested);
}

SimpleConnection::~SimpleConnection()
{
}

void SimpleConnection::connectCallback(Tp::DBusError *error)
{
    setStatus(Tp::ConnectionStatusConnecting, Tp::ConnectionStatusReasonRequested);

    simplePresenceIface->setStatuses(getSimpleStatusSpecMap());

    Tp::SimpleContactPresences presences;
    Tp::SimplePresence presence;
    presence.status = QLatin1String("available");
    presence.statusMessage = QString();
    presence.type = Tp::ConnectionPresenceTypeAvailable;
    presences[selfHandle()] = presence;
    simplePresenceIface->setPresences(presences);

    setStatus(Tp::ConnectionStatusConnected, Tp::ConnectionStatusReasonRequested);

    /* Set ContactList status */
    contactListIface->setContactListState(Tp::ContactListStateSuccess);
}

void SimpleConnection::onDisconnectRequested()
{
    setStatus(Tp::ConnectionStatusDisconnected, Tp::ConnectionStatusReasonRequested);
}

QStringList SimpleConnection::inspectHandles(uint handleType, const Tp::UIntList &handles, Tp::DBusError *error)
{
    qDebug() << Q_FUNC_INFO;

    if (handleType != Tp::HandleTypeContact) {
        error->set(TP_QT_ERROR_INVALID_ARGUMENT, QLatin1String("Unsupported handle type"));
        return QStringList();
    }

    QStringList result;

    foreach (uint handle, handles) {
        if (!m_handles.contains(handle)) {
            return QStringList();
        }

        result.append(m_handles.value(handle));
    }

    return result;
}

Tp::BaseChannelPtr SimpleConnection::createChannel(const QVariantMap &request, Tp::DBusError *error)
{
    const QString channelType = request.value(TP_QT_IFACE_CHANNEL + QLatin1String(".ChannelType")).toString();

    uint targetHandleType = request.value(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandleType")).toUInt();
    uint targetHandle = 0;
    QString targetID;

    switch (targetHandleType) {
    case Tp::HandleTypeContact:
        if (request.contains(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandle"))) {
            targetHandle = request.value(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandle")).toUInt();
            targetID = m_handles.value(targetHandle);
        } else if (request.contains(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetID"))) {
            targetID = request.value(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetID")).toString();
            targetHandle = ensureContact(targetID);
        }
        break;
    default:
        break;
    }

    // Looks like there is no any case for InitiatorID other than selfID
    uint initiatorHandle = 0;

    if (targetHandleType == Tp::HandleTypeContact) {
        initiatorHandle = request.value(TP_QT_IFACE_CHANNEL + QLatin1String(".InitiatorHandle"), selfHandle()).toUInt();
    }

    qDebug() << "SimpleConnection::createChannel " << channelType
             << " " << targetHandleType
             << " " << targetHandle
             << " " << request;

    switch (targetHandleType) {
    case Tp::HandleTypeContact:
        break;
    default:
        if (error) {
            error->set(TP_QT_ERROR_INVALID_ARGUMENT, QLatin1String("Unknown target handle type"));
        }
        return Tp::BaseChannelPtr();
        break;
    }

    if (!targetHandle) {
        if (error) {
            error->set(TP_QT_ERROR_INVALID_HANDLE, QLatin1String("Target handle is unknown."));
        }
        return Tp::BaseChannelPtr();
    }

    Tp::BaseChannelPtr baseChannel = Tp::BaseChannel::create(this, channelType, Tp::HandleType(targetHandleType), targetHandle);
    baseChannel->setTargetID(targetID);
    baseChannel->setInitiatorHandle(initiatorHandle);

    if (channelType == TP_QT_IFACE_CHANNEL_TYPE_TEXT) {
        SimpleTextChannelPtr textChannel = SimpleTextChannel::create(baseChannel.data());
        baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(textChannel));
        connect(textChannel.data(), &SimpleTextChannel::sendMessage, this, &SimpleConnection::sendMessage);
    }

    return baseChannel;
}

SimpleTextChannelPtr SimpleConnection::ensureTextChannel(const SimpleCM::Chat &chat)
{
    uint initiatorHandle = 0;
    uint targetHandle = 0;
    Tp::HandleType targetHandleType = Tp::HandleTypeNone;

    if (chat.type == SimpleCM::Chat::Contact) {
        targetHandleType = Tp::HandleTypeContact;
        targetHandle = ensureContact(chat.identifier);
        initiatorHandle = targetHandle;
    } else {
        return SimpleTextChannelPtr();
    }

    Tp::DBusError error;
    bool yours;

    QVariantMap request;
    request[TP_QT_IFACE_CHANNEL + QLatin1String(".ChannelType")] = TP_QT_IFACE_CHANNEL_TYPE_TEXT;
    request[TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandle")] = targetHandle;
    request[TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandleType")] = targetHandleType;
    request[TP_QT_IFACE_CHANNEL + QLatin1String(".InitiatorHandle")] = initiatorHandle;

    Tp::BaseChannelPtr channel = ensureChannel(request, yours, /* suppressHandler */ false, &error);
    if (error.isValid()) {
        qWarning() << Q_FUNC_INFO << "ensureChannel failed:" << error.name() << " " << error.message();
        return SimpleTextChannelPtr();
    }

    return SimpleTextChannelPtr::dynamicCast(channel->interface(TP_QT_IFACE_CHANNEL_TYPE_TEXT));
}

Tp::UIntList SimpleConnection::requestHandles(uint handleType, const QStringList &identifiers, Tp::DBusError *error)
{
    qDebug() << Q_FUNC_INFO << identifiers;

    Tp::UIntList result;

    if (handleType != Tp::HandleTypeContact) {
        error->set(TP_QT_ERROR_INVALID_ARGUMENT, QLatin1String("SimpleConnection::requestHandles - Handle Type unknown"));
        return result;
    }

    foreach(const QString &identify, identifiers) {
        result.append(ensureContact(identify));
    }

    return result;
}

Tp::ContactAttributesMap SimpleConnection::getContactListAttributes(const QStringList &interfaces, bool hold, Tp::DBusError *error)
{
    qDebug() << Q_FUNC_INFO;

    Tp::ContactAttributesMap contactAttributes;

    foreach (const uint handle, m_handles.keys()) {
        if (handle == selfHandle()) {
            continue;
        }
        QVariantMap attributes;
        attributes[TP_QT_IFACE_CONNECTION + QLatin1String("/contact-id")] = m_handles.value(handle);
        attributes[TP_QT_IFACE_CONNECTION_INTERFACE_CONTACT_LIST + QLatin1String("/subscribe")] = Tp::SubscriptionStateYes;
        attributes[TP_QT_IFACE_CONNECTION_INTERFACE_CONTACT_LIST + QLatin1String("/publish")] = Tp::SubscriptionStateYes;
        attributes[TP_QT_IFACE_CONNECTION_INTERFACE_SIMPLE_PRESENCE + QLatin1String("/presence")] = QVariant::fromValue(getPresence(handle));
        contactAttributes[handle] = attributes;
    }
    return contactAttributes;
}

Tp::ContactAttributesMap SimpleConnection::getContactAttributes(const Tp::UIntList &handles, const QStringList &interfaces, Tp::DBusError *error)
{
//    Connection.Interface.Contacts
//    http://telepathy.freedesktop.org/spec/Connection_Interface_Contacts.html#Method:GetContactAttributes
    qDebug() << Q_FUNC_INFO << handles;

    Tp::ContactAttributesMap contactAttributes;

    foreach (const uint handle, handles) {
        if (m_handles.contains(handle)){
            QVariantMap attributes;
            attributes[TP_QT_IFACE_CONNECTION + QLatin1String("/contact-id")] = m_handles.value(handle);

            if (interfaces.contains(TP_QT_IFACE_CONNECTION_INTERFACE_CONTACT_LIST)) {
                attributes[TP_QT_IFACE_CONNECTION_INTERFACE_CONTACT_LIST + QLatin1String("/subscribe")] = Tp::SubscriptionStateYes;
                attributes[TP_QT_IFACE_CONNECTION_INTERFACE_CONTACT_LIST + QLatin1String("/publish")] = Tp::SubscriptionStateYes;
            }

            if (interfaces.contains(TP_QT_IFACE_CONNECTION_INTERFACE_SIMPLE_PRESENCE)) {
                attributes[TP_QT_IFACE_CONNECTION_INTERFACE_SIMPLE_PRESENCE + QLatin1String("/presence")] = QVariant::fromValue(getPresence(handle));
            }
            contactAttributes[handle] = attributes;
        }
    }
    return contactAttributes;
}

Tp::SimplePresence SimpleConnection::getPresence(uint handle)
{
    if (!m_presences.contains(handle)) {
        return Tp::SimplePresence();
    }

    return m_presences.value(handle);
}

uint SimpleConnection::setPresence(const QString &status, const QString &message, Tp::DBusError *error)
{
    qDebug() << Q_FUNC_INFO << "not implemented";
    return selfHandle();
}

uint SimpleConnection::ensureContact(const QString &identifier)
{
    uint handle = getHandle(identifier);
    if (!handle) {
        handle = addContact(identifier);
    }
    return handle;
}

uint SimpleConnection::addContacts(const QStringList &identifiers)
{
    qDebug() << Q_FUNC_INFO;
    uint handle = 0;

    if (!m_handles.isEmpty()) {
        handle = m_handles.keys().last();
    }

    QList<uint> newHandles;
    foreach(const QString &identifier, identifiers) {
        ++handle;
        m_handles.insert(handle, identifier);
        newHandles << handle;
    }

    setPresenceState(newHandles, QLatin1String("unknown"));
    setSubscriptionState(identifiers, newHandles, Tp::SubscriptionStateUnknown);

    return handle;
}

uint SimpleConnection::addContact(const QString &identifier)
{
    qDebug() << Q_FUNC_INFO;
    return addContacts(QStringList() << identifier);
}

void SimpleConnection::setPresenceState(const QList<uint> &handles, const QString &status)
{
    qDebug() << Q_FUNC_INFO;
    Tp::SimpleContactPresences newPresences;
    const static Tp::SimpleStatusSpecMap statusSpecMap = getSimpleStatusSpecMap();
    foreach (uint handle, handles) {
        uint type = 0;
        if (statusSpecMap.contains(status)) {
            type = statusSpecMap.value(status).type;
        }

        Tp::SimplePresence presence;
        presence.status = status;
//        presence.statusMessage;
        presence.type = type;
        m_presences[handle] = presence;
        newPresences[handle] = presence;
    }
    simplePresenceIface->setPresences(newPresences);
}

void SimpleConnection::setSubscriptionState(const QStringList &identifiers, const QList<uint> &handles, uint state)
{
    qDebug() << Q_FUNC_INFO;
    Tp::ContactSubscriptionMap changes;
    Tp::HandleIdentifierMap identifiersMap;

    for(int i = 0; i < identifiers.size(); ++i) {
        Tp::ContactSubscriptions change;
        change.publish = Tp::SubscriptionStateYes;
        change.publishRequest = QString();
        change.subscribe = state;
        changes[handles[i]] = change;
        identifiersMap[handles[i]] = identifiers[i];
        m_contactsSubscription[handles[i]] = state;
    }
    Tp::HandleIdentifierMap removals;
    contactListIface->contactsChangedWithID(changes, identifiersMap, removals);
}

/* Receive message from someone to ourself */
void SimpleConnection::receiveMessage(const QString &identifier, const QString &message)
{
    SimpleTextChannelPtr textChannel = ensureTextChannel(SimpleCM::Chat::fromContactId(identifier));

    if (!textChannel) {
        qDebug() << Q_FUNC_INFO << "Error: channel is not a SimpleTextChannel?";
        return;
    }

    textChannel->addIncomingMessage(message);
}

void SimpleConnection::setContactList(const QStringList &identifiers)
{
    // Actually it don't clear previous list (not implemented yet)
    addContacts(identifiers);

//    Tp::ContactSubscriptionMap changes;
//    Tp::HandleIdentifierMap identifiers;
//    Tp::HandleIdentifierMap removals;

    QList<uint> handles;

    for (int i = 0; i < identifiers.count(); ++i) {
        handles.append(ensureContact(identifiers.at(i)));
    }

    setSubscriptionState(identifiers, handles, Tp::SubscriptionStateYes);
}

void SimpleConnection::setContactPresence(const QString &identifier, const QString &presence)
{
    uint handle = ensureContact(identifier);
    setPresenceState(QList<uint>() << handle, presence);

    // Let it be here until proper subscription implementation
    if (handle != selfHandle()) {
        setSubscriptionState(QStringList() << identifier, QList<uint>() << handle, Tp::SubscriptionStateYes);
    }
}

uint SimpleConnection::getHandle(const QString &identifier) const
{
    return m_handles.key(identifier, 0);
}
