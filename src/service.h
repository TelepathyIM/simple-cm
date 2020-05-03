#ifndef SIMPLESERVICE_H
#define SIMPLESERVICE_H

#include <QObject>

namespace SimpleCM {

struct Peer
{
    enum Type {
        Invalid,
        Contact,
        Room,
    };

    Peer() = default;

    Peer(const QString &id, Type t) : type(t), identifier(id)
    {
    }

    Type type = Type::Invalid;
    QString identifier;

    bool operator==(const Peer &p) const
    {
        return (p.type == type) && (p.identifier == identifier);
    }

    static Peer fromContactId(const QString &id)
    {
        return Peer(id, Type::Contact);
    }

    static Peer fromRoomId(const QString &id)
    {
        return Peer(id, Type::Room);
    }
};

struct Message
{
    Peer to;
    QString from;
    QString text;
};

class ServicePrivate;
class Service : public QObject
{
    Q_OBJECT
public:
    explicit Service(QObject *parent = nullptr);

    bool isRunning() const;

    QString selfContactIdentifier() const;

#if defined(BUILD_SIMPLECM_LIB)
    bool prepare();
#endif

signals:
    void messageSent(const Message &message);

public slots:
    bool start();
    bool stop();

    void setSelfContactIdentifier(const QString &selfId);

    void setManagerName(const QString &name);
    void setProtocolName(const QString &name);

    void addContact(const QString &contact);
    void setContactList(const QStringList &list);
    void setContactPresence(const QString &identifier, const QString &presence);

    void addMessage(const Message &message);

protected:
    ServicePrivate *m_d = nullptr;
    Q_DECLARE_PRIVATE_D(m_d, Service)
};

} // SimpleCM

#endif // SIMPLESERVICE_H
