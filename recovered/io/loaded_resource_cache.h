#ifndef HD2_IO_LOADED_RESOURCE_CACHE_H
#define HD2_IO_LOADED_RESOURCE_CACHE_H

#include "io/resource_directories.h"
#include <functional>
#include <cstring>
#include <map>
#include <utility>

namespace hd2::io {

template<class Caches>
void clear_controller_loaded_caches(Caches& caches) {
    for (const auto category : {2u, 1u, 3u, 4u, 5u}) caches.cache(category).clear(); //006C0920.
}

template<class Caches>
void clear_final_loaded_caches(Caches& caches) {
    for (const auto category : {1u, 2u, 4u, 5u}) caches.cache(category).clear(); //006DC3A0; no3.
}

//004290A0/0044A1C0/006D5DF0 and004C76C0/006D7EA0/0041FC10/006D64D0.
// Backend supplies actual factory/open/copy/retain/probe/release policies for
// each family. Directory state is borrowed from the already populated owner.
// Tree values borrow the objects owned by entries; null lookup nodes persist
// after misses/evictions. Native containers replace COW strings/RB-tree ABI.
template<class Backend>
class LoadedResourceCache {
public:
    using Resource = typename Backend::Resource;
    using Owner = typename Backend::Owner;
    LoadedResourceCache(Backend& backend, ResourceDirectoryList& directories)
        : backend_(backend), directories_(directories) {}
    ~LoadedResourceCache() { clear(); }
    LoadedResourceCache(const LoadedResourceCache&) = delete;
    LoadedResourceCache& operator=(const LoadedResourceCache&) = delete;

    void capacity(std::uint32_t value) { capacity_ = value; } // +00, default256.
    std::size_t size() const { return entries_.size(); }
    std::size_t lookup_size() const { return lookup_.size(); }
    Resource* current() const { return current_.get(); }

    int load_into(Resource* destination, const std::string& name, std::uint32_t flags = 0,
                  bool callback_present = false) {
        // Category3's +04 is the actual RET/zero006D5600, not a loader.
        if (!backend_.loads_into()) return 0;
        if (!destination) throw std::invalid_argument("Cache destination is null");
        for (auto count = entries_.size(); count != 0; --count) {
            if (entries_[count - 1].object.get() == destination) {
                remove(count - 1);
                break; // Original removes the LAST pointer match, before lookup.
            }
        }
        const auto key = directory_c_string(name);
        if (auto* hit = lookup_[key]) { backend_.copy(destination, hit); return 0; }
        const auto status = search(destination, key, flags | 0x10u | (callback_present ? 4u : 0u));
        if (status >= 0) {
            evict_one();
            auto prototype = backend_.create();
            if (!prototype) throw std::runtime_error("Cache copy factory returned null");
            backend_.copy(prototype.get(), destination);
            insert(key, std::move(prototype));
        }
        return status;
    }

    // Get retains a hit. On a miss, load +38 scratch, allocate its replacement,
    // publish old scratch to the cache/caller, and install replacement last.
    int get(const std::string& name, Owner& output,
            const std::function<void(const std::string&)>& failed = {}) {
        output.reset();
        const auto key = directory_c_string(name);
        if (auto* hit = lookup_[key]) { output = backend_.retain(hit); return 0; }
        if (!current_) current_ = backend_.create();
        if (!current_) throw std::runtime_error("Cache scratch factory returned null");
        const auto status = search(current_.get(), key, failed ? 4u : 0u);
        if (status < 0) {
            if (!terminal(status) && failed) failed(key);
            return status;
        }
        evict_one();
        auto next = backend_.create();
        if (!next) return signed_status(0x80000005u);
        // Native exception-safe publication: allocation failure cannot leave a
        // null record in the original unguarded vector insertion sequence.
        output = backend_.retain(current_.get());
        insert(key, std::move(current_));
        current_ = std::move(next);
        return status;
    }

    void clear() {
        // Original clears references in forward insertion order, then the tree,
        // then +38. Registered search directories and capacity survive.
        for (auto& entry : entries_) entry.object.reset();
        entries_.clear();
        lookup_.clear();
        current_.reset();
    }
private:
    static int signed_status(std::uint32_t bits) {
        static_assert(sizeof(int) == sizeof(bits));
        int result; std::memcpy(&result, &bits, sizeof(result)); return result;
    }
    static bool terminal(int status) {
        return status == signed_status(0x80000002u) || status == signed_status(0x8000000au);
    }
    int search(Resource* object, const std::string& key, std::uint32_t flags) {
        int status = signed_status(0x80000001u);
        // Resolve count/path afresh each iteration; engine calls can mutate
        // directory state. Preserve registered order, spelling and duplicates.
        for (std::size_t index = 0; index < directories_.paths().size(); ++index) {
            const auto path = directories_.paths()[index] + key;
            status = backend_.open(object, path, flags);
            if (terminal(status)) return status;
            if (status == signed_status(0x80000013u) || status == signed_status(0x80000014u))
                backend_.diagnostic(path, status);
            if (status >= 0) return status;
        }
        return status;
    }
    struct Entry { Owner object; std::string key; };
    void insert(const std::string& key, Owner object) {
        auto* raw = object.get();
        entries_.push_back({std::move(object), key});
        lookup_[key] = raw;
    }
    void remove(std::size_t index) {
        lookup_[entries_.at(index).key] = nullptr;
        entries_.erase(entries_.begin() + static_cast<std::ptrdiff_t>(index));
    }
    void evict_one() {
        if (entries_.size() < capacity_) return;
        for (std::size_t index = 0; index < entries_.size(); ++index) {
            // Original retain then Release probes the actual remaining count.
            // Capacity is SOFT: pinned entries may force growth beyond it.
            if (backend_.probe_references(entries_[index].object.get()) == 1) {
                remove(index); break;
            }
        }
    }
    Backend& backend_;
    ResourceDirectoryList& directories_;
    std::uint32_t capacity_ = 256;
    std::vector<Entry> entries_;
    std::map<std::string, Resource*> lookup_;
    Owner current_{nullptr, nullptr};
};

}
#endif
