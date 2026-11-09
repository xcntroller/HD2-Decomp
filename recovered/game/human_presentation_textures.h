#ifndef HD2_GAME_HUMAN_PRESENTATION_TEXTURES_H
#define HD2_GAME_HUMAN_PRESENTATION_TEXTURES_H

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

namespace hd2::game {

//0078A4F0's thirteen records at0084F43C, stride1C. This is the texture
//dependency of the shared presentation, not its runtime mark/model pools.
//Construct on first active Human and clear on last, before graphics teardown.
template<class Frames>
class HumanPresentationTextures {
public:
    using Owner=typename Frames::ResourceOwner;
    struct Definition {const char* prefix;std::int32_t key;};
    inline static constexpr std::array<Definition,13> definitions{{
        {"s_",35},{"s_",36},{"s_",41},{"a_",42},{"a_",86},{"a_",91},
        {"s_",43},{"d_",44},{"d_",-2},{"b_",-3},{"b_",-4},{"w_",-5},{"w_",40}
    }};
    explicit HumanPresentationTextures(Frames& frames):frames_(frames) {}
    ~HumanPresentationTextures(){clear();}
    HumanPresentationTextures(const HumanPresentationTextures&)=delete;
    HumanPresentationTextures& operator=(const HumanPresentationTextures&)=delete;
    void load(std::uint32_t mode) {
        if(loaded_)throw std::logic_error("Human presentation textures loaded twice");
        if(!(mode&2)){loaded_=true;return;}
        try {
            for(unsigned index=0;index!=definitions.size();++index)for(unsigned side=0;side!=2;++side) {
                const auto name=std::string(definitions[index].prefix)+(side?"r.tga":"l.tga");
                auto texture=frames_.load_car_particle_texture(name,"",0x402);
                if(!texture)throw std::runtime_error("Required Human presentation texture failed: "+name);
                textures_[index][side]=std::move(texture);
            }
            loaded_=true;
        }catch(...) {clear();throw;} //Never publish native partial acquisition as ready.
    }
    void clear() noexcept { //Texture tail of0078A5F0, ascending record then L/R.
        for(auto& pair:textures_)for(auto& texture:pair)texture.reset();
        loaded_=false;
    }
    const auto& textures() const{return textures_;}
    bool loaded() const{return loaded_;}
private:
    Frames& frames_;std::array<std::array<Owner,2>,13> textures_{};
    bool loaded_=false;
};
} //namespace hd2::game
#endif
