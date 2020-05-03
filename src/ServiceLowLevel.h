#ifndef SIMPLE_SERVICE_LOW_LEVEL_H
#define SIMPLE_SERVICE_LOW_LEVEL_H

#include <QObject>

#include <TelepathyQt/ServiceTypes>

namespace SimpleCM {

class Chat;

class ServiceLowLevelPrivate;
class ServiceLowLevel : public QObject
{
    Q_OBJECT
public:
    Tp::BaseProtocolPtr getProtocol();
    Tp::BaseConnectionManagerPtr getConnectionManager();

    void sendJsonMessage(const Chat &target, const QByteArray &json);

protected:
    explicit ServiceLowLevel(QObject *parent = nullptr);

    ServiceLowLevelPrivate *m_d = nullptr;
    Q_DECLARE_PRIVATE_D(m_d, ServiceLowLevel)
};

} // SimpleCM

#endif // SIMPLE_SERVICE_LOW_LEVEL_H
