#include "scene/model_resource_key.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

int main() {
    const std::vector<std::pair<std::string, std::string>> cases{
        {"", ""},
        {"crate", "crate"},
        {"crate.4ds", "crate"},
        {"crate.variant.4ds", "crate.variant"},
        {".hidden", ""},
        {"folder.name/object", "folder"},
        {"folder/object.4ds", "folder/object"},
        {"folder\\object.4ds", "folder\\object"},
        {"name.", "name"},
        {"MiXeD.4DS", "MiXeD"},
        {" a .b", " a "},
        {".", ""},
        {"..", "."},
    };
    std::size_t failures = 0;
    for (const auto& [input, expected] : cases) {
        if (hd2::recovered::model_resource_key(input) != expected) {
            std::cerr << "FAIL model resource key for synthetic input: " << input << '\n';
            ++failures;
        }
    }
    bool rejected_nul = false;
    try {
        hd2::recovered::model_resource_key(std::string("before\0after", 12));
    } catch (const std::invalid_argument&) {
        rejected_nul = true;
    }
    if (!rejected_nul) {
        std::cerr << "FAIL embedded NUL was accepted\n";
        ++failures;
    }
    const auto total = cases.size() + 1;
    std::cout << (total - failures) << '/' << total << " model-resource-key tests passed\n";
    return failures == 0 ? 0 : 1;
}
