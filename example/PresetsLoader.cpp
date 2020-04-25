#include "PresetsLoader.hpp"

QList<ManagerPreset> PresetsLoader::presets()
{
    static QList<ManagerPreset> presets = {
        ManagerPreset("simplecm", "simplecm"),
        ManagerPreset("ofono", "ofono"),
        ManagerPreset("ring", "tel"),
    };

    return presets;
}
