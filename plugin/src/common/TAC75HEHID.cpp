#include "TAC75HEHID.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <utility>
#include <vector>

namespace
{
constexpr unsigned short TAC75_VID = 0x3151;
constexpr unsigned short TAC75_PID = 0x502D;

constexpr int TAC75_FEATURE_INTERFACE = 2;

constexpr unsigned short USAGE_PAGE_VENDOR     = 0xFFFF;
constexpr unsigned short USAGE_PAGE_VENDOR_ALT = 0xFF00;
constexpr unsigned short USAGE_FEATURE         = 0x0002;

constexpr std::size_t REPORT_SIZE       = 65;
constexpr std::size_t LEDS_PER_PAGE     = 18;
constexpr std::size_t BYTES_PER_PAGE    = LEDS_PER_PAGE * 3;
constexpr std::size_t PAGE_COUNT         = 6;

constexpr unsigned char CMD_PATCH_INFO = 0xE7;
constexpr unsigned char CMD_LED_STREAM = 0xE8;

constexpr unsigned char PATCH_MAGIC_HI = 0xCA;
constexpr unsigned char PATCH_MAGIC_LO = 0xFE;

constexpr std::array<unsigned char, 4> PATCH_REQUEST_MAGIC = {
    'T', '7', '5', 'D'
};

constexpr std::uint16_t CAP_LED_STREAM = 1u << 1;

std::array<unsigned char, REPORT_SIZE> BuildCommand(
    unsigned char command,
    const unsigned char* data,
    std::size_t data_size,
    bool bit7_checksum
)
{
    std::array<unsigned char, REPORT_SIZE> report{};

    report[0] = 0x00;
    report[1] = command;

    const std::size_t copied = std::min(
        data_size,
        REPORT_SIZE - 2
    );

    if(copied != 0 && data != nullptr)
    {
        std::memcpy(&report[2], data, copied);
    }

    if(bit7_checksum)
    {
        unsigned int sum = 0;

        for(std::size_t index = 1; index <= 7; index++)
        {
            sum += report[index];
        }

        report[8] = static_cast<unsigned char>(
            255u - (sum & 0xFFu)
        );
    }

    return report;
}

std::string ReadPatchName(
    const unsigned char* response,
    std::size_t response_size,
    std::size_t name_offset
)
{
    if(name_offset >= response_size)
    {
        return {};
    }

    const std::size_t name_end = std::min(
        response_size,
        name_offset + 9
    );

    std::size_t length = 0;

    while(name_offset + length < name_end
       && response[name_offset + length] != 0)
    {
        length++;
    }

    return std::string(
        reinterpret_cast<const char*>(response + name_offset),
        length
    );
}
}

std::unique_ptr<TAC75HEHID> TAC75HEHID::OpenAndProbe(
    TAC75HEPatchInfo& patch_info,
    std::string& error
)
{
    patch_info = {};
    error.clear();

    if(hid_init() != 0)
    {
        error = "hid_init failed";
        return nullptr;
    }

    std::string path;

    hid_device* opened = OpenCompatibleDevice(
        patch_info,
        path,
        error
    );

    if(opened == nullptr)
    {
        return nullptr;
    }

    /*
     * Probe succeeded. Keep only the path and metadata; OpenRGB must not
     * hold the vendor HID interface while idle.
     */
    hid_close(opened);

    return std::unique_ptr<TAC75HEHID>(
        new TAC75HEHID(
            std::move(path),
            patch_info
        )
    );
}

