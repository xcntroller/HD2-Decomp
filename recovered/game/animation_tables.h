#ifndef HD2_GAME_ANIMATION_TABLES_H
#define HD2_GAME_ANIMATION_TABLES_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace hd2::game {

//00783BC0 and the read/fixup pairs00783710/760,00783490/4E0,
//00783960/9B0,00783B00/B40. These are serialized DWORDs, not native
// pointers. Keep the actual animation reference beside its original words.
// Resource resolution is0041FC10; its failure leaves a null reference and is
// deliberately not promoted to a successful animation resource here.
template<class Resources>
class AnimationTables {
public:
    using Reference = typename Resources::Reference;
    struct Row {
        std::array<std::uint32_t, 3> words{};
        std::string name;
        Reference animation{};
        int load_status = 0;
    };
    static constexpr std::size_t serialized_size = 2 * 0x55f8 + 7 * 0x2220 + 0x189c + 0xf0;
    static constexpr std::size_t row_count = 2 * 786 + 7 * 273 + 225 + 12;

    // Native publication is transactional. A bad stream must not publish a
    // mixture of old/new tables or retain a prefix of acquired animations.
    void load(const std::vector<unsigned char>& bytes, Resources& resources) {
        if (bytes.size() < serialized_size)
            throw std::runtime_error("Tables\\anim.sav is truncated: expected " +
                std::to_string(serialized_size) + " bytes, got " + std::to_string(bytes.size()));
        std::array<std::vector<Row>, 11> next;
        std::size_t offset = 0;
        for (unsigned table = 0; table != next.size(); ++table) {
            const auto count = table < 2 ? 786u : table < 9 ? 273u : table == 9 ? 225u : 12u;
            const auto stride = table < 2 || table == 9 ? 28u : table < 9 ? 32u : 20u;
            next[table].reserve(count);
            for (unsigned i = 0; i != count; ++i, offset += stride) {
                Row row;
                const auto* p = bytes.data() + offset;
                const auto name_offset = table < 2 || table == 9 ? 8u : table < 9 ? 4u : 0u;
                row.name = fixed_name(p + name_offset);
                if (table < 2 || table == 9) {
                    row.words[0] = word(p);
                    row.words[1] = word(p + 4);
                } else if (table < 9) {
                    row.words[0] = word(p);
                    row.words[1] = word(p + 24);
                    row.words[2] = word(p + 28);
                }
                //B40 gates on signed first character; the other fixups use
                //strlen>4. Preserve the two distinct original tests.
                const bool resolve = table == 10
                    ? !row.name.empty() && static_cast<signed char>(row.name.front()) > 32
                    : row.name.size() > 4;
                if (resolve) row.load_status = resources.resolve_animation(row.name, row.animation);
                next[table].push_back(std::move(row));
            }
        }
        tables_.swap(next);
        available_ = true;
    }
    void clear() noexcept {
        // Individual references own their actual resolver results; the cache
        // can outlive this owner without dangling pointers in these rows.
        for (auto& table : tables_) table.clear();
        available_ = false;
    }
    bool available() const { return available_; }
    const std::array<std::vector<Row>, 11>& tables() const { return tables_; }
private:
    static std::uint32_t word(const unsigned char* p) {
        return std::uint32_t(p[0]) | (std::uint32_t(p[1]) << 8) |
            (std::uint32_t(p[2]) << 16) | (std::uint32_t(p[3]) << 24);
    }
    static std::string fixed_name(const unsigned char* p) {
        const auto* end = static_cast<const unsigned char*>(std::memchr(p, 0, 20));
        if (!end) throw std::runtime_error("Unterminated animation-table resource name");
        return {reinterpret_cast<const char*>(p), static_cast<std::size_t>(end - p)};
    }
    std::array<std::vector<Row>, 11> tables_;
    bool available_ = false;
};

//00783350 sets the attempt latch BEFORE00783BC0, including failed loads. Its
//return value alone therefore cannot establish readiness on a later call.
class AnimationLoadGate {
public:
    template<class Load> bool original_result(Load&& load) {
        if (attempted_) return true;
        attempted_ = true;
        return std::forward<Load>(load)();
    }
    bool attempted() const { return attempted_; }
private:
    bool attempted_ = false;
};

} // namespace hd2::game
#endif
