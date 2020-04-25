#ifndef SIMPLE_MANAGER_PRESET_HPP
#define SIMPLE_MANAGER_PRESET_HPP

#include <QString>

struct ManagerPreset
{
    ManagerPreset() = default;
    ManagerPreset(const QString &name, const QString &protocol)
    {
        this->name = name;
        this->protocol = protocol;
    }

    QString name;
    QString protocol;
};

#endif // SIMPLE_MANAGER_PRESET_HPP
