#include "host/bitmap_font.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace hd2::host {
namespace {

constexpr std::size_t max_labels = 256;
constexpr std::size_t max_label_length = 4096;
constexpr std::size_t max_label_file = 1024 * 1024;

std::uint32_t read_u32(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    if (offset > bytes.size() || bytes.size() - offset < 4) {
        throw std::invalid_argument("Truncated menu text data");
    }
    return static_cast<std::uint32_t>(bytes[offset])
        | (static_cast<std::uint32_t>(bytes[offset + 1]) << 8)
        | (static_cast<std::uint32_t>(bytes[offset + 2]) << 16)
        | (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
}

float read_float(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559);
    const auto bits = read_u32(bytes, offset);
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

}

FontMetrics decode_font_metrics(const std::vector<std::uint8_t>& bytes) {
    if (bytes.size() != 5120) {
        throw std::invalid_argument("Font TBL must contain exactly 5120 bytes");
    }
    FontMetrics result{};
    for (std::size_t i = 0; i < result.size(); ++i) {
        float fields[5];
        for (std::size_t j = 0; j < 5; ++j) {
            fields[j] = read_float(bytes, i * 20 + j * 4);
            if (!std::isfinite(fields[j])) {
                throw std::invalid_argument("Non-finite font metric");
            }
        }
        for (std::size_t j = 0; j < 4; ++j) {
            if (fields[j] < 0 || fields[j] > 1) {
                throw std::invalid_argument("Font UV outside unit interval");
            }
        }
        if (fields[0] > fields[2] || fields[1] > fields[3]
                || fields[4] < 0 || fields[4] > 16) {
            throw std::invalid_argument("Invalid font UV order or aspect");
        }
        result[i] = {fields[0], fields[1], fields[2], fields[3], fields[4]};
    }
    return result;
}

std::map<std::uint32_t, std::string> decode_menu_labels(
        const std::vector<std::uint8_t>& bytes) {
    if (bytes.size() < 8 || bytes.size() > max_label_file
            || std::memcmp(bytes.data(), "H2MT", 4) != 0) {
        throw std::invalid_argument("Invalid menu label slice header");
    }
    const auto count = read_u32(bytes, 4);
    if (count == 0 || count > max_labels) {
        throw std::invalid_argument("Invalid menu label count");
    }
    std::map<std::uint32_t, std::string> result;
    std::size_t offset = 8;
    for (std::uint32_t i = 0; i < count; ++i) {
        const auto id = read_u32(bytes, offset);
        const auto length = read_u32(bytes, offset + 4);
        offset += 8;
        if ((length == 0 && id != 2002) || length > max_label_length || length > bytes.size() - offset) {
            throw std::invalid_argument("Invalid menu label length");
        }
        for (std::size_t j = 0; j < length; ++j) {
            if (bytes[offset + j] < 0x20 || bytes[offset + j] > 0x7e) {
                throw std::invalid_argument("Menu labels must contain printable ASCII");
            }
        }
        const std::string label(
            reinterpret_cast<const char*>(bytes.data() + offset), length);
        if (!result.emplace(id, label).second) {
            throw std::invalid_argument("Duplicate menu label ID");
        }
        offset += length;
    }
    if (offset != bytes.size()) {
        throw std::invalid_argument("Trailing menu label data");
    }
    return result;
}

}
