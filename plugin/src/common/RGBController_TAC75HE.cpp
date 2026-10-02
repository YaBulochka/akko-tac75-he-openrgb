#include "RGBController_TAC75HE.h"
#include "TAC75HEHID.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

namespace
{
constexpr unsigned int NA              = 0xFFFFFFFFu;
constexpr unsigned int TAC75_LED_COUNT = 83;

/*
 * OpenRGB visual layout: 6 rows x 19 columns.
 *
 * The firmware transport still uses its own internal 6x16 layout.
 * This wider map exists only to make LED View resemble the real keyboard.
 */
unsigned int tac75_matrix_map[6][16] =
{
    /* Esc | F1-F12 | Delete Insert */
    {
        0x00, NA,   0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E
    },

    /* ` 1 2 3 4 5 6 7 8 9 0 - = Backspace | Home */
    {
        0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16,
        0x15, 0x14, 0x13, 0x12, 0x11, 0x10, NA,   0x0F
    },

    /* Tab Q W E R T Y U I O P [ ] \ | Page Up */
    {
        0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
        0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, NA,   0x2C
    },

    /* Caps A S D F G H J K L ; ' Enter | Page Down */
    {
        0x3A, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33,
        0x32, 0x31, 0x30, 0x2F, 0x2E, NA,   NA,   0x2D
    },

    /* Shift Z X C V B N M , . / Shift | Up | End */
    {
        0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42,
        0x43, 0x44, 0x45, 0x46, NA,   0x47, NA,   0x48
    },

    /* Ctrl Win Alt | Space | Alt Fn Ctrl | Left Down Right */
    {
        0x52, 0x51, 0x50, NA,   NA,   0x4F, NA,   NA,
        0x4E, 0x4D, 0x4C, NA,   0x4B, 0x4A, 0x49, NA
    }
};

/*
 * Actual E8 transport grid.
 *
 * Each entry is the physical OpenRGB LED index stored at that 6x16
 * firmware position. NA marks one of the thirteen unused positions.
 *
 * Do not confuse this with tac75_matrix_map above: that map is only
 * the compact visual layout shown by OpenRGB.
 */
constexpr unsigned int tac75_transport_map[96] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, NA,

    0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16,
    0x15, 0x14, 0x13, 0x12, 0x11, 0x10, 0x0F, NA,

    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, NA,

    0x3A, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33,
    0x32, 0x31, 0x30, 0x2F, NA,   0x2E, 0x2D, NA,

    0x3B, NA,   0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41,
    0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, NA,

    0x52, NA,   0x51, 0x50, NA,   NA,   0x4F, NA,
    NA,   0x4E, 0x4D, 0x4C, 0x4B, 0x4A, 0x49, NA
};

constexpr std::uint64_t POWER_BUDGET_MA = 3000;
constexpr std::uint64_t CHANNEL_FULL_MA = 20;

/*
 * Sum of all R+G+B byte values allowed by the current budget.
 * At 20 mA per full channel:
 *
 *     sum * 20 / 255 <= 3000
 */
constexpr std::uint64_t MAX_CHANNEL_SUM =
    POWER_BUDGET_MA * 255 / CHANNEL_FULL_MA;


/*
 * Names are indexed by the physical LED number used by the firmware.
 */
