#ifndef HD2_FRONTEND_MODULE_REGISTRATION_H
#define HD2_FRONTEND_MODULE_REGISTRATION_H

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <memory>

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

//Source composition adapter: production gameplay owners borrow their existing
//process registration. Standalone source fixtures can still own a registration.
//Borrowing never overwrites/unregisters a slot or changes0048D950/0048DA30's
//actual owned lifetime above. Registry and borrowed registration must outlive it.
class ClientModuleBinding {
public:
    ClientModuleBinding(ClientModuleRegistry& registry,std::uint32_t kind,const char* name)
        :owner_(std::make_unique<ClientModuleRegistration>(registry,kind,name)),registration_(owner_.get()){}
    ClientModuleBinding(ClientModuleRegistry& registry,ClientModuleRegistration& registration,std::uint32_t kind,const char* name)
        :registration_(&registration) {
        if(kind>=registry.size()||registry[kind]!=&registration||registration.kind()!=kind||registration.name()!=(name?name:""))
            throw std::logic_error("Gameplay owner requires its actual process module registration");
    }
    ClientModuleBinding(const ClientModuleBinding&)=delete;ClientModuleBinding& operator=(const ClientModuleBinding&)=delete;
    ClientModuleRegistration& get(){return *registration_;}const ClientModuleRegistration& get()const{return *registration_;}
    bool owns_registration()const{return bool(owner_);}
private:
    std::unique_ptr<ClientModuleRegistration> owner_;ClientModuleRegistration* registration_;
};

}
#endif
