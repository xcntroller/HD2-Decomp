#include "scene/model_resource_key.h"

#include <stdexcept>

namespace hd2::recovered {

std::string model_resource_key(std::string_view name) {
    if (name.find('\0') != std::string_view::npos) {
        throw std::invalid_argument("Model resource names must not contain embedded NUL bytes");
    }
    return std::string(name.substr(0, name.rfind('.')));
}

}
