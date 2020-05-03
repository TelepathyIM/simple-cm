#ifndef SIMPLE_SERVICE_LOW_LEVEL_P_H
#define SIMPLE_SERVICE_LOW_LEVEL_P_H

#include "ServiceLowLevel.h"

namespace SimpleCM {

class ServiceLowLevelPrivate
{
public:
    virtual ~ServiceLowLevelPrivate();

    static ServiceLowLevel *createLowLevel(QObject *parent = nullptr);
    static ServiceLowLevelPrivate *get(ServiceLowLevel *parent);

    Tp::BaseProtocolPtr baseProtocol;
    Tp::BaseConnectionManagerPtr connectionManager;
};

} // SimpleCM

#endif // SIMPLE_SERVICE_LOW_LEVEL_P_H
