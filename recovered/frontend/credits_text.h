#ifndef HD2_FRONTEND_CREDITS_TEXT_H
#define HD2_FRONTEND_CREDITS_TEXT_H

#include "frontend/opening_subtitles.h"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace hd2::frontend {

// Native style/record storage for006887C0 and table008412C0 (14 x52 bytes).
// All entries are white, centered/top aligned, with wrapping disabled. Column
// styles set +2c and flags18/10; the remaining style fields are zero or -1.
struct CreditsStyle { unsigned font; float height; float column; bool new_row; };
inline constexpr std::array<CreditsStyle,14> credits_styles{{
    {2,.06f,-1,true}, {3,.04f,-1,true}, {3,.022f,-1,true},
    {3,.04f,-1,true}, {3,.025f,-1,true}, {5,.06f,-1,true},
    {6,.04f,-1,true}, {6,.022f,-1,true},
    {6,.022f,.25f,true}, {6,.022f,.5f,false}, {6,.022f,.75f,false},
    {3,.022f,.25f,true}, {3,.022f,.5f,false}, {3,.022f,.75f,false}
}};
struct CreditsRecord { std::string text; std::uint8_t style; };
struct CreditsDocument {
    std::vector<CreditsRecord> records;
    float speed;
};

//00689340: signed localized-ID order; exact tags, raw single-byte payloads.
// Credits text is independent of the ordinary subtitle preference. Consumes the
// actual cache, including on an empty/missing file; no movie or voice is opened.
inline CreditsDocument parse_credits(LocalizedTextCache& input, int width) {
    const auto rate = [width](std::int32_t value) {
        return static_cast<float>(static_cast<long double>(width) * (1.0f/1024) * value * .001f);
    };
    CreditsDocument result{{},rate(50)};
    const std::array<const char*,8> tags{{"TI","NA","SM","EL","ES","TICZ","NACZ","SMCZ"}};
    for (const auto& entry : input) {
        const std::string line(entry.second.text.c_str());
        const auto left = line.find('<'), right = line.find('>');
        if (left == std::string::npos || right == std::string::npos || right <= left + 1) continue;
        const auto tag = line.substr(left+1,right-left-1);
        const auto payload = line.substr(right+1);
        if (tag == "SPEED") {
            const auto first = payload.find_first_not_of(" \t\r\n\v\f");
            auto digit = first;
            if (digit != std::string::npos && (payload[digit]=='+' || payload[digit]=='-')) ++digit;
            if (digit < payload.size() && payload[digit]>='0' && payload[digit]<='9') {
                const auto bits = subtitle_integer(payload.c_str());
                std::int32_t value; std::memcpy(&value,&bits,4);
                result.speed = rate(value);
            }
            continue;
        }
        const auto found = std::find(tags.begin(),tags.end(),tag);
        if (found == tags.end()) continue;
        const auto style = static_cast<std::uint8_t>(found-tags.begin());
        if (style == 2 || style == 7) {
            const auto a=payload.find("<C_1>"), b=payload.find("<C_2>"), c=payload.find("<C_3>");
            if (a!=std::string::npos && b!=std::string::npos && c!=std::string::npos) {
                // Original replaces each marker's '<' with NUL and reads its
                // following bytes. Bound each slice at the next marker/NUL.
                for (const auto start : {a,b,c}) {
                    const auto end=std::min({a>start?a:payload.size(),b>start?b:payload.size(),c>start?c:payload.size()});
                    const auto index=static_cast<unsigned>(start==a?0:start==b?1:2);
                    result.records.push_back({payload.substr(start+5,end-(start+5)),
                        static_cast<std::uint8_t>((style==7?8:11)+index)});
                }
                continue;
            }
        }
        result.records.push_back({payload,style});
    }
    input.clear();
    return result;
}

//00688C10/+14 float offset and0064B940 low signed word. Defined conversion
// preserves x86 low-word behavior; reject nonfinite/overflow instead of C++ UB.
inline std::int16_t credits_short(std::int64_t value) {
    const auto bits=static_cast<std::uint16_t>(static_cast<std::uint64_t>(value));
    std::int16_t result; std::memcpy(&result,&bits,2); return result;
}
inline std::int16_t credits_scroll_word(float value) {
    if (!std::isfinite(value) || value < -2147483648.0 || value >= 2147483648.0)
        throw std::out_of_range("Credits scroll exceeds original signed word conversion");
    return credits_short(static_cast<std::int32_t>(value));
}
class CreditsScroll {
public:
    CreditsScroll(int width,int height,float speed)
        : offset_(static_cast<float>(-static_cast<long double>(width)*(1.0f/1024)*(height+100))),speed_(speed) {}
    bool update(std::uint32_t milliseconds,bool exhausted) {
        offset_=static_cast<float>(static_cast<long double>(offset_)+static_cast<long double>(milliseconds)*speed_);
        if (exhausted) { completion_+=milliseconds; return completion_>=3000; }
        return false;
    }
    std::int16_t word() const { return credits_scroll_word(offset_); }
private:
    float offset_,speed_;
    std::uint32_t completion_=0;
};

} // namespace hd2::frontend
#endif