const char* tac75_led_names[TAC75_LED_COUNT] =
{
    "Escape",       /* 00 */
    "F1",           /* 01 */
    "F2",           /* 02 */
    "F3",           /* 03 */
    "F4",           /* 04 */
    "F5",           /* 05 */
    "F6",           /* 06 */
    "F7",           /* 07 */
    "F8",           /* 08 */
    "F9",           /* 09 */
    "F10",          /* 0A */
    "F11",          /* 0B */
    "F12",          /* 0C */
    "Delete",       /* 0D */
    "Insert",       /* 0E */

    "Home",         /* 0F */
    "Backspace",    /* 10 */
    "Equals",       /* 11 */
    "Minus",        /* 12 */
    "0",            /* 13 */
    "9",            /* 14 */
    "8",            /* 15 */
    "7",            /* 16 */
    "6",            /* 17 */
    "5",            /* 18 */
    "4",            /* 19 */
    "3",            /* 1A */
    "2",            /* 1B */
    "1",            /* 1C */
    "Back Tick",    /* 1D */

    "Tab",          /* 1E */
    "Q",            /* 1F */
    "W",            /* 20 */
    "E",            /* 21 */
    "R",            /* 22 */
    "T",            /* 23 */
    "Y",            /* 24 */
    "U",            /* 25 */
    "I",            /* 26 */
    "O",            /* 27 */
    "P",            /* 28 */
    "Left Bracket", /* 29 */
    "Right Bracket",/* 2A */
    "Backslash",    /* 2B */
    "Page Up",      /* 2C */

    "Page Down",    /* 2D */
    "Enter",        /* 2E */
    "Apostrophe",   /* 2F */
    "Semicolon",    /* 30 */
    "L",            /* 31 */
    "K",            /* 32 */
    "J",            /* 33 */
    "H",            /* 34 */
    "G",            /* 35 */
    "F",            /* 36 */
    "D",            /* 37 */
    "S",            /* 38 */
    "A",            /* 39 */
    "Caps Lock",    /* 3A */

    "Left Shift",   /* 3B */
    "Z",            /* 3C */
    "X",            /* 3D */
    "C",            /* 3E */
    "V",            /* 3F */
    "B",            /* 40 */
    "N",            /* 41 */
    "M",            /* 42 */
    "Comma",        /* 43 */
    "Period",       /* 44 */
    "Slash",        /* 45 */
    "Right Shift",  /* 46 */
    "Up",           /* 47 */
    "End",          /* 48 */

    "Right",        /* 49 */
    "Down",         /* 4A */
    "Left",         /* 4B */
    "Right Control",/* 4C */
    "Fn",           /* 4D */
    "Right Alt",    /* 4E */
    "Space",        /* 4F */
    "Left Alt",     /* 50 */
    "Left Windows", /* 51 */
    "Left Control"  /* 52 */
};
}


RGBController_TAC75HE::RGBController_TAC75HE(
    std::unique_ptr<TAC75HEHID> hid_transport
)
    : hid(std::move(hid_transport))
{
}

RGBController_TAC75HE::~RGBController_TAC75HE() = default;

RGBController_Setup RGBController_TAC75HE::BuildSetup()
{
    RGBController_Setup setup{};

    setup.name        = "Akko TAC75 HE";
    setup.vendor      = "Akko";
    setup.description = "TAC75 HE Direct Mode plugin";
    setup.version     = "TAC75DM v" + std::to_string(hid->PatchInfo().version);
    setup.location    = hid->Path();
    setup.type        = DEVICE_TYPE_KEYBOARD;
    setup.flags       = CONTROLLER_FLAG_VIRTUAL;
    setup.active_mode = 0;
    setup.object_ptr  = this;

    mode direct_mode;
    direct_mode.name           = "Direct";
    direct_mode.value          = 0;
    direct_mode.flags          = MODE_FLAG_HAS_PER_LED_COLOR
                               | MODE_FLAG_HAS_BRIGHTNESS;
    direct_mode.color_mode     = MODE_COLORS_PER_LED;
    direct_mode.brightness_min = 0;
    direct_mode.brightness_max = 100;
    direct_mode.brightness     = 100;
    setup.modes.push_back(direct_mode);

    zone keyboard_zone;
    keyboard_zone.name       = "Keyboard";
    keyboard_zone.type       = ZONE_TYPE_MATRIX;
    keyboard_zone.leds_min   = TAC75_LED_COUNT;
    keyboard_zone.leds_max   = TAC75_LED_COUNT;
    keyboard_zone.leds_count = TAC75_LED_COUNT;
    keyboard_zone.matrix_map.Set(6, 16, &tac75_matrix_map[0][0]);
    setup.zones.push_back(keyboard_zone);

    setup.leds.reserve(TAC75_LED_COUNT);

    for(unsigned int led_index = 0;
        led_index < TAC75_LED_COUNT;
        led_index++)
    {
        led new_led;
        new_led.name  = tac75_led_names[led_index];
        new_led.value = led_index;
        setup.leds.push_back(new_led);
    }

    setup.DeviceConfigureZone       = &RGBController_TAC75HE::DeviceConfigureZone;
    setup.DeviceUpdateLEDs          = &RGBController_TAC75HE::DeviceUpdateLEDs;
    setup.DeviceUpdateZoneLEDs     = &RGBController_TAC75HE::DeviceUpdateZoneLEDs;
    setup.DeviceUpdateSingleLED    = &RGBController_TAC75HE::DeviceUpdateSingleLED;
    setup.DeviceUpdateMode         = &RGBController_TAC75HE::DeviceUpdateMode;
    setup.DeviceSaveMode           = nullptr;
    setup.DeviceUpdateZoneMode     = nullptr;
    setup.DeviceUpdateDeviceSpecificConfiguration = nullptr;
    setup.DeviceUpdateDeviceSpecificZoneConfiguration = nullptr;

    return setup;
}

