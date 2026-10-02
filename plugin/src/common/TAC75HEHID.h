#pragma once

#include <hidapi.h>

#include <array>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

struct TAC75HEPatchInfo
{
    std::uint8_t  version      = 0;
    std::uint16_t capabilities = 0;
    std::string   name;
};

using TAC75HEFrame = std::array<std::uint8_t, 96u * 3u>;

class TAC75HEHID
{
public:
    static std::unique_ptr<TAC75HEHID> OpenAndProbe(
        TAC75HEPatchInfo& patch_info,
        std::string& error
    );

    ~TAC75HEHID();

    TAC75HEHID(const TAC75HEHID&) = delete;
    TAC75HEHID& operator=(const TAC75HEHID&) = delete;

    const TAC75HEPatchInfo& PatchInfo() const;
    const std::string& Path() const;

    bool SendFrame(
        const TAC75HEFrame& frame,
        std::string& error
    );

    bool Release(std::string& error);

private:
    TAC75HEHID(
        std::string path,
        TAC75HEPatchInfo patch_info
    );

    static bool IsVendorUsagePage(std::uint16_t usage_page);
    static std::string HidError(hid_device* device);

    static hid_device* OpenCompatibleDevice(
        TAC75HEPatchInfo& patch_info,
        std::string& path,
        std::string& error
    );

    static bool ProbePatch(
        hid_device* device,
        TAC75HEPatchInfo& patch_info,
        std::string& error
    );

    static bool SendFramePackets(
        hid_device* device,
        const TAC75HEFrame& frame,
        bool repeat_first_frame,
        std::string& error
    );

    static bool SendReleasePacket(
        hid_device* device,
        std::string& error
    );

    hid_device* OpenForIo(std::string& error);

    std::string      device_path;
    TAC75HEPatchInfo patch;

    mutable std::mutex io_mutex;
    bool stream_active = false;
};
