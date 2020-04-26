#ifndef SIMPLE_PRESETS_LOADER_HPP
#define SIMPLE_PRESETS_LOADER_HPP

#include "ManagerPreset.hpp"

#include <QList>

class PresetsLoader
{
public:
    static QList<ManagerPreset> presets();
};

#endif // SIMPLE_PRESETS_LOADER_HPP