hid_device* TAC75HEHID::OpenCompatibleDevice(
    TAC75HEPatchInfo& patch_info,
    std::string& path,
    std::string& error
)
{
    patch_info = {};
    path.clear();
    error.clear();

    hid_device_info* devices = hid_enumerate(
        TAC75_VID,
        TAC75_PID
    );

    if(devices == nullptr)
    {
        error = "3151:502D not found";
        return nullptr;
    }

    std::vector<std::string> candidates;

    for(hid_device_info* current = devices;
        current != nullptr;
        current = current->next)
    {
        if(current->path == nullptr)
        {
            continue;
        }

        if(current->interface_number != TAC75_FEATURE_INTERFACE)
        {
            continue;
        }

        if(!IsVendorUsagePage(current->usage_page))
        {
            continue;
        }

        if(current->usage != USAGE_FEATURE)
        {
            continue;
        }

        candidates.emplace_back(current->path);
    }

    hid_free_enumeration(devices);

    if(candidates.empty())
    {
        error = "TAC75 HE vendor feature interface IF2 not found";
        return nullptr;
    }

    std::string last_error;

    for(const std::string& candidate : candidates)
    {
        hid_device* opened = hid_open_path(candidate.c_str());

        if(opened == nullptr)
        {
            last_error = "hid_open_path failed for " + candidate;
            continue;
        }

        TAC75HEPatchInfo detected;
        std::string probe_error;

        if(!ProbePatch(opened, detected, probe_error))
        {
            last_error = probe_error;
            hid_close(opened);
            continue;
        }

        if(detected.name != "TAC75DM")
        {
            last_error =
                "unexpected firmware patch name: "
                + detected.name;

            hid_close(opened);
            continue;
        }

        if(detected.version != 1)
        {
            last_error =
                "unsupported TAC75DM protocol version: "
                + std::to_string(detected.version);

            hid_close(opened);
            continue;
        }

        if((detected.capabilities & CAP_LED_STREAM) == 0)
        {
            last_error =
                "TAC75DM patch has no led_stream capability";

            hid_close(opened);
            continue;
        }

        patch_info = std::move(detected);
        path       = candidate;

        return opened;
    }

    error = last_error.empty()
        ? "no compatible TAC75DM device found"
        : last_error;

    return nullptr;
}

TAC75HEHID::TAC75HEHID(
    std::string path,
    TAC75HEPatchInfo patch_info
)
    : device_path(std::move(path))
    , patch(std::move(patch_info))
{
}

TAC75HEHID::~TAC75HEHID()
{
    std::string ignored_error;
    Release(ignored_error);
}

hid_device* TAC75HEHID::OpenForIo(std::string& error)
{
    error.clear();

    if(!device_path.empty())
    {
        hid_device* opened =
            hid_open_path(device_path.c_str());

        if(opened != nullptr)
        {
            return opened;
        }
    }

    /*
     * The hidraw path can change after unplug/replug. Rediscover only when
     * an actual OpenRGB update needs the keyboard; never poll in background.
     */
    TAC75HEPatchInfo detected_patch;
    std::string detected_path;

    hid_device* opened = OpenCompatibleDevice(
        detected_patch,
        detected_path,
        error
    );

    if(opened == nullptr)
    {
        return nullptr;
    }

    device_path = std::move(detected_path);
    patch       = std::move(detected_patch);

    return opened;
}

const TAC75HEPatchInfo& TAC75HEHID::PatchInfo() const
{
    return patch;
}

const std::string& TAC75HEHID::Path() const
{
    return device_path;
}

bool TAC75HEHID::SendFrame(
    const TAC75HEFrame& frame,
    std::string& error
)
{
    std::lock_guard<std::mutex> lock(io_mutex);

    hid_device* opened = OpenForIo(error);

    if(opened == nullptr)
    {
        stream_active = false;
        return false;
    }

    /*
     * Each update is an independent short transaction. Repeating the frame
     * makes activation reliable even when Akko Web Driver previously took
     * ownership back.
     */
    const bool sent = SendFramePackets(
        opened,
        frame,
        true,
        error
    );

    hid_close(opened);

    if(!sent)
    {
        stream_active = false;
        return false;
    }

    stream_active = true;
    return true;
}

bool TAC75HEHID::Release(std::string& error)
{
    std::lock_guard<std::mutex> lock(io_mutex);

    error.clear();

    if(!stream_active)
    {
        return true;
    }

    hid_device* opened = OpenForIo(error);

    if(opened == nullptr)
    {
        stream_active = false;
        return false;
    }

    const bool released =
        SendReleasePacket(opened, error);

    hid_close(opened);
    stream_active = false;

    return released;
}

