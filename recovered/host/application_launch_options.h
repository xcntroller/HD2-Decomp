#ifndef HD2_HOST_APPLICATION_LAUNCH_OPTIONS_H
#define HD2_HOST_APPLICATION_LAUNCH_OPTIONS_H

#include "host/graphics_settings.h"
#include "host/menu_presentation.h"
#include "host/loading_input_capture.h"
#include <array>
#include <string>

namespace hd2::host {

// Native packaging entry policy; no original function-body/ABI credit.
// Portable: EXE [--frames N] [--width N] [--height N] [--language NAME]
//               [--fullscreen | --windowed].
// Keep the existing developer launcher: EXE ENGINE --frames N [options].
struct ApplicationLaunchOptions {
    std::wstring directory, engine;
    std::uint32_t frames = 0;
    int width = 800, height = 600;
    std::string language;
    bool fullscreen = false;
    std::wstring loading_capture;
};

inline ApplicationLaunchOptions application_launch_options(
        int argc, const wchar_t* const* argv, const std::wstring& executable) {
    if (argc < 1 || !argv) throw std::invalid_argument("Missing launch arguments");
    const auto slash = executable.find_last_of(L"\\/");
    if (slash == std::wstring::npos)
        throw std::invalid_argument("Launch executable needs its full path");
    ApplicationLaunchOptions result;
    result.directory = executable.substr(0, slash + 1);
    result.engine = result.directory + L"LS3DF.dll";
    int first = 1;
    if (argc > 1 && std::wstring_view(argv[1]).substr(0, 2) != L"--") {
        if (argc < 4 || !*argv[1] || std::wstring_view(argv[2]) != L"--frames")
            throw std::invalid_argument("Use EXE [options] or EXE ENGINE --frames N [options]");
        result.engine = argv[1];
        first = 2;
    }
    std::array<bool, 5> seen{};
    for (int index = first; index < argc;) {
        const std::wstring_view option(argv[index]);
        if (option == L"--capture-loading-inputs") {
            if (!result.loading_capture.empty() || index + 1 >= argc)
                throw std::invalid_argument("Loading-input capture needs one fresh output name");
            result.loading_capture = argv[index + 1];
            validate_loading_capture_name(result.loading_capture);
            index += 2;
            continue;
        }
        if (option==L"--fullscreen" || option==L"--windowed") {
            if (seen[4]) throw std::invalid_argument("Specify only one display mode");
            seen[4]=true;
            result.fullscreen=option==L"--fullscreen";
            ++index;
            continue;
        }
        const unsigned kind = option == L"--frames" ? 0 : option == L"--width" ? 1 :
                              option == L"--height" ? 2 : option == L"--language" ? 3 : 4;
        if (kind == 4 || seen[kind])
            throw std::invalid_argument("Unknown or repeated launch option");
        if (index+1>=argc) throw std::invalid_argument("Launch option needs a value");
        seen[kind] = true;
        if (kind == 3) {
            for (const auto character : std::wstring_view(argv[index + 1])) {
                if (!((character >= L'A' && character <= L'Z') ||
                      (character >= L'a' && character <= L'z')))
                    throw std::invalid_argument("Language must name an installed translation directory");
                result.language += static_cast<char>(character);
            }
            if (result.language.empty() || result.language.size() > 64)
                throw std::invalid_argument("Invalid language name");
        } else {
            const auto value = parse_frame_limit(argv[index + 1]);
            if (kind == 0) result.frames = value;
            else {
                if (value > 16384) throw std::invalid_argument("Render dimension exceeds limit");
                (kind == 1 ? result.width : result.height) = static_cast<int>(value);
            }
        }
        index+=2;
    }
    validate_menu_dimensions(result.width, result.height);
    return result;
}

} // namespace hd2::host
#endif
