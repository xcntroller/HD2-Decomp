#ifndef HD2_GAME_HUD_COMMAND_ROUTES_H
#define HD2_GAME_HUD_COMMAND_ROUTES_H
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
namespace hd2::game {
//004B9D50/004B7780. Graph88 format0 is a24-byte world vertex, unlike the
//32-byte transformed text vertex. Native buffers begin at command+75C/C760.
struct HudRouteVertex {float x,y,z;std::uint32_t colour;float u,v;};
static_assert(sizeof(HudRouteVertex)==24);
class HudCommandRoutes {
public:
    struct Route {
        std::array<HudRouteVertex,2048> path{};
        std::uint32_t path_count=0; //C75C: LINE STRIP primitive count, not vertices.
        std::array<HudRouteVertex,512> auxiliary{};
        std::uint32_t auxiliary_count=0; //F760.
    };
    static_assert(offsetof(Route,path_count)==0xc000);
    static_assert(offsetof(Route,auxiliary)==0xc004);
    static_assert(offsetof(Route,auxiliary_count)==0xf004);
    static_assert(sizeof(Route)==0xf008);
    void clear_counts()noexcept{for(auto& route:routes_)route.path_count=route.auxiliary_count=0;}
    auto& routes(){return routes_;}const auto& routes()const{return routes_;}
    auto counts()const{
        std::array<std::array<std::uint32_t,2>,6> result{};
        for(std::size_t n=0;n!=routes_.size();++n)result[n]={{routes_[n].path_count,routes_[n].auxiliary_count}};
        return result;
    }
    //004B9D50 takes the actual route's16-byte position list, ascending. The
    //next path overwrites the last endpoint; do not concatenate disjoint strips.
    template<class Positions>void append(std::size_t index,const std::array<float,3>& colour,const Positions& positions){
        auto& route=routes_.at(index);const auto count=positions.size();
        if(!count||route.path_count>=route.path.size()||count>route.path.size()-route.path_count)
            throw std::runtime_error("HUD command route exceeds its actual native vertex buffer");
        for(std::size_t n=0;n!=count;++n){const auto& position=positions[n];
            const auto packed=(((round_word(127.5f)<<8)|round_colour(colour[0]))<<8|round_colour(colour[1]))<<8|round_colour(colour[2]);
            route.path[route.path_count+n]={position[0],static_cast<float>(static_cast<long double>(position[1])+.1f),position[2],packed,0,0};
        }
        route.path_count+=static_cast<std::uint32_t>(count-1);
    }
    template<class Factory>void draw(Factory& factory){ //004B7780 auxiliary FIRST, then path; six ascending pairs.
        for(auto& route:routes_){
            if(route.auxiliary_count){
                require_count(route.auxiliary_count,route.auxiliary.size());
                factory.hud_draw_command_route(route.auxiliary.data(),route.auxiliary_count);
            }
            if(route.path_count){
                require_count(route.path_count,route.path.size());
                factory.hud_draw_command_route(route.path.data(),route.path_count);
            }
        }
    }
private:
    static void require_count(std::uint32_t count,std::size_t capacity){
        if(count>=capacity)throw std::runtime_error("HUD command route strip has no final vertex");
    }
    //Native FISTP DWORD honours the current rounding mode, unlike007E5E00.
    //The colour multiplication is stored to FLOAT before that conversion.
    static std::uint32_t round_word(float value){
        const auto rounded=std::nearbyint(static_cast<double>(value));
        if(!std::isfinite(rounded)||rounded < -2147483648.||rounded>=2147483648.)return 0x80000000u;
        return static_cast<std::uint32_t>(static_cast<std::int32_t>(rounded));
    }
    static std::uint32_t round_colour(float value){return round_word(static_cast<float>(static_cast<long double>(value)*255.f));}
    std::array<Route,6> routes_{};
};
} //namespace hd2::game
#endif
