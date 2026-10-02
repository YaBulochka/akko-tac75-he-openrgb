#pragma once

#include "OpenRGBPluginInterface.h"

#include <QObject>
#include <QtPlugin>

#include <string>

class QMenu;
class QWidget;
class RGBController_TAC75HE;

class OpenRGBTAC75HEPlugin final
    : public QObject
    , public OpenRGBPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID OpenRGBPluginInterface_IID FILE "OpenRGBTAC75HEPlugin.json")
    Q_INTERFACES(OpenRGBPluginInterface)

public:
    OpenRGBTAC75HEPlugin();
    ~OpenRGBTAC75HEPlugin() override;

    OpenRGBPluginInfo GetPluginInfo() override;
    unsigned int GetPluginAPIVersion() override;

    void Load(OpenRGBPluginAPIInterface* plugin_api_ptr) override;
    QWidget* GetWidget() override;
    QMenu* GetTrayMenu() override;
    void Unload() override;

    void OnProfileAboutToLoad() override {}
    void OnProfileLoad(nlohmann::json) override {}
    nlohmann::json OnProfileSave() override { return {}; }
    unsigned char* OnSDKCommand(unsigned int, unsigned char*, unsigned int*) override { return nullptr; }
    void ProfileManagerUpdated(unsigned int) override {}
    void ResourceManagerUpdated(unsigned int) override {}
    void SettingsManagerUpdated(unsigned int) override {}

private:
    OpenRGBPluginAPIInterface* plugin_api = nullptr;
    RGBController_TAC75HE* controller = nullptr;
    std::string status = "Not loaded";
};
