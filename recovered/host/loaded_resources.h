#ifndef HD2_HOST_LOADED_RESOURCES_H
#define HD2_HOST_LOADED_RESOURCES_H

#include "host/menu_scene.h"
#include "io/loaded_resource_cache.h"

namespace hd2::host {

// Actual engine services for the five separate client cache owners. Nothing
// calls original client code. Engine frame/animation factories and I/O remain
// delegated and guarded against the fingerprinted virtual targets.
class CachedResourceBackend {
public:
    using Resource = EngineFrame;
    using Owner = MenuDriver::Owner;
    CachedResourceBackend(HMODULE module, unsigned category) : module_(module), category_(category) {}
    bool loads_into() const { return category_ != 3; } //006D5600 is RET/zero.
    Owner create() const {
        const auto entry = GetProcAddress(module_, "I3DGetDriver");
        if (reinterpret_cast<std::uintptr_t>(entry) != reinterpret_cast<std::uintptr_t>(module_) + 0x68fb0)
            throw std::runtime_error("Unsupported cache I3DGetDriver export");
        using Getter = EngineFrame* (__cdecl *)();
        Getter get; std::memcpy(&get, &entry, sizeof(get));
        auto* driver = get();
        EngineFrame* resource;
        if (category_ == 3) {
            using Create = EngineFrame* (WINAPI *)(EngineFrame*);
            resource = scene_method<Create>(driver, module_, 0x70, 0x18d20)(driver);
        } else {
            const auto kind = category_ == 2 ? 4u : category_ == 4 ? 15u : 9u;
            resource = scene_method<MenuDriver::Factory>(driver, module_, 0x54, 0x19e10)(driver, kind);
        }
        if (!resource) return Owner(nullptr, nullptr);
        return Owner(resource, release_method(resource));
    }
    Owner retain(Resource* resource) const {
        add_reference(resource);
        return Owner(resource, release_method(resource));
    }
    int probe_references(Resource* resource) const {
        add_reference(resource);
        return release_method(resource)(resource);
    }
    int open(Resource* resource, const std::string& path, std::uint32_t flags) const {
        if (category_ == 1 || category_ == 5) {
            using Open = int (WINAPI *)(EngineFrame*, const char*, std::uint32_t, void*, void*);
            return scene_method<Open>(resource, module_, 0x74, 0x340b0)(resource, path.c_str(), flags, nullptr, nullptr);
        }
        using Open = int (WINAPI *)(EngineFrame*, const char*, std::uint32_t);
        const auto slot = category_ == 3 ? 4u : 0x74u;
        const auto rva = category_ == 2 ? 0x545a0u : category_ == 3 ? 0x04ce0u : 0x4f120u;
        return scene_method<Open>(resource, module_, slot, rva)(resource, path.c_str(), flags);
    }
    void copy(Resource* destination, Resource* source) const {
        if (category_ == 2) {
            //006D52D0 reads the source sound's resource-name owner +140;
            //0051C0D0 / engine10056150 resolve its +0c COW string (+8 data).
            const void* file = field<const void*>(source, 0x140);
            const char* name = "";
            if (file) {
                const void* cow = field<const void*>(file, 0x0c);
                if (cow) name = static_cast<const char*>(cow) + 8;
            }
            using Open = int (WINAPI *)(EngineFrame*, const char*, std::uint32_t);
            // Original ignores this copy-open result, unlike initial search.
            scene_method<Open>(destination, module_, 0x74, 0x545a0)(
                destination, name, field<std::uint32_t>(source, 0x178));
            return;
        }
        if (category_ == 3) return; // Actual006D5610 RET8; +04 does not copy.
        if (category_ == 1 || category_ == 5) {
            using Clone = int (WINAPI *)(EngineFrame*, EngineFrame*);
            scene_method<Clone>(destination, module_, 0xd8, 0x34760)(destination, source);
        }
        //006D4F90/006D5CC0 enumerate the destination AFTER clone; category4's
        //006D5920 only enumerates destination and does not copy the source.
        using Callback = int (WINAPI *)(EngineFrame*, void*);
        using Enumerate = void (WINAPI *)(EngineFrame*, Callback, void*, std::uint32_t, const char*);
        scene_method<Enumerate>(destination, module_, 0x44, 0x1de40)(destination, clear_pick_word, nullptr, 0xffff, nullptr);
    }
    void diagnostic(const std::string& path, int status) const {
        const auto message = std::string("Cache - ") +
            (static_cast<std::uint32_t>(status) == 0x80000013u ? "BADFILEVERSION " : "BADFILETIME ") + path + "\n";
        OutputDebugStringA(message.c_str());
    }
private:
    static int WINAPI clear_pick_word(EngineFrame* frame, void*) {
        const std::uint32_t zero = 0;
        std::memcpy(reinterpret_cast<unsigned char*>(frame) + 0xec, &zero, sizeof(zero));
        return 0; //006D4FC0/006D5CF0/006D59B0.
    }
    template<class Value> static Value field(const void* object, std::size_t offset) {
        Value value; std::memcpy(&value, static_cast<const unsigned char*>(object) + offset, sizeof(value));
        return value;
    }
    void add_reference(Resource* resource) const {
        if (!resource) throw std::invalid_argument("Cannot retain a null cache resource");
        if (category_ == 3) {
            auto count = field<std::uint32_t>(resource, 4) + 1;
            std::memcpy(reinterpret_cast<unsigned char*>(resource) + 4, &count, sizeof(count));
        } else {
            const auto count = static_cast<std::uint16_t>(field<std::uint16_t>(resource, 0xe4) + 1);
            std::memcpy(reinterpret_cast<unsigned char*>(resource) + 0xe4, &count, sizeof(count));
        }
    }
    MenuDriver::Release release_method(Resource* resource) const {
        return scene_method<MenuDriver::Release>(resource, module_, 0, category_ == 3 ? 0x04c80 : 0x1c370);
    }
    HMODULE module_;
    unsigned category_;
};

class LoadedResources final : public MenuResourceServices {
public:
    using Cache = io::LoadedResourceCache<CachedResourceBackend>;
    template<class Directories>
    LoadedResources(HMODULE module, Directories& directories)
        : backends_{{{module,1}, {module,2}, {module,3}, {module,4}, {module,5}}} {
        for (unsigned i = 0; i != caches_.size(); ++i)
            caches_[i] = std::make_unique<Cache>(backends_[i], directories.client(i + 1));
    }
    Cache& cache(unsigned category) {
        if (category < 1 || category > 5) throw std::out_of_range("Invalid loaded-resource category");
        return *caches_[category - 1];
    }
    int load_model(EngineFrame* model, const std::string& key, std::uint32_t flags) override {
        const auto status = cache(1).load_into(model, key, flags);
        std::printf("Client model cache: key=%s status=%08x entries=%zu\n", key.c_str(), static_cast<unsigned>(status), cache(1).size());
        return status;
    }
    int load_sound(EngineFrame* sound, const std::string& key, std::uint32_t flags) override {
        const auto status = cache(2).load_into(sound, key, flags);
        std::printf("Client sound cache: key=%s status=%08x entries=%zu\n", key.c_str(), static_cast<unsigned>(status), cache(2).size());
        return status;
    }
    void clear_controller() {
        //006C0920, AFTER state shutdown and BEFORE state deletion/mouse close.
        io::clear_controller_loaded_caches(*this);
        std::puts("Controller loaded-resource caches cleared: 2/1/3/4/5; directories retained");
    }
    void clear_final() {
        //006DC3A0 final tail omits category3. Native failure fallback reclaims
        // any category3 acquired without a completed controller shutdown too.
        io::clear_final_loaded_caches(*this);
        cache(3).clear();
        std::puts("Shutdown: loaded-resource caches released before engine drivers");
    }
private:
    std::array<CachedResourceBackend,5> backends_;
    std::array<std::unique_ptr<Cache>,5> caches_;
};

}
#endif
