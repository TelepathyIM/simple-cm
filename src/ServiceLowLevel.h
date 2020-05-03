#ifndef SIMPLE_SERVICE_LOW_LEVEL_H
#define SIMPLE_SERVICE_LOW_LEVEL_H

#include <QObject>

#include <TelepathyQt/ServiceTypes>

namespace SimpleCM {

class ServiceLowLevelPrivate;
class ServiceLowLevel : public QObject
{
    Q_OBJECT
public:
    Tp::BaseProtocolPtr getProtocol();
    Tp::BaseConnectionManagerPtr getConnectionManager();

protected:
    explicit ServiceLowLevel(QObject *parent = nullptr);

    ServiceLowLevelPrivate *m_d = nullptr;
    Q_DECLARE_PRIVATE_D(m_d, ServiceLowLevel)
};

} // SimpleCM

#endif // SIMPLE_SERVICE_LOW_LEVEL_H
