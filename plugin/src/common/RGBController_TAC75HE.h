#pragma once

#include "RGBControllerInterface.h"

#include <memory>

class TAC75HEHID;

class RGBController_TAC75HE
{
public:
    explicit RGBController_TAC75HE(
        std::unique_ptr<TAC75HEHID> hid_transport
    );

    ~RGBController_TAC75HE();

    RGBController_Setup BuildSetup();
    void AttachController(RGBControllerInterface* controller_ptr);
    RGBControllerInterface* GetController() const;
    void SetRegistered(bool registered);

    static void DeviceConfigureZone(void* object, int zone);
    static void DeviceUpdateLEDs(void* object);
    static void DeviceUpdateZoneLEDs(void* object, int zone);
    static void DeviceUpdateSingleLED(void* object, int led);
    static void DeviceUpdateMode(void* object);

private:
    void UpdateLEDs();

    std::unique_ptr<TAC75HEHID> hid;
    RGBControllerInterface* controller = nullptr;
    bool registered = false;
    bool io_error_reported = false;
};
