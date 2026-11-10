#ifndef HD2_GAME_CAR_DEFORMABLE_PARTS_H
#define HD2_GAME_CAR_DEFORMABLE_PARTS_H

#include "game/car_physics.h"
#include <string_view>
#include <cstdlib>
#include <functional>

namespace hd2::game {

//004FA520/004FAEA0's actual attribute-driven type and variant owners. Mesh
//cloning/material separation remain real engine operations on the backend.
//The original failed clone status2 must be surfaced, never reported as success.
template<class Frames>
class CarDeformableParts {
public:
    using Frame=typename Frames::Frame;
    using TextureOwner=decltype(std::declval<Frames&>().load_car_glass_texture(
        std::declval<const std::string&>(),std::declval<const std::string&>()));
    struct Part {
        Frame frame{},mesh{};
        std::string name;
        std::uint8_t type=0,group=0;
        bool variants=false;
        std::vector<Frame> alternatives;
        std::uint8_t flags=0; //Original+38, including detached bit1.
        float health=0; //24, produced by004FE910 from the SAME applied part+1C.
        std::optional<TextureOwner> glass_texture{}; //0C, released with the actual part owner.
    };
    void discover(Frames& frames,Frame model) {
        if(!parts_.empty())throw std::logic_error("Car deformable discovery repeated");
        for(auto frame:frames.car_parts(model,"*",0x10001)) read(frames,frame);
    }
    void initialize_visibility(Frames& frames) { //004FE6B0.
        auto last=parts_.size();
        for(auto index=parts_.size();index;--index)if(parts_[index-1].type==2) {
            if(index<last)std::swap(parts_[index-1],parts_[last-1]);
            --last;
        }
        for(auto part=parts_.rbegin();part!=parts_.rend();++part) {
            for(auto alternative=part->alternatives.rbegin();alternative!=part->alternatives.rend();++alternative) {
                if(!*alternative)throw std::runtime_error("Car deformable variant sequence has a missing frame");
                frames.object_visible(*alternative,false);frames.update_scene_object(*alternative);
            }
            if(!part->frame)throw std::runtime_error("Car deformable base frame is unavailable");
            frames.object_visible(part->frame,true);frames.update_scene_object(part->frame);
        }
    }
    const std::vector<Part>& parts() const {return parts_;}
    std::vector<Part>& parts() {return parts_;}
    void reset_loading_health(const CarPhysics& physics,const std::function<void()>& callback) { //004FE910(1).
        const auto& applied=physics.geometry().parts;
        if(applied.size()!=parts_.size())throw std::logic_error("Car part health requires the SAME applied geometry");
        for(auto n=parts_.size();n;) {
            const auto index=--n;auto& part=parts_[index];
            if(part.type!=applied[index].type)throw std::logic_error("Car part health ordering differs from applied geometry");
            part.health=applied[index].scalars.get(0x1c);part.flags&=0xfeu;
        }
        //The callback runs once, including for an empty parts list. The real
        //cold actor installs004D4A00, a genuine empty body, in slot6B4.
        if(callback)callback();
    }
    void load_glass_textures(Frames& frames,const CarParameters& parameters) { //004FE850.
        //004ED270 copies the parameter block to nativeFDC;175C/176C are
        //therefore names1E0/1E4. Both inline pointers exist even for empty text.
        const auto primary=parameters.name(0x1e0),secondary=parameters.name(0x1e4);
        if(primary.empty())return;
        for(auto& part:parts_)if(part.type==3&&!part.glass_texture) {
            auto texture=frames.load_car_glass_texture(primary,secondary);
            if(texture)part.glass_texture.emplace(std::move(texture));
            //A rejected graphics status leaves this part empty and continues.
        }
    }
    //004FE910(0), for the actual cold unoccupied lifetime. Mesh backup and
    //glass swaps are produced only by later positive deformation. No seat
    //attachment/nativeFA8+28 owner may be installed through this entry.
    void restore_cold(Frames& frames,const CarPhysics& physics,const std::function<void()>& callback) {
        const auto& applied=physics.geometry().parts;
        if(applied.size()!=parts_.size())throw std::logic_error("Car part restore requires the SAME applied geometry");
        for(std::size_t n=0;n<parts_.size();++n){
            if(parts_[n].type!=applied[n].type)throw std::logic_error("Car part restore ordering differs from applied geometry");
            if(parts_[n].flags&0xcu)throw std::runtime_error("Car part restore requires its actual deformed mesh/glass backup owner");
        }
        for(auto n=parts_.size();n;){const auto index=--n;auto& part=parts_[index];
            for(auto alternative=part.alternatives.rbegin();alternative!=part.alternatives.rend();++alternative){
                if(!*alternative)throw std::runtime_error("Car deformable variant sequence has a missing frame");
                frames.object_visible(*alternative,false);frames.update_scene_object(*alternative);
            }
            if(!part.frame)throw std::runtime_error("Car deformable base frame is unavailable");
            frames.object_visible(part.frame,true);frames.update_scene_object(part.frame);
            part.health=applied[index].scalars.get(0x1c);part.flags&=0xfeu;
        }
        if(callback)callback();
    }
    std::vector<CarPartPhysics> physics_parts() const {
        std::vector<CarPartPhysics> result;
        for(const auto& part:parts_)result.push_back({part.type,{}});
        return result;
    }
private:
    struct Tag {const char* name;std::uint8_t type;};
    inline static constexpr std::array<Tag,10> tags{{
        {"DEFORM BODY",1},{"DEFORM BUMPER",2},{"DEFORM LIGHT",5},{"DEFORM SPZ",7},
        {"DEFORM MIRROR",4},{"DEFORM WING",6},{"DEFORM DOOR",8},{"DEFORM ROOF",9},
        {"DEFORM STEPNEY",10},{"DEFORM GLASS",3}
    }};
    static std::string short_name(const std::string& name) {
        const auto dot=name.rfind('.');
        //004FA520 scans down to index0, then advances even when no dot exists.
        return name.substr(dot==std::string::npos?std::min<std::size_t>(1,name.size()):dot+1);
    }
    void read(Frames& frames,Frame frame) {
        const auto user=frames.car_user_data(frame);
        if(user.size()<=11)return;
        const auto begin=user.find('['),end=begin==std::string::npos?std::string::npos:user.find(']',begin+1);
        if(end==std::string::npos)return;
        const std::string_view tag(user.data()+begin+1,end-begin-1);
        if(tag.size()<10)return;
        for(const auto& known:tags) {
            const std::string_view text=known.name;
            if(tag.size()>=text.size()&&tag.substr(0,text.size())==text) {
                parts_.push_back({frame,frames.prepare_car_deform_mesh(frame,known.type==3),
                                  short_name(frames.frame_name(frame)),known.type,0,false,{}});
                return;
            }
        }
        //The alternate variant syntax only exists for these eight families.
        if(tag.empty()||tag.front()!='+')return;
        for(const auto& known:tags) {
            if(known.type==3||known.type==10)continue;
            const std::string_view text=known.name;
            if(tag.size()<=text.size()+2||tag.substr(1,text.size())!=text)continue;
            //004FAEA0 uses atoi(user + '['-start +12), irrespective of the
            //matched family's text length. Preserve this original offset.
            const auto number_offset=begin+1+12;
            if(number_offset>=user.size())return;
            char* last=nullptr;const auto number=std::strtol(user.c_str()+number_offset,&last,10);
            const auto value=last==user.c_str()+number_offset?0L:number;
            if(value<0||value>2559)throw std::runtime_error("Invalid Car deformable variant number");
            const auto group=static_cast<std::uint8_t>(value/10);
            const auto alternative=static_cast<std::size_t>(value%10);
            auto found=std::find_if(parts_.rbegin(),parts_.rend(),[&](const Part& part) {
                return part.type==known.type&&part.group==group&&part.variants;
            });
            if(found!=parts_.rend()) {
                if(!alternative)found->frame=frame;
                else {
                    if(found->alternatives.size()<alternative)found->alternatives.resize(alternative);
                    found->alternatives[alternative-1]=frame;
                }
                return;
            }
            Part part;
            part.type=known.type;part.group=group;part.variants=true;
            part.name=short_name(frames.frame_name(frame));part.mesh=frames.car_mesh(frame);
            //004FAEA0's local frame starts as the enumerated frame even when
            //the first encountered member is an alternative (not variant0).
            part.frame=frame;
            if(alternative) {part.alternatives.resize(alternative);part.alternatives.back()=frame;}
            parts_.push_back(std::move(part));return;
        }
    }
    std::vector<Part> parts_;
};

} //namespace hd2::game
#endif
