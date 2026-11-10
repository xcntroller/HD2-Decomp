#ifndef HD2_GAME_CAR_LIGHTING_H
#define HD2_GAME_CAR_LIGHTING_H

#include "game/car_geometry_math.h"
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace hd2::game {

//004E7450 light bindings, with metadata from004E87C0/004E8D80. Actual engine
//projectors, lights and textures belong to mandatory Frames::car_projector.
template<class Frames>
class CarLighting {
public:
    using Frame=typename Frames::Frame;
    using Vector=typename Frames::Vector;
    using Projector=typename Frames::CarProjector;
    static constexpr std::size_t none=std::numeric_limits<std::size_t>::max();
    struct Light {
        Frame frame{},billboard{};
        std::size_t projector=none;
        std::uint16_t type=0,part1a=0xffff;
        std::uint32_t flags10=0;float field14=0; //SAME native594 records used by damage/part detachment.
        bool shared=false;
    };
    void scale_projectors(Frames& frames,std::uint32_t mask,const Vector& scale)const { //004F4E40.
        struct Family {std::uint32_t mask;std::uint16_t first,last;};
        constexpr Family families[]={{0x100000,1,2},{0x200000,5,6},{0x400000,3,4},
                                     {0x800000,7,7},{0x1000000,8,8},{0x2000000,9,9}};
        for(const auto& family:families)if(mask&family.mask)for(auto n=lights_.size();n;) {
            const auto& light=lights_[--n];
            if(light.type<family.first||light.type>family.last||light.projector==none)continue;
            if(const auto visual=projectors_.at(light.projector).visual.get())frames.car_projector_scale(visual,scale);
        }
    }
    template<class Parts> void discover(Frames& frames,Frame model,const Parts& parts) {
        struct Family {const char* pattern;std::uint16_t first,second;};
        constexpr Family families[]={{"LIGHTRE*",7,7},{"LIGHTIN*",1,2},{"LIGHTBI*",3,4},
                                     {"LIGHTLI*",5,6},{"LIGHTBR*",8,8},{"LIGHTBA*",9,9}};
        for(const auto& family:families)for(auto frame:frames.car_parts(model,family.pattern,0x10021)) {
            Light record;record.frame=frame;
            record.billboard=frames.car_find(frame,"BILLBOARD*",0x1ffff);
            if(record.billboard)frames.object_visible(record.billboard,false);
            std::array<std::string,3> names{};
            if(single_texture(frames.car_user_data(frame),names[0])) {
                const auto world=frames.car_world_matrix(frame);
                auto projector=frames.car_projector(frame,names,1,car_normalize(Vector{world[8],world[9],world[10],0}));
                if(projector.visual) {record.projector=projectors_.size();projectors_.push_back(std::move(projector));}
            }
            record.type=family.first;
            if(family.first!=family.second) {
                const auto world=frames.car_world_matrix(frame);
                const auto position=frames.car_transform(Vector{world[12],world[13],world[14],0},
                    frames.car_inverse_matrix(frames.car_world_matrix(model)));
                if(position[0]>0)record.type=family.second;
            }
            lights_.push_back(record);
        }
        bind_parts(frames,model,parts);
        for(auto marker:frames.car_parts(model,"PROJECTOR*",0x10020)) {
            const auto children=frames.car_parts(marker,"LIGHT*",0x10021);
            if(children.size()!=2)continue;
            const auto first=find_unprojected(children[0]),second=find_unprojected(children[1]);
            if(first==none||second==none||first==second)continue;
            std::array<std::string,3> names{};
            if(!three_textures(frames.car_user_data(marker),names))continue;
            const auto world=frames.car_world_matrix(model);
            auto projector=frames.car_projector(marker,names,3,Vector{world[8],world[9],world[10],0});
            if(!projector.visual)continue;
            const auto index=projectors_.size();projectors_.push_back(std::move(projector));
            lights_[first].projector=index;lights_[second].projector=index;
            lights_[first].shared=lights_[second].shared=true;
            lights_[first].flags10|=0x80000000u;lights_[second].flags10|=0x80000000u;
        }
    }
    const std::vector<Light>& lights() const {return lights_;}
    std::vector<Light>& lights() {return lights_;}
    const std::vector<Projector>& projectors() const {return projectors_;}
private:
    static bool single_texture(const std::string& text,std::string& name) {
        const auto first=text.find('['),last=text.rfind(']');
        if(text.size()<=2||first==std::string::npos||last==std::string::npos||last<=first)return false;
        name=text.substr(first+1,last-first-1);
        //Original accepts an empty [] within a longer attribute string.
        if(text.size()-first-1>=140)throw std::runtime_error("Car projector metadata exceeds original buffer");
        return true;
    }
    static bool three_textures(const std::string& text,std::array<std::string,3>& names) {
        if(text.size()<=2)return false;
        auto begin=text.find('[');if(begin==std::string::npos)return false;
        ++begin;
        for(std::size_t i=0;i!=3;++i) {
            const auto end=text.find(i==2?']':',',begin);
            if(end==std::string::npos)return false;
            if(end-begin>=32)throw std::runtime_error("Car projector texture name exceeds original buffer");
            names[i]=text.substr(begin,end-begin);begin=end+1;
        }
        return true;
    }
    std::size_t find_unprojected(Frame frame) const {
        for(auto i=lights_.size();i;--i)if(lights_[i-1].frame==frame&&lights_[i-1].projector==none)return i-1;
        return none;
    }
    template<class Parts> void bind_parts(Frames& frames,Frame model,const Parts& parts) {
        for(auto light=lights_.rbegin();light!=lights_.rend();++light) {
            auto match=[&](Frame frame,bool exact) {
                for(auto i=parts.size();i;--i)if(parts[i-1].frame==frame&&(!exact||parts[i-1].type==5)) {
                    if(i-1>=0xffff)throw std::runtime_error("Car light part index overflow");
                    light->part1a=static_cast<std::uint16_t>(i-1);return true;
                }
                return false;
            };
            if(match(light->frame,true))continue;
            auto parent=frames.object_parent(light->frame);std::size_t depth=0;
            while(parent!=model) {
                if(!parent||++depth>1024)throw std::runtime_error("Car light is outside its model hierarchy");
                if(match(parent,false))break;
                parent=frames.object_parent(parent);
            }
        }
    }
    //Independent engine owners; lights_ holds only indices. The containing
    //Car geometry retains its model until after this owner is destroyed.
    std::vector<Projector> projectors_;
    std::vector<Light> lights_;
};

} //namespace hd2::game
#endif
