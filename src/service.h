#ifndef SIMPLESERVICE_H
#define SIMPLESERVICE_H

#include <QObject>

#include "Chat.hpp"

namespace SimpleCM {

struct Message
{
    Chat to;
    QString from;
    QString text;
};

class ServiceLowLevel;
class ServicePrivate;
class Service : public QObject
{
    Q_OBJECT
public:
    explicit Service(QObject *parent = nullptr);

    bool isRunning() const;

    QString selfContactIdentifier() const;

#if defined(BUILD_SIMPLECM_LIB) || defined(SIMPLECM_ENABLE_LOWLEVEL_API)
    bool prepare();
    ServiceLowLevel *lowLevel();
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
