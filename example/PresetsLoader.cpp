#include "PresetsLoader.hpp"

QList<ManagerPreset> PresetsLoader::presets()
{
    static QList<ManagerPreset> presets;
    if (presets.isEmpty()) {
        {
            ManagerPreset preset("simplecm", "simplecm");
            presets << preset;
        }
        {
            ManagerPreset preset("ofono", "ofono");
            presets << preset;
        }
        {
            ManagerPreset preset("ring", "tel");
            presets << preset;
        }
    }

    return presets;
}
