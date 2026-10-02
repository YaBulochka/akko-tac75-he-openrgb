#include "OpenRGBTAC75HEPlugin.h"

#include "RGBController_TAC75HE.h"
#include "TAC75HEHID.h"

#include <QLabel>
#include <QMenu>
#include <QVBoxLayout>
#include <QWidget>

#include <cstdio>
#include <memory>
#include <utility>

OpenRGBTAC75HEPlugin::OpenRGBTAC75HEPlugin() = default;

OpenRGBTAC75HEPlugin::~OpenRGBTAC75HEPlugin()
{
    Unload();
}

OpenRGBPluginInfo OpenRGBTAC75HEPlugin::GetPluginInfo()
{
    OpenRGBPluginInfo info;

    info.Name        = "Akko TAC75 HE";
    info.Description = "Direct Mode support for the Akko TAC75 HE";
    info.Version     = "1.0.0";
    info.Commit      = "OpenRGB Plugin API 5 / virtual controller";
    info.URL         = "https://github.com/echtzeit-solutions/monsgeek-akko-linux";

    info.Location      = OPENRGB_PLUGIN_LOCATION_TOP;
    info.Label         = "TAC75 HE";
    info.TabIconString = "TAC75 HE";

    return info;
}

unsigned int OpenRGBTAC75HEPlugin::GetPluginAPIVersion()
{
    return OPENRGB_PLUGIN_API_VERSION;
}

void OpenRGBTAC75HEPlugin::Load(OpenRGBPluginAPIInterface* plugin_api_ptr)
{
    if(controller != nullptr || plugin_api_ptr == nullptr)
    {
        return;
    }

    plugin_api = plugin_api_ptr;

    TAC75HEPatchInfo patch;
    std::string error;

    std::unique_ptr<TAC75HEHID> hid =
        TAC75HEHID::OpenAndProbe(patch, error);

    if(!hid)
    {
        status = "TAC75 HE not registered: " + error;

        std::fprintf(
            stderr,
            "[OpenRGBTAC75HEPlugin] %s\n",
            status.c_str()
        );

        return;
    }

    status =
        "Connected to "
        + patch.name
        + " v"
        + std::to_string(patch.version)
        + ", capabilities=0x"
        + [] (std::uint16_t value)
        {
            char buffer[5];
            std::snprintf(buffer, sizeof(buffer), "%04X", value);
            return std::string(buffer);
        }(patch.capabilities);

    std::fprintf(
        stderr,
        "[OpenRGBTAC75HEPlugin] %s\n",
        status.c_str()
    );

    controller = new RGBController_TAC75HE(std::move(hid));

    RGBController_Setup setup = controller->BuildSetup();

    RGBControllerInterface* rgb_controller =
        plugin_api->CreateVirtualRGBController(&setup);

    if(rgb_controller == nullptr)
    {
        status = "Failed to create OpenRGB virtual controller";
        delete controller;
        controller = nullptr;
        plugin_api = nullptr;
        return;
    }

    controller->AttachController(rgb_controller);
    controller->SetRegistered(true);

    plugin_api->RegisterVirtualRGBController(rgb_controller);
}

QWidget* OpenRGBTAC75HEPlugin::GetWidget()
{
    QWidget* widget = new QWidget(nullptr);
    QVBoxLayout* layout = new QVBoxLayout(widget);

    layout->addWidget(
        new QLabel(QString::fromStdString(status), widget)
    );

    return widget;
}

QMenu* OpenRGBTAC75HEPlugin::GetTrayMenu()
{
    return new QMenu("TAC75 HE");
}

void OpenRGBTAC75HEPlugin::Unload()
{
    if(controller != nullptr)
    {
        RGBControllerInterface* rgb_controller = controller->GetController();

        if(plugin_api != nullptr && rgb_controller != nullptr)
        {
            plugin_api->UnregisterVirtualRGBController(rgb_controller);
            plugin_api->DeleteVirtualRGBController(rgb_controller);
        }

        delete controller;
        controller = nullptr;
    }

    plugin_api = nullptr;
}