void RGBController_TAC75HE::AttachController(
    RGBControllerInterface* controller_ptr
)
{
    controller = controller_ptr;
}

RGBControllerInterface* RGBController_TAC75HE::GetController() const
{
    return controller;
}

void RGBController_TAC75HE::SetRegistered(bool value)
{
    registered = value;
}

void RGBController_TAC75HE::DeviceConfigureZone(void*, int)
{
    /* Fixed-size hardware zone. */
}

void RGBController_TAC75HE::DeviceUpdateLEDs(void* object)
{
    if(object != nullptr)
    {
        static_cast<RGBController_TAC75HE*>(object)->UpdateLEDs();
    }
}

void RGBController_TAC75HE::DeviceUpdateZoneLEDs(void* object, int)
{
    DeviceUpdateLEDs(object);
}

void RGBController_TAC75HE::DeviceUpdateSingleLED(void* object, int)
{
    DeviceUpdateLEDs(object);
}

void RGBController_TAC75HE::DeviceUpdateMode(void* object)
{
    DeviceUpdateLEDs(object);
}

void RGBController_TAC75HE::UpdateLEDs()
{
    if(!hid || controller == nullptr || controller->GetLEDCount() < TAC75_LED_COUNT)
    {
        return;
    }

    TAC75HEFrame frame{};

    unsigned int brightness = 100;
    const int active_mode = controller->GetActiveMode();

    if(active_mode >= 0
    && static_cast<unsigned int>(active_mode) < controller->GetModeCount())
    {
        brightness = controller->GetModeBrightness(
            static_cast<unsigned int>(active_mode)
        );
    }

    for(std::size_t position = 0;
        position < std::size(tac75_transport_map);
        position++)
    {
        const unsigned int led_index = tac75_transport_map[position];

        if(led_index == NA || led_index >= controller->GetLEDCount())
        {
            continue;
        }

        const RGBColor color = controller->GetColor(led_index);

        frame[position * 3 + 0] =
            static_cast<std::uint8_t>(
                RGBGetRValue(color) * brightness / 100
            );

        frame[position * 3 + 1] =
            static_cast<std::uint8_t>(
                RGBGetGValue(color) * brightness / 100
            );

        frame[position * 3 + 2] =
            static_cast<std::uint8_t>(
                RGBGetBValue(color) * brightness / 100
            );
    }

    std::uint64_t channel_sum = 0;

    for(const std::uint8_t value : frame)
    {
        channel_sum += value;
    }

    if(channel_sum > MAX_CHANNEL_SUM)
    {
        for(std::uint8_t& value : frame)
        {
            value = static_cast<std::uint8_t>(
                static_cast<std::uint64_t>(value)
                * MAX_CHANNEL_SUM
                / channel_sum
            );
        }
    }

    std::string error;

    if(!hid->SendFrame(frame, error))
    {
        if(!io_error_reported)
        {
            std::fprintf(
                stderr,
                "[OpenRGBTAC75HEPlugin] LED update failed: %s\n",
                error.c_str()
            );

            io_error_reported = true;
        }

        return;
    }

    io_error_reported = false;
}
