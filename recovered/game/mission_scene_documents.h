#ifndef HD2_GAME_MISSION_SCENE_DOCUMENTS_H
#define HD2_GAME_MISSION_SCENE_DOCUMENTS_H

#include "game/mission_scene_reader.h"
#include "io/resource_observations.h"
#include <functional>

namespace hd2::game {

//006CBF30/006D45C0/006D4880/006D4280 file-stage composition. Scene and
//SceneData must refer to the SAME actual scene; SceneData's table reader uses
//the containing session's existing two datasets. Files is the live resolver.
//The required Car prelude is004D6900 then004D57A0, even on a missing Scene2.
//No actor factory, fallback navigation data or loaded-world flag is supplied.
template<class Files,class Scene,class SceneData>class MissionSceneDocuments {
public:
    struct Scratch {typename Scene::Names names;}; //Borrowed frames; discard before scene reset.
    MissionSceneDocuments(Files& files,Scene& scene,SceneData& data,std::function<void()> prepare_cars)
        :files_(files),scene_(scene),data_(data),prepare_cars_(std::move(prepare_cars)) {
        if(!prepare_cars_)throw std::logic_error("Scene2 requires its actual shared Car preparation receiver");
    }
    MissionSceneFields& fields(){return fields_;}const MissionSceneFields& fields()const{return fields_;}
    std::uint32_t load_scene2(const std::string& resource,Scratch& scratch) {
        prepare_cars_();
        std::uint32_t status=3;
        data_.read_scene_document(path(resource,"scene2.bin"),[&](SerializedBytes bytes) {
            scene_.register_existing_frames(scratch.names);scene_.prepare_mission_camera();
            if(bytes.remaining()<6){status=1;return;} //Failed initial native six-byte header read.
            auto header=bytes;
            if(header.half()==0x4c53)data_.read_mission_scene_document(bytes,scratch.names,fields_,1);
            status=0; //Original also accepts a complete, different root without parsing it.
        });
        io::observe_bound_data(data_,"loading-outcome","scene2.bin","original status",status);
        return status;
    }
    void load_actors(const std::string& resource,Scratch& scratch){load_optional(resource,"actors.bin",scratch,2);}
    void load_sounds(const std::string& resource,Scratch& scratch){load_optional(resource,"sounds.bin",scratch,3);}
    void load_check_tables(const std::string& resource) {
        if(data_.read_scene_document(path(resource,"check2.bin"),[&](SerializedBytes cursor){data_.read_scene_tables(cursor);}))
            {io::observe_bound_data(data_,"loading-outcome","check2.bin","parsed both datasets");return;} //00523C00 consumes two datasets; trailing file bytes are not parsed.
        io::observe_bound_data(data_,"original-fallback","check2.bin absent","probe check.bin without legacy decode");
        data_.clear_scene_tables(); //00523E10 before trying the older filename.
        Open check(files_,path(resource,"check.bin"));
        //006D4280's legacy-present branch only closes this handle. There is no
        //legacy decoder call or successful substitute dataset on this path.
        io::observe_bound_data(data_,"loading-outcome","check.bin",check?"legacy present: close only":"legacy absent: clear tables");
        if(!check)data_.clear_scene_tables();
    }
private:
    class Open {
    public:
        Open(Files& files,const std::string& path):files_(files),handle_(files.open(path.c_str(),0)){}
        ~Open(){if(*this)files_.close(handle_);}
        Open(const Open&)=delete;Open& operator=(const Open&)=delete;
        explicit operator bool()const{return handle_!=0xffffffffu;}
    private:
        Files& files_;std::uint32_t handle_;
    };
    static std::string path(const std::string& resource,const char* file){return "missions\\"+resource+"\\"+file;}
    void load_optional(const std::string& resource,const char* name,Scratch& scratch,unsigned stage) {
        const auto present=data_.read_scene_document(path(resource,name),[&](SerializedBytes bytes) {
            //The host retains this SAME document's handle and byte view for
            //embedded light-map reads; do not reopen or copy the parser input.
            if(bytes.remaining()<6)throw std::runtime_error("Truncated optional mission document header");
            auto header=bytes;
            if(header.half()==0x4c53)data_.read_mission_scene_document(bytes,scratch.names,fields_,stage);
        });
        io::observe_bound_data(data_,"loading-outcome",name,present?"present: original parser completed":"optional absent");
    }
    Files& files_;Scene& scene_;SceneData& data_;std::function<void()> prepare_cars_;MissionSceneFields fields_;
};
} //namespace hd2::game
#endif
