#ifndef HD2_FRONTEND_MODULE_REGISTRATION_H
#define HD2_FRONTEND_MODULE_REGISTRATION_H

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace hd2::frontend {

class ClientModuleRegistration;
// Registry0089BAD8..0089BB23. Borrowed entries; registry must outlive modules.
using ClientModuleRegistry = std::array<ClientModuleRegistration*, 19>;

// 0048D950/0048DA30: native name ownership and registry slot lifetime only.
// Original base virtual event methods are separate; no successful handlers are
// supplied here for unrecovered module operations. Duplicate kinds overwrite;
// destruction clears the slot unconditionally, even after another registration.
class ClientModuleRegistration {
public:
    ClientModuleRegistration(ClientModuleRegistry& registry, std::uint32_t kind, const char* name)
        : registry_(registry), kind_(kind), name_(name ? name : "") {
        if (kind >= registry_.size()) throw std::out_of_range("Invalid original module registry kind");
        registry_[kind_] = this;
    }
    ~ClientModuleRegistration() { registry_[kind_] = nullptr; }
    ClientModuleRegistration(const ClientModuleRegistration&) = delete;
    ClientModuleRegistration& operator=(const ClientModuleRegistration&) = delete;
    std::uint32_t kind() const { return kind_; }
    const std::string& name() const { return name_; }
private:
    ClientModuleRegistry& registry_;
    std::uint32_t kind_;
    std::string name_;
};

}
#endif
