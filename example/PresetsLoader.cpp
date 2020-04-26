#include "PresetsLoader.hpp"

QList<ManagerPreset> PresetsLoader::presets()
{
    static QList<ManagerPreset> presets;
    if (presets.isEmpty()) {
        {
            ManagerPreset preset("simplecm", "simplecm");
            preset.protocolDisplayName = "Simple Protocol";
            presets << preset;
        }
        {
            ManagerPreset preset("ofono", "ofono");
            preset.protocolDisplayName = "ofono";
            preset.addressableURISchemes << "tel";
            preset.addressableVCardFields << "tel";
            presets << preset;
        }
        {
            ManagerPreset preset("ring", "tel");
            preset.protocolDisplayName = "Mobile Telephony";
            preset.protocolIcon = "im-tel";
            preset.addressableVCardFields << "TEL";
            presets << preset;
        }
    }

    return presets;
}
