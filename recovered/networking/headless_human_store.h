#ifndef HD2_NETWORKING_HEADLESS_HUMAN_STORE_H
#define HD2_NETWORKING_HEADLESS_HUMAN_STORE_H

#include "frontend/human_store.h"
#include <utility>

namespace hd2::networking {

//006E33E0/006E36E0's required startup data owner. Server-address contracts are
// checked separately from client mappings. No graphics service is supplied:
// original006E2730 does no device work and006E0DF0 clears its pointer directly.
class HeadlessHumanStore {
    struct Headless {};
public:
    HeadlessHumanStore() : data_(headless_) {}
    HeadlessHumanStore(const HeadlessHumanStore&) = delete;
    HeadlessHumanStore& operator=(const HeadlessHumanStore&) = delete;
    template<class Files, class ProfileDirectory>
    bool load(Files& files,const frontend::LocalizedTextCache& text,
            ProfileDirectory&& directory,bool wide) {
        return data_.load_with_profile_directory(files,"tables\\HumanStore.def",text,
            std::forward<ProfileDirectory>(directory),wide);
    }
    const auto& records() const { return data_.records(); }
    const auto& default_names() const { return data_.default_names(); }
    const frontend::HumanRecord* record(std::int32_t index) const { return data_.record(index); }
    void reset(const frontend::LocalizedTextCache& text) { data_.reset(text); }
private:
    Headless headless_;
    frontend::HumanStore<Headless,false> data_;
};

} // namespace hd2::networking
#endif
