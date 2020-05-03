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

#ifndef SIMPLECM_CONNECTION_H
#define SIMPLECM_CONNECTION_H

#include "simplecm_export.h"

#include <TelepathyQt/BaseConnection>
#include <TelepathyQt/BaseChannel>

class SimpleTextChannel;

typedef Tp::SharedPtr<SimpleTextChannel> SimpleTextChannelPtr;

namespace SimpleCM {

class Chat;
class Message;

} // SimpleCM

class SimpleConnection : public Tp::BaseConnection
{
    Q_OBJECT
public:
    SimpleConnection(const QDBusConnection &dbusConnection,
            const QString &cmName, const QString &protocolName,
            const QVariantMap &parameters);
    ~SimpleConnection();

    static Tp::SimpleStatusSpecMap getSimpleStatusSpecMap();

    void connectCallback(Tp::DBusError *error);
    void onDisconnectRequested();

    QStringList inspectHandles(uint handleType, const Tp::UIntList &handles, Tp::DBusError *error);

    Tp::BaseChannelPtr createChannel(const QVariantMap &request, Tp::DBusError *error);
    SimpleTextChannelPtr ensureTextChannel(const SimpleCM::Chat &chat);

    Tp::UIntList requestHandles(uint handleType, const QStringList &identifiers, Tp::DBusError *error);

    Tp::ContactAttributesMap getContactListAttributes(const QStringList &interfaces, bool hold, Tp::DBusError *error);
    Tp::ContactAttributesMap getContactAttributes(const Tp::UIntList &handles, const QStringList &interfaces, Tp::DBusError *error);

    Tp::SimplePresence getPresence(uint handle);
    uint setPresence(const QString &status, const QString &message, Tp::DBusError *error);

    uint ensureContact(const QString &identifier);

public slots:
    void receiveMessage(const QString &identifier, const QString &message);

    uint addContact(const QString &identifier);
    uint addContacts(const QStringList &identifiers);

    void setContactList(const QStringList &identifiers);
    void setContactPresence(const QString &identifier, const QString &presence);

signals:
    void newMessage(const SimpleCM::Message &message);

protected slots:
    void onChannelSendMessageRequested(const QString &target, const QString &content);

private:
    uint getHandle(const QString &identifier) const;

    void setPresenceState(const QList<uint> &handles, const QString &status);
    void setSubscriptionState(const QStringList &identifiers, const QList<uint> &handles, uint state);

    Tp::BaseConnectionContactsInterfacePtr contactsIface;
    Tp::BaseConnectionSimplePresenceInterfacePtr simplePresenceIface;
    Tp::BaseConnectionContactListInterfacePtr contactListIface;
    Tp::BaseConnectionAddressingInterfacePtr addressingIface;
    Tp::BaseConnectionRequestsInterfacePtr requestsIface;

    Tp::SimpleContactPresences m_presences;

    QMap<uint, QString> m_handles;
    /* Maps a contact handle to its subscription state */
    QHash<uint, uint> m_contactsSubscription;

    QString m_selfId;
};

#endif // SIMPLECM_CONNECTION_H
