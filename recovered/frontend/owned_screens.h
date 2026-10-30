#ifndef HD2_FRONTEND_OWNED_SCREENS_H
#define HD2_FRONTEND_OWNED_SCREENS_H

#include "frontend/screen_registry.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace hd2::frontend {

// Native ownership for the original registered-screen pointer vector. Append
// compacts successful products; definition identifiers are not registry indices.
// Slot zero, if needed, must be constructed separately before named screens.
// Registrations and product addresses stay stable across vector growth. Stack
// entries/previous pointers borrow them; destroy the stack before clearing or
// destroying this owner. Products own their own resources/children separately.
template<class Screen>
class OwnedScreens {
public:
    OwnedScreens() = default;
    OwnedScreens(const OwnedScreens&) = delete;
    OwnedScreens& operator=(const OwnedScreens&) = delete;
    ~OwnedScreens() { clear(); }

    std::size_t size() const { return entries_.size(); }
    const ScreenRegistration& registration(std::size_t index) const {
        return entries_.at(index)->registration;
    }
    Screen& screen(std::size_t index) { return *entries_.at(index)->screen; }
    const Screen& screen(std::size_t index) const { return *entries_.at(index)->screen; }
    void append(const ScreenRegistration& registration, std::unique_ptr<Screen> screen) {
        if (!screen) throw std::invalid_argument("Cannot register a null screen");
        entries_.push_back(std::make_unique<Entry>(registration, std::move(screen)));
    }
    void clear() {
        while (!entries_.empty()) entries_.pop_back();
    }

private:
    struct Entry {
        Entry(const ScreenRegistration& source, std::unique_ptr<Screen> product)
            : name(source.name ? source.name : ""), model(source.model ? source.model : ""),
              registration{name.c_str(), model.c_str(), source.identifier}, screen(std::move(product)) {}
        std::string name;
        std::string model;
        ScreenRegistration registration;
        std::unique_ptr<Screen> screen;
    };
    std::vector<std::unique_ptr<Entry>> entries_;
};

}

#endif
