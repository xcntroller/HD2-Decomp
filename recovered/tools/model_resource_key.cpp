#include "scene/model_resource_key.h"

#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: model_resource_key decoded_resource_name\n";
        return 2;
    }
    try {
        std::cout << hd2::recovered::model_resource_key(argv[1]) << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Model resource key failed: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
