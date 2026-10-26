#include "scene/model_file_header.h"

#include <array>
#include <iostream>
#include <sstream>
#include <string>

namespace {

std::string make_header(std::uint16_t count = 13, std::uint16_t version = 0x29) {
    std::string bytes("4DS\0", 4);
    bytes.push_back(static_cast<char>(version & 0xff));
    bytes.push_back(static_cast<char>(version >> 8));
    bytes.append("\x80\xff\x00\x01\x12\x34\x56\x78", 8);
    bytes.push_back(static_cast<char>(count & 0xff));
    bytes.push_back(static_cast<char>(count >> 8));
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
    using hd2::recovered::ModelHeaderError;
    using Failure = hd2::recovered::ModelHeaderFailure;
    using hd2::recovered::read_model_file_header;
    std::size_t total = 0;
    std::size_t failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++total;
        if (!condition) {
            ++failures;
            std::cerr << "FAIL " << name << '\n';
        }
    };
    const auto reject = [&](const std::string& bytes, Failure reason, std::streamoff position) {
        std::istringstream input(bytes);
        bool rejected = false;
        try {
            read_model_file_header(input);
        } catch (const ModelHeaderError& error) {
            rejected = error.reason() == reason;
        }
        if (position >= 0) {
            rejected = rejected && input.tellg() == position;
        }
        check(rejected, "rejection reason and stopping position");
    };
    constexpr std::array<std::uint8_t, 8> expected_metadata{
        0x80, 0xff, 0x00, 0x01, 0x12, 0x34, 0x56, 0x78
    };
    for (const auto count : {0, 1, 13, 86, 5999, 6000}) {
        std::istringstream input(make_header(static_cast<std::uint16_t>(count)) + "tail");
        const auto header = read_model_file_header(input);
        check(header.version == 0x29 && header.first_table_entry_count == count
                  && header.opaque_metadata == expected_metadata
                  && input.tellg() == 16 && input.peek() == 't',
              "valid prefix, opaque bytes and unconsumed payload");
    }
    for (std::size_t index = 0; index < 4; ++index) {
        auto bytes = make_header();
        bytes[index] ^= 1;
        reject(bytes, Failure::invalid_signature, 4);
    }
    for (const auto version : {0, 0x28, 0x2a, 0xffff}) {
        reject(make_header(13, static_cast<std::uint16_t>(version)),
               Failure::unsupported_version, 6);
    }
    for (const auto count : {6001, 32768, 65535}) {
        reject(make_header(static_cast<std::uint16_t>(count)),
               Failure::entry_limit_exceeded, 16);
    }
    for (std::size_t size = 0; size < 16; ++size) {
        reject(make_header().substr(0, size), Failure::incomplete_read, -1);
    }
    std::istringstream failed(make_header());
    failed.setstate(std::ios::badbit);
    bool rejected_bad_stream = false;
    try {
        read_model_file_header(failed);
    } catch (const ModelHeaderError& error) {
        rejected_bad_stream = error.reason() == Failure::incomplete_read;
    }
    check(rejected_bad_stream, "pre-failed stream");

    std::istringstream throwing("4DS");
    throwing.exceptions(std::ios::failbit | std::ios::badbit);
    bool propagated_io_error = false;
    try {
        read_model_file_header(throwing);
    } catch (const std::ios_base::failure&) {
        propagated_io_error = true;
    }
    check(propagated_io_error, "stream exception propagation");

    std::istringstream offset_input("skip" + make_header(1));
    offset_input.ignore(4);
    check(read_model_file_header(offset_input).first_table_entry_count == 1
              && offset_input.tellg() == 20, "current-position reading");

    NonSeekableBuffer buffer(make_header(86));
    std::istream nonseekable(&buffer);
    check(read_model_file_header(nonseekable).first_table_entry_count == 86,
          "nonseekable stream");

    std::istringstream consecutive(make_header(13) + make_header(86));
    const auto first = read_model_file_header(consecutive);
    const auto second = read_model_file_header(consecutive);
    check(first.first_table_entry_count == 13 && second.first_table_entry_count == 86
              && consecutive.tellg() == 32, "consecutive prefixes");
    reject(make_header(6001, 0x28), Failure::unsupported_version, 6);
    std::cout << total - failures << '/' << total << " model-file-header tests passed\n";
    return failures == 0 ? 0 : 1;
}
