#ifndef HD2_GAME_SELECTED_RESOURCE_ANALYSIS_H
#define HD2_GAME_SELECTED_RESOURCE_ANALYSIS_H
#include "game/actor_properties.h"
#include "game/car_parameter_block.h"
#include "game/animation_tables.h"
#include "io/resource_observations.h"
#include <set>

namespace hd2::game {
struct SelectedRequirement {std::string family,name;std::uint64_t value=0;};
//Static decoding only. No Scene backend, fake actors, successful animation
//resolver or runtime registration. Exact parser scope is emitted alongside data.
class SelectedSceneRequirements {
public:
    const auto& records()const{return records_;}
    void read(const std::vector<unsigned char>& bytes){
        records_.clear();
        SerializedBytes input(bytes);auto root=next_mission_chunk(input);
        if(!root||root->id!=0x4c53)throw std::runtime_error("Static scene analysis requires 4C53 root");
        while(auto chunk=next_mission_chunk(root->bytes)){
            if(chunk->id==1){const auto version=chunk->bytes.half();add("scene-version","006CC760",version);if(version!=7)add("unsupported-required","scene version",version);}
            else if(chunk->id==0xae20)actors(chunk->bytes);
            else if(chunk->id==0x4000){while(auto object=next_mission_chunk(chunk->bytes))if(object->id==0x4010)objects(object->bytes);}
            else if(chunk->id==0xae00&&chunk->bytes.remaining())add("unsupported-required","AE00 lightmap references",chunk->bytes.remaining());
            else if(chunk->id==0xae40)add("navigation-dependency","embedded mission tables",chunk->bytes.remaining());
        }
    }
private:
    void add(const char* family,const std::string& name,std::uint64_t value=0){records_.push_back({family,name,value});}
    template<class P>void property(SerializedBytes& bytes){P value;value.load(bytes);}
    void properties(std::uint32_t type,SerializedBytes bytes){
        //Same property decoders as real SceneActor::load. Callbacks here report
        //STATIC side-effect requirements and never establish receiver execution.
        switch(type){
        case 3:{HumanActorProperties p;p.load(bytes);add("authored-human-state","field19c",p.field19c);add("authored-human-reference","field170",p.field170);break;}
        case 4:{const auto decoded=read_car_parameter_block(bytes);add("decoded-car-parameter-bytes","004D55C0",decoded.size());break;}
        case 6:{DoorActorProperties p;p.load(bytes,[&]{add("required-receiver","00600A50 door portal refresh");});if(!p.name120.empty())add("door-reference",p.name120);break;}
        case 8:property<LadderActorProperties>(bytes);break;
        case 9:{SupplyBoxActorProperties p;p.load(bytes);for(const auto& list:p.items)for(auto item:list)add("equipment-definition","SupplyBox",item);break;}
        case 10:property<SpawnPointActorProperties>(bytes);break;
        case 13:{SwitchActorProperties p;p.load(bytes,[&](std::uint32_t state){add("required-receiver","006136F0 switch animation",state);});break;}
        case 21:case 33:break; //Actual StaticWeapon/Null property RET bodies.
        case 22:property<ExplosivePlaceActorProperties>(bytes);break;
        case 25:{AnimationActorProperties p;p.load(bytes);if(!p.name0.empty())add("animation-reference",p.name0);if(!p.name5.empty())add("animation-reference",p.name5);break;}
        case 28:property<PikeActorProperties>(bytes);break;
        case 29:{FowlGeneratorActorProperties p;p.load(bytes);if(!p.name.empty())add("fowl-reference",p.name);break;}
        case 30:property<DogActorProperties>(bytes);break;
        default:add("unsupported-required","actor properties",type);return;
        }
        add("property-decoded","006CC760 AE24",type);
        //Several original property readers intentionally leave trailing bytes.
        if(bytes.remaining())add("unconsumed-property-bytes","not proof of malformed data",bytes.remaining());
    }
    void actors(SerializedBytes bytes){
        while(auto actor=next_mission_chunk(bytes))if(actor->id==0xae21){
            std::uint32_t type=0;bool created=false;std::string name;
            while(auto field=next_mission_chunk(actor->bytes)){
                if(field->id==0xae23)name=field->bytes.text();
                else if(field->id==0xae22){type=field->bytes.word();created=true;add("actor-family",name,type);
                    switch(type){case 3:case 4:case 6:case 8:case 9:case 10:case 13:case 21:case 22:case 25:case 28:case 29:case 30:case 33:break;default:add("unsupported-required","actor family",type);}}
                else if(field->id==0xae24&&created&&field->bytes.remaining())properties(type,field->bytes);
            }
        }
    }
    void objects(SerializedBytes bytes){
        std::uint32_t kind=0;std::string name;
        while(auto field=next_mission_chunk(bytes)){
            if(field->id==0x4011)kind=field->bytes.word();
            else if(field->id==0x10){name=field->bytes.text();add("frame-declaration",name,kind);}
            else if(field->id==0x2012)add("model-reference",field->bytes.text());
            else if(field->id==0xb150)add("geometry-dependency",name,field->bytes.remaining());
            else if(field->id==0x4080||field->id==0x40f0||field->id==0xb401)
                add("conditional-unsupported-record",name,field->id); //Actual frame/visual type gates require engine owner.
            else if(field->id==0x4060){while(auto sound=next_mission_chunk(field->bytes))if(sound->id==0x10)add("sound-reference",sound->bytes.text());}
        }
    }
    std::vector<SelectedRequirement> records_;
};
}
#endif
