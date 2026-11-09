#ifndef HD2_NETWORKING_STARTUP_MODULE_REGISTRY_H
#define HD2_NETWORKING_STARTUP_MODULE_REGISTRY_H

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>

namespace hd2::networking {

// Real module receivers must implement their original +04/+08 bodies. This
// registry owns no modules and supplies no default successful service methods.
class StartupModule {
public:
    virtual ~StartupModule() = default;
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
};

//004A3AB0/004A3AE0: read the live nineteen-slot table at008D5058..008D50A0,
// in ascending order for BOTH operations. Do not snapshot it or reverse cleanup:
// a callback can change entries that a later iteration will observe. Borrowed
// receivers must survive the active virtual call; registry outlives registrations.
class StartupModuleRegistry {
public:
    static constexpr std::size_t count = 19;
    void initialize() {
        for (std::size_t index = 0; index < count; ++index)
            if (slots_[index]) slots_[index]->initialize();
    }
    void shutdown() {
        for (std::size_t index = 0; index < count; ++index)
            if (slots_[index]) slots_[index]->shutdown();
    }
    StartupModule* get(std::size_t index) const { return slots_.at(index); }
    //004A3950 overwrites, without duplicate rejection or ownership transfer.
    void assign(std::size_t index, StartupModule* module) { slots_.at(index) = module; }
private:
    std::array<StartupModule*, count> slots_{}; // Original BSS starts zero.
};

//004A3950/004A3A70: own name/index, register receiver, clear that index before
// releasing name. Destruction clears even if another registration overwrote the
// slot. This surprising original behavior is not an identity-checked removal.
// Native bounds checking replaces the original unchecked table indexing.
class StartupModuleRegistration {
public:
    StartupModuleRegistration(StartupModuleRegistry& registry, StartupModule& module,
                              std::size_t index, const char* name)
        : registry_(registry), name_(name ? name : ""), index_(index) {
        registry_.assign(index_, &module);
    }
    ~StartupModuleRegistration() { registry_.assign(index_, nullptr); }
    StartupModuleRegistration(const StartupModuleRegistration&) = delete;
    StartupModuleRegistration& operator=(const StartupModuleRegistration&) = delete;
    const std::string& name() const noexcept { return name_; }
    std::size_t index() const noexcept { return index_; }
private:
    StartupModuleRegistry& registry_;
    std::string name_;
    std::size_t index_;
};

}
#endif
