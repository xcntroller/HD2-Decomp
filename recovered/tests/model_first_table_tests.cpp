#include "scene/model_first_table.h"

#include <iostream>
#include <sstream>
#include <string>

namespace {

constexpr std::uint32_t bits[]{0x00080000U, 0x04000000U, 0x40000000U,
                               0x80000000U, 0x20000000U, 0x01000000U};

void append_name(std::string& bytes, const std::string& name) {
    if (name.size() > 255) {
        throw std::logic_error("Synthetic name exceeds the one-byte length");
    }
    bytes.push_back(static_cast<char>(name.size()));
    bytes += name;
}

std::string record_bytes(std::uint32_t flags, const std::string& primary,
                         const std::string& extra = "", const std::string& secondary = "") {
    std::string bytes;
    for (unsigned int shift = 0; shift < 32; shift += 8) {
        bytes.push_back(static_cast<char>((flags >> shift) & 0xff));
    }
    const std::size_t size = flags & 0x00080000U ? 76 : 72;
    for (std::size_t index = 0; index < size; ++index) {
        bytes.push_back(static_cast<char>(index + 128));
    }
    if (flags & 0x00080000U) {
        append_name(bytes, extra);
    }
    append_name(bytes, primary);
    if (!primary.empty() && (flags & 0x40000000U) && !(flags & 0xa1000000U)) {
        append_name(bytes, secondary);
    }
    if (flags & 0x04000000U) {
        bytes.append("\xfe\x81", 2);
        bytes.append("\x00\x01\x02\x03\x04\x05\x06\x07"
                     "\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f", 16);
    }
    return bytes;
}

class NonSeekableBuffer : public std::stringbuf {
public:
    explicit NonSeekableBuffer(const std::string& bytes) : std::stringbuf(bytes) {}

protected:
    pos_type seekoff(off_type, std::ios_base::seekdir, std::ios_base::openmode) override {
        return pos_type(off_type(-1));
    }
    pos_type seekpos(pos_type, std::ios_base::openmode) override {
        return pos_type(off_type(-1));
    }
};

}

int main() {
    using hd2::recovered::ModelTableReadError;
    using hd2::recovered::read_model_first_table;
    std::size_t total = 0;
    std::size_t failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++total;
        if (!condition) {
            ++failures;
            std::cerr << "FAIL " << name << '\n';
        }
    };
    for (unsigned int mask = 0; mask < 64; ++mask) {
        std::uint32_t flags = 0x00800001U;
        for (std::size_t index = 0; index < 6; ++index) {
            if (mask & (1U << index)) {
                flags |= bits[index];
            }
        }
        for (const bool empty : {false, true}) {
            const std::string primary = empty ? "" : std::string("\0\xff" "A", 3);
            const auto encoded = record_bytes(flags, primary);
            std::istringstream input(encoded + "tail");
            const auto records = read_model_first_table(input, 1);
            const auto& record = records.at(0);
            const bool extra = (flags & 0x00080000U) != 0;
            const bool secondary = !empty && (flags & 0x40000000U)
                && !(flags & 0xa1000000U);
            const bool sequence = (flags & 0x04000000U) != 0;
            bool valid = record.serialized_flags == flags
                && record.opaque_payload.size() == (extra ? 76 : 72)
                && record.primary_name_bytes == primary
                && record.extra_name_bytes.has_value() == extra
                && (!extra || record.extra_name_bytes->empty())
                && record.secondary_name_bytes.has_value() == secondary
                && (!secondary || record.secondary_name_bytes->empty())
                && record.sequence.has_value() == sequence
                && input.tellg() == static_cast<std::streamoff>(encoded.size())
                && input.peek() == 't';
            for (std::size_t index = 0; index < record.opaque_payload.size(); ++index) {
                valid = valid && record.opaque_payload[index] == index + 128;
            }
            if (sequence) {
                valid = valid && record.sequence->counts[0] == 254
                    && record.sequence->counts[1] == 129;
                for (std::size_t index = 0; index < 16; ++index) {
                    valid = valid && record.sequence->opaque_parameters[index] == index;
                }
            }
            check(valid, "all layout flags, zero-length optional names and raw primary bytes");
        }
    }
    const std::string longest(255, '\xff');
    const auto maximal = record_bytes(0x44080000U, longest, longest, longest);
    std::istringstream maximum_names(maximal);
    const auto maximum = read_model_first_table(maximum_names, 1);
    check(maximum[0].primary_name_bytes == longest
              && maximum[0].extra_name_bytes == longest
              && maximum[0].secondary_name_bytes == longest,
          "255-byte names");
    for (std::size_t size = 0; size < maximal.size(); ++size) {
        std::istringstream input(maximal.substr(0, size));
        bool rejected = false;
        try {
            read_model_first_table(input, 1);
        } catch (const ModelTableReadError&) {
            rejected = true;
        }
        check(rejected, "every truncated maximal record");
    }
    std::istringstream zero("tail");
    check(read_model_first_table(zero, 0).empty() && zero.tellg() == 0,
          "zero records leave input untouched");
    std::string many;
    for (std::size_t index = 0; index < 6000; ++index) {
        many += record_bytes(0, "");
    }
    std::istringstream limit(many + "tail");
    check(read_model_first_table(limit, 6000).size() == 6000 && limit.peek() == 't',
          "6000 records accepted");
    std::istringstream excessive("tail");
    bool rejected_count = false;
    try {
        read_model_first_table(excessive, 6001);
    } catch (const ModelTableReadError&) {
        rejected_count = excessive.tellg() == 0;
    }
    check(rejected_count, "6001 records rejected before reading");
    const auto minimal = record_bytes(0, "");
    NonSeekableBuffer buffer(minimal);
    std::istream nonseekable(&buffer);
    check(read_model_first_table(nonseekable, 1).size() == 1, "nonseekable input");
    std::istringstream positioned("skip" + minimal + maximal + "tail");
    positioned.ignore(4);
    const auto combined = read_model_first_table(positioned, 2);
    check(combined.size() == 2 && !combined[0].sequence && combined[1].sequence
              && positioned.peek() == 't', "current position and mixed consecutive records");
    std::istringstream failed(minimal);
    failed.setstate(std::ios::badbit);
    bool rejected_stream = false;
    try {
        read_model_first_table(failed, 1);
    } catch (const ModelTableReadError&) {
        rejected_stream = true;
    }
    check(rejected_stream, "pre-failed stream");
    std::istringstream throwing("abc");
    throwing.exceptions(std::ios::failbit | std::ios::badbit);
    bool propagated = false;
    try {
        read_model_first_table(throwing, 1);
    } catch (const std::ios_base::failure&) {
        propagated = true;
    }
    check(propagated, "configured stream exceptions propagate");
    std::cout << total - failures << '/' << total << " model-first-table tests passed\n";
    return failures == 0 ? 0 : 1;
}
