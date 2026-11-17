#ifndef HD2_GAME_PROCEDURAL_CLOUDS_H
#define HD2_GAME_PROCEDURAL_CLOUDS_H
#include "game/volumetric_lights.h"
namespace hd2::game {
//Separate process0089B63C, NOT GEffects+C4/224. Texture slots gate00482E50;
//the passed clock is unused by the original body. Resource owners use the
//existing graphics release service; these buffers are original10368x24 bytes.
template<class TextureOwner>class ProceduralClouds {
public:
    static constexpr std::size_t capacity=0x3cc00/sizeof(VolumetricVertex);
    ProceduralClouds()=default;
    ProceduralClouds(const ProceduralClouds&)=delete;
    ProceduralClouds& operator=(const ProceduralClouds&)=delete;
    ~ProceduralClouds(){clear();}
    void clear()noexcept{ //00483DE0, release textures BEFORE the two buffers.
        textures_[0].reset();textures_[1].reset();buffers_[0].reset();buffers_[1].reset();
    }
    template<class Frames>void configure(bool enabled,Frames& frames){ //00485B50/GEffects virtualB8.
        clear();if(!enabled)return;
        //00483E50 ignores load status, retains actual out pointers, and allocates
        //both buffers even when a texture is absent. Partial throws are retained
        //for clear/retry, not reported as successful completed initialization.
        textures_[0].emplace(frames.load_procedural_cloud_texture("clouder.tga"));
        textures_[1].emplace(frames.load_procedural_cloud_texture("clouder2.tga"));
        buffers_[0]=std::make_unique<VolumetricVertex[]>(capacity);
        buffers_[1]=std::make_unique<VolumetricVertex[]>(capacity);
    }
    bool has_texture(unsigned slot)const{return textures_.at(slot)&&bool(*textures_.at(slot));}
    template<class Frames>void render(std::int32_t /*time*/,Frames& frames){ //00482E50 RET4.
        if(!has_texture(0))return; //Legitimate actual missing texture, not a forced flag.
        if(!buffers_[0]||!buffers_[1])throw std::runtime_error("Incomplete procedural geometry initialization");
        const auto eye_matrix=frames.car_world_matrix(frames.volume_render_camera());
        const auto direction_matrix=frames.car_world_matrix(frames.volume_render_camera());
        const auto forward=normalize({direction_matrix[8],direction_matrix[9],direction_matrix[10]});
        const auto up_matrix=frames.car_world_matrix(frames.volume_render_camera());
        const std::array<float,4> up{up_matrix[4],up_matrix[5],up_matrix[6],up_matrix[7]};
        std::array<std::int32_t,3> centre{};
        for(unsigned axis=0;axis!=3;++axis){
            const auto value=std::trunc(static_cast<long double>(eye_matrix[12+axis])*.0625f);
            if(!std::isfinite(value)||value<-2147483642.L||value>2147483641.L)
                throw std::runtime_error("Procedural camera exceeds native lattice range");
            centre[axis]=static_cast<std::int32_t>(value);
        }
        std::array<std::size_t,2> counts{};
        for(auto x=centre[0]-6;x<centre[0]+6;++x)
        for(auto y=centre[1]-6;y<centre[1]+6;++y)
        for(auto z=centre[2]-6;z<centre[2]+6;++z){
            const auto bits=std::uint32_t(x)*2u+std::uint32_t(y)*5u+std::uint32_t(z)*3u;
            std::int32_t index;std::memcpy(&index,&bits,4);
            const auto sine=std::sin(static_cast<long double>(index));
            const float stored_sine=static_cast<float>(sine);
            const float size=static_cast<float>(sine*.75f*4.f+12.199999809265137f);
            const std::array<float,3> position{
                static_cast<float>((static_cast<long double>(x)+stored_sine)*16.f),
                static_cast<float>((static_cast<long double>(y)+std::cos(static_cast<long double>(index)))*16.f),
                static_cast<float>((static_cast<long double>(z)+stored_sine)*16.f)};
            //The original retains X in x87 while storing Y/Z before length.
            const long double dx=static_cast<long double>(eye_matrix[12])-position[0];
            const float dy=static_cast<float>(static_cast<long double>(eye_matrix[13])-position[1]);
            const float dz=static_cast<float>(static_cast<long double>(eye_matrix[14])-position[2]);
            const float squared=static_cast<float>(dx*dx+static_cast<long double>(dz)*dz+static_cast<long double>(dy)*dy);
            const auto toward=normalize({dx,dy,dz});
            const std::array<float,4> away{-toward[0],-toward[1],-toward[2],0};
            if(static_cast<long double>(away[0])*forward[0]+static_cast<long double>(forward[2])*away[2]+static_cast<long double>(forward[1])*away[1]<0)continue;
            float alpha=.4f;
            if(squared<400.f){
                alpha=squared<=100.f?0:static_cast<float>((std::sqrt(static_cast<long double>(squared))-10.)*.1*0.4000000059604645);
            }else if(squared>2025.f)
                alpha=static_cast<float>((1.L-(static_cast<long double>(squared)-2025.f)*.00033057850669138134f)*.4f);
            alpha=std::min(1.f,std::max(0.f,alpha));
            if(alpha<=.0010000000474974513f)continue;
            auto matrix=frames.procedural_cloud_matrix(away,up); //Actual full SetDir, not SetDir3.
            matrix[12]=position[0];matrix[13]=position[1];matrix[14]=position[2];
            const float scaled=static_cast<float>(static_cast<long double>(alpha)*255.f);
            const auto rounded=std::nearbyint(static_cast<long double>(scaled)); //Stored FLOAT then current-mode FISTP.
            const auto colour=(static_cast<std::uint32_t>(static_cast<std::int32_t>(rounded))<<24)|0x00ffffffu;
            const unsigned batch=stored_sine>0?0:1;
            auto& count=counts[batch];if(count+6>capacity)throw std::runtime_error("Procedural geometry exceeds original buffer");
            auto* vertices=buffers_[batch].get()+count;
            auto vertex=[&](unsigned slot,float px,float py,float u,float v){
                const auto transformed=frames.formation_transform({px,py,0,0},matrix);
                vertices[slot]={{transformed[0],transformed[1],transformed[2]},colour,{u,v}};
            };
            //004835C8/36AD/3793/3876 and copies3887..38BE: exact triangles.
            vertex(2,-size,-size,0,0);vertex(1,size,-size,1,0);
            vertex(0,size,size,1,1);vertex(3,-size,size,0,1);
            vertices[4]=vertices[0];vertices[5]=vertices[2];count+=6;
        }
        frames.hud_driver_state(3,0); //Intentional original state; containing00482260 restores it.
        const std::array<float,16> identity{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        for(unsigned batch=0;batch!=2;++batch)if(counts[batch]){
            //Second texture may legitimately be null: original binds it anyway.
            frames.volume_bind_texture(has_texture(batch)?textures_[batch]->get():nullptr);
            frames.volume_world_transform(identity);
            frames.volume_draw(buffers_[batch].get(),static_cast<std::uint32_t>(counts[batch]/3));
        }
    }
private:
    static std::array<float,3> normalize(std::array<long double,3> vector){
        const float squared=static_cast<float>(vector[0]*vector[0]+vector[2]*vector[2]+vector[1]*vector[1]);
        if(!std::isfinite(squared))throw std::runtime_error("Non-finite procedural camera direction");
        if(std::abs(static_cast<long double>(squared)-1)>=9.99999993922529e-09f){
            if(squared>=9.99999993922529e-09f){const auto scale=1.L/std::sqrt(static_cast<long double>(squared));for(auto& v:vector)v*=scale;}
            else if(vector[0]!=0)vector[0]=vector[0]<0?-1:1;
            else if(vector[2]!=0)vector[2]=vector[2]<0?-1:1;
            else vector[1]=vector[1]<0?-1:1;
        }
        return {static_cast<float>(vector[0]),static_cast<float>(vector[1]),static_cast<float>(vector[2])};
    }
    std::array<std::optional<TextureOwner>,2> textures_;
    std::array<std::unique_ptr<VolumetricVertex[]>,2> buffers_;
};
}
#endif
