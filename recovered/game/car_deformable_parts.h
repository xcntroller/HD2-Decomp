#ifndef HD2_GAME_CAR_DEFORMABLE_PARTS_H
#define HD2_GAME_CAR_DEFORMABLE_PARTS_H

#include "game/car_physics.h"
#include "game/car_deform_mesh.h"
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
        std::uint16_t flags=0; //Original WORD+38, including detached bit1.
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
    struct MeshImpact {bool first=false,second=false;float absorbed=0;explicit operator bool()const{return first||second;}};
    //004FF8B0's original mesh capture is deferred until the FIRST hit vertex.
    //Native654/658 is this physical owner's vector, initialized by004E2900.
    template<class Random>MeshImpact deform_mesh(Frames& frames,const std::string& resource,std::size_t index,
            const CarPartPhysics& applied,const CarContactVector& point,const CarContactVector& direction,
            float radius,float impulse,Random& random){
        auto& part=parts_.at(index);MeshImpact result;
        result.absorbed=static_cast<float>(static_cast<long double>(impulse)-
            (static_cast<long double>(applied.scalars.get(0x28))*.009999999776482582f)*impulse);
        float ratio=result.absorbed/applied.scalars.get(0x14);if(ratio>1.f)ratio=1.f;
        auto backup=find_backup(resource,part.name);const bool fresh=!part.variants&&backup==backups_.size();
        auto source=!part.variants&&!fresh?backups_[backup].mesh.get():part.mesh;
        CarContactVector local{},force{},limits{};
        for(unsigned axis=0;axis<3;++axis)limits[axis]=applied.scalars.get(0x2c+axis*4);
        const float square=static_cast<float>(static_cast<long double>(radius)*radius);
        const auto affect=[&](unsigned lod,bool& changed){
            const auto original=frames.car_deform_lod(source,lod),current=frames.car_deform_lod(part.mesh,lod);
            if(lod&&(!original||!current))return; //Native LOD1 pair is optional.
            if(!original||!current)throw std::runtime_error("Car deformation requires actual LOD0");
            visit_car_deform_vertices(frames,original,current,[&](std::size_t count,auto read_original,auto read_current,auto write){
                if(!lod){ //Native matrix/ray conversion follows BOTH locks.
                    auto inverse=frames.car_inverse_matrix(frames.actor_world_matrix(part.frame));
                    local=frames.car_transform(point,inverse);
                    inverse[12]=inverse[13]=inverse[14]=0;force=frames.car_transform(direction,inverse);
                    for(unsigned axis=0;axis<3;++axis)force[axis]=static_cast<float>(clamp_car_deformation(force[axis],limits[axis]));
                }
                for(auto n=count;n;){const auto vertex=--n;const auto current_value=read_current(vertex);
                    const auto x=static_cast<long double>(current_value[0])-local[0],y=static_cast<long double>(current_value[1])-local[1],z=static_cast<long double>(current_value[2])-local[2];
                    if(!((x*x+z*z)+y*y<square))continue;
                    if(!changed&&!part.variants&&(!lod||!result.first)){
                        if(fresh){
                            //Strings precede factory/copy; the record is published
                            //before flag4 and before any changed vertex.
                            std::string name=part.name,key=resource;
                            auto mesh=frames.copy_car_deform_backup(part.mesh);
                            backups_.push_back({std::move(mesh),1,std::move(name),std::move(key)});
                            part.flags|=4u;
                        }else if(!(part.flags&4u)){part.flags|=4u;++backups_[backup].uses;}
                    }
                    changed=true;
                    if(!part.variants){const auto draw=random();const auto saved=read_original(vertex);
                        write(vertex,deform_car_vertex(saved,current_value,force,limits,ratio,draw));}
                }
            });
        };
        affect(0,result.first);
        //A fresh hit deliberately retains its current source for LOD1, even
        //when LOD0 just published a backup. Repeated hits use the saved mesh.
        if(frames.car_deform_lod_count(source)>1)affect(1,result.second);
        return result;
    }
    void finish_mesh_impact(Frames& frames,std::size_t index,const MeshImpact& result){
        const auto& part=parts_.at(index);if(part.variants)return;
        if(result.first)frames.car_rebuild_deform_lod(frames.car_deform_lod(part.mesh,0));
        if(result.second)frames.car_rebuild_deform_lod(frames.car_deform_lod(part.mesh,1));
        frames.car_refresh_deform_mesh(part.mesh);frames.car_invalidate_deform_frame(part.frame,0);frames.update_scene_object(part.frame);
    }
    void reset_loading_health(const CarPhysics& physics,const std::function<void()>& callback) { //004FE910(1).
        const auto& applied=physics.geometry().parts;
        if(applied.size()!=parts_.size())throw std::logic_error("Car part health requires the SAME applied geometry");
        for(auto n=parts_.size();n;) {
            const auto index=--n;auto& part=parts_[index];
            if(part.type!=applied[index].type)throw std::logic_error("Car part health ordering differs from applied geometry");
            part.health=applied[index].scalars.get(0x1c);part.flags&=0xfffeu;
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
    //004FE910(0), same parts and retained glass texture exchange as00500DA0.
    //The containing geometry supplies actual seat-associated Door repair.
    void restore_cold(Frames& frames,const CarPhysics& physics,const std::function<void()>& callback,const std::string& resource={},
            const std::function<void(std::size_t,Frame)>& repair={}) {
        const auto& applied=physics.geometry().parts;
        if(applied.size()!=parts_.size())throw std::logic_error("Car part restore requires the SAME applied geometry");
        for(std::size_t n=0;n<parts_.size();++n){
            if(parts_[n].type!=applied[n].type)throw std::logic_error("Car part restore ordering differs from applied geometry");
        }
        for(auto n=parts_.size();n;){const auto index=--n;auto& part=parts_[index];
            restore_mesh(frames,index,resource);
            if(part.type==3&&(part.flags&8u)){
                if(frames.car_exchange_glass_texture(part.mesh,part.glass_texture))frames.update_scene_object(part.frame);
                part.flags&=0xfff7u; //Native clears8 even when either texture was NULL.
            }
            if(repair)repair(index,part.frame); //FA8+28 Door20, before variants/visibility.
            for(auto alternative=part.alternatives.rbegin();alternative!=part.alternatives.rend();++alternative){
                if(!*alternative)throw std::runtime_error("Car deformable variant sequence has a missing frame");
                frames.object_visible(*alternative,false);frames.update_scene_object(*alternative);
            }
            if(!part.frame)throw std::runtime_error("Car deformable base frame is unavailable");
            frames.object_visible(part.frame,true);frames.update_scene_object(part.frame);
            part.health=applied[index].scalars.get(0x1c);part.flags&=0xfffeu;
        }
        if(callback)callback();
    }
    std::vector<CarPartPhysics> physics_parts() const {
        std::vector<CarPartPhysics> result;
        for(const auto& part:parts_)result.push_back({part.type,{}});
        return result;
    }
private:
    struct Backup {TextureOwner mesh;std::uint32_t uses;std::string name,resource;};
    std::size_t find_backup(const std::string& resource,const std::string& name)const{
        for(auto n=backups_.size();n;)if(backups_[--n].resource==resource&&backups_[n].name==name)return n;
        return backups_.size();
    }
    void restore_mesh(Frames& frames,std::size_t index,const std::string& resource){
        auto& part=parts_[index];if(part.variants||!(part.flags&4u))return;
        //Native requires BOTH case-sensitive keys before restoration. A
        //nonmatching record is a genuine skip; flag4 remains set in that case.
        const auto backup=find_backup(resource,part.name);
        if(backup==backups_.size())return;
        auto& record=backups_[backup];const auto original=record.mesh.get();
        const auto restore=[&](unsigned lod){const auto source=frames.car_deform_lod(original,lod),destination=frames.car_deform_lod(part.mesh,lod);
            if(lod&&(!source||!destination))return;
            if(!source||!destination)throw std::runtime_error("Car mesh restore requires actual LOD0");
            visit_car_deform_vertices(frames,source,destination,[&](std::size_t count,auto read,auto,auto write){for(auto n=count;n;) {--n;write(n,read(n));}});
        };
        restore(0);const bool second=frames.car_deform_lod_count(original)>1;if(second)restore(1);
        frames.car_rebuild_deform_lod(frames.car_deform_lod(part.mesh,0));
        if(second)frames.car_rebuild_deform_lod(frames.car_deform_lod(part.mesh,1));
        frames.car_refresh_deform_mesh(part.mesh);frames.car_invalidate_deform_frame(part.frame,0);
        if(second)frames.car_invalidate_deform_frame(part.frame,1);
        frames.update_scene_object(part.frame);part.flags&=0xfffbu;
        if(!--record.uses){record.mesh.reset();backups_.erase(backups_.begin()+backup);}
    }
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
    std::vector<Backup> backups_;
};

//Both004EC5C0 retirement and004E9DB0 reset call004FE910 on these SAME
//geometry/resource/Door owners. Keep their containing bindings identical.
template<class Frames,class Geometry>void restore_car_deformable_parts(Frames& frames,Geometry& geometry,
        const std::function<void()>& callback){
    geometry.deformables().restore_cold(frames,geometry.physics(),callback,geometry.resource(),
        [&geometry](std::size_t index,typename Frames::Frame frame){geometry.auxiliary().restore_part_frame(index,frame);});
}

} //namespace hd2::game
#endif