bool TAC75HEHID::SendFramePackets(
    hid_device* target,
    const TAC75HEFrame& frame,
    bool repeat_first_frame,
    std::string& error
)
{
    const auto send_once = [&]() -> bool
    {
        for(std::size_t page = 0;
            page < PAGE_COUNT;
            page++)
        {
            std::array<unsigned char, REPORT_SIZE> report{};

            report[0] = 0x00;
            report[1] = CMD_LED_STREAM;

            std::copy(
                PATCH_REQUEST_MAGIC.begin(),
                PATCH_REQUEST_MAGIC.end(),
                report.begin() + 2
            );

            report[6] = static_cast<unsigned char>(page);

            const std::size_t source_offset =
                page * BYTES_PER_PAGE;

            const std::size_t remaining =
                source_offset < frame.size()
                    ? frame.size() - source_offset
                    : 0;

            const std::size_t copy_size = std::min(
                BYTES_PER_PAGE,
                remaining
            );

            if(copy_size != 0)
            {
                std::memcpy(
                    report.data() + 7,
                    frame.data() + source_offset,
                    copy_size
                );
            }

            if(hid_send_feature_report(
                target,
                report.data(),
                report.size()
            ) < 0)
            {
                error =
                    "E8 page "
                    + std::to_string(page)
                    + " failed: "
                    + HidError(target);

                return false;
            }
        }

        std::array<unsigned char, REPORT_SIZE> commit{};

        commit[0] = 0x00;
        commit[1] = CMD_LED_STREAM;

        std::copy(
            PATCH_REQUEST_MAGIC.begin(),
            PATCH_REQUEST_MAGIC.end(),
            commit.begin() + 2
        );

        commit[6] = 0xFF;

        if(hid_send_feature_report(
            target,
            commit.data(),
            commit.size()
        ) < 0)
        {
            error = "E8 commit failed: " + HidError(target);
            return false;
        }

        return true;
    };

    if(!send_once())
    {
        return false;
    }

    if(repeat_first_frame && !send_once())
    {
        return false;
    }

    return true;
}

bool TAC75HEHID::SendReleasePacket(
    hid_device* target,
    std::string& error
)
{
    std::array<unsigned char, REPORT_SIZE> report{};

    report[0] = 0x00;
    report[1] = CMD_LED_STREAM;

    std::copy(
        PATCH_REQUEST_MAGIC.begin(),
        PATCH_REQUEST_MAGIC.end(),
        report.begin() + 2
    );

    report[6] = 0xFE;

    if(hid_send_feature_report(
        target,
        report.data(),
        report.size()
    ) < 0)
    {
        error = "E8 release failed: " + HidError(target);
        return false;
    }

    return true;
}

bool TAC75HEHID::IsVendorUsagePage(
    std::uint16_t usage_page
)
{
    return usage_page == USAGE_PAGE_VENDOR
        || usage_page == USAGE_PAGE_VENDOR_ALT;
}

std::string TAC75HEHID::HidError(hid_device* target)
{
    const wchar_t* wide_error = hid_error(target);

    if(wide_error == nullptr)
    {
        return "unknown hidapi error";
    }

    std::string result;

    while(*wide_error != L'\0')
    {
        const wchar_t character = *wide_error++;

        result.push_back(
            character >= 0 && character <= 0x7F
                ? static_cast<char>(character)
                : '?'
        );
    }

    return result;
}

bool TAC75HEHID::ProbePatch(
    hid_device* target,
    TAC75HEPatchInfo& patch_info,
    std::string& error
)
{
    const auto request = BuildCommand(
        CMD_PATCH_INFO,
        PATCH_REQUEST_MAGIC.data(),
        PATCH_REQUEST_MAGIC.size(),
        true
    );

    if(hid_send_feature_report(
        target,
        request.data(),
        request.size()
    ) < 0)
    {
        error = "E7 send failed: " + HidError(target);
        return false;
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(100)
    );

    std::array<unsigned char, REPORT_SIZE> response{};
    response[0] = 0x00;

    const int received = hid_get_feature_report(
        target,
        response.data(),
        response.size()
    );

    if(received < 0)
    {
        error = "E7 read failed: " + HidError(target);
        return false;
    }

    const std::size_t size =
        static_cast<std::size_t>(received);

    std::size_t version_offset = 0;
    std::size_t caps_offset    = 0;
    std::size_t name_offset    = 0;

    if(size >= 8
    && response[1] == CMD_PATCH_INFO
    && response[2] == PATCH_MAGIC_HI
    && response[3] == PATCH_MAGIC_LO)
    {
        version_offset = 4;
        caps_offset    = 5;
        name_offset    = 7;
    }
    else if(size >= 7
         && response[1] == PATCH_MAGIC_HI
         && response[2] == PATCH_MAGIC_LO)
    {
        version_offset = 3;
        caps_offset    = 4;
        name_offset    = 6;
    }
    else
    {
        error =
            "E7 response does not contain CA FE patch magic";

        return false;
    }

    patch_info.version = response[version_offset];

    patch_info.capabilities =
        static_cast<std::uint16_t>(
            response[caps_offset]
        )
      | static_cast<std::uint16_t>(
            response[caps_offset + 1]
        ) << 8;

    patch_info.name = ReadPatchName(
        response.data(),
        size,
        name_offset
    );

    return true;
}
