#ifndef HD2_GAME_WORLD_VIEWS_H
#define HD2_GAME_WORLD_VIEWS_H

#include <functional>
#include <stdexcept>
#include <utility>
namespace hd2::game {

//00698500 borrows an actually constructed game/scene, independently of loading
//success. Camera24C is a separate late query on the SAME World's camera owner.
//Neither query publishes readiness or substitutes a missing listener/scene.
class WorldViews {
public:
    void bind(void* owner, void* scene) {
        if (!owner || !scene || owner_) throw std::logic_error("Invalid game owner publication");
        owner_ = owner;scene_ = scene;
    }
    void unbind(void* owner) noexcept {
        if (owner_ != owner) return;
        camera_query_={};camera_borrower_=nullptr;cutscene_query_={};cutscene_borrower_=nullptr;scene_=nullptr;owner_=nullptr;
    }
    void* owner() const { return owner_; }
    void* scene() const { return scene_; }
    void bind_cutscene(const void* borrower,void* scene,std::function<bool()> query){
        if(!owner_||!borrower||scene!=scene_||!query||cutscene_borrower_)
            throw std::logic_error("Cutscene view requires its SAME live game/scene and unique Script owner");
        cutscene_query_=std::move(query);cutscene_borrower_=borrower;
    }
    void unbind_cutscene(const void* borrower)noexcept{
        if(cutscene_borrower_!=borrower)return;
        cutscene_query_={};cutscene_borrower_=nullptr;
    }
    bool cutscene_active()const{
        if(!owner_)return false;
        if(!cutscene_query_)throw std::logic_error("Application viewport requires its actual current Script owner");
        return cutscene_query_(); //Live same-World Script824; no copied process state.
    }
    void bind_sound_camera(const void* borrower,void* scene,std::function<void*()> query){
        if(!owner_||!borrower||scene!=scene_||!query||camera_borrower_)
            throw std::logic_error("Sound camera requires its SAME live game/scene and a unique actual camera owner");
        camera_query_=std::move(query);camera_borrower_=borrower;
    }
    void unbind_sound_camera(const void* borrower)noexcept{
        if(camera_borrower_!=borrower)return;
        camera_query_={};camera_borrower_=nullptr;
    }
    void* sound_camera_sector()const{
        if(!owner_||!camera_query_)throw std::logic_error("Sound environment requires its actual current game camera");
        return camera_query_(); //004650C0: actual native24C may legitimately be NULL.
    }
private:
    void* owner_=nullptr;void* scene_=nullptr;
    const void* camera_borrower_=nullptr;std::function<void*()> camera_query_;
    const void* cutscene_borrower_=nullptr;std::function<bool()> cutscene_query_;
};

//Retire the camera query BEFORE the containing World/camera dies, including a
//constructor throw. An explicit failed close keeps this loan for cleanup retry.
class WorldSoundCameraLoan {
public:
    WorldSoundCameraLoan()=default;
    ~WorldSoundCameraLoan(){reset();}
    WorldSoundCameraLoan(const WorldSoundCameraLoan&)=delete;
    WorldSoundCameraLoan& operator=(const WorldSoundCameraLoan&)=delete;
    void bind(WorldViews& views,void* scene,std::function<void*()> query){
        if(views_)throw std::logic_error("Sound camera loan already bound");
        views.bind_sound_camera(this,scene,std::move(query));views_=&views;
    }
    void reset()noexcept{if(views_)views_->unbind_sound_camera(this);views_=nullptr;}
private:
    WorldViews* views_=nullptr;
};
//The outer frame loop queries cutscene presentation before Scene rendering.
//Retire its Script borrower before the containing World, including failed construction.
class WorldCutsceneLoan {
public:
    ~WorldCutsceneLoan(){reset();}
    WorldCutsceneLoan()=default;
    WorldCutsceneLoan(const WorldCutsceneLoan&)=delete;
    WorldCutsceneLoan& operator=(const WorldCutsceneLoan&)=delete;
    void bind(WorldViews& views,void* scene,std::function<bool()> query){
        if(views_)throw std::logic_error("Cutscene presentation loan already bound");
        views.bind_cutscene(this,scene,std::move(query));views_=&views;
    }
    void reset()noexcept{if(views_)views_->unbind_cutscene(this);views_=nullptr;}
private:
    WorldViews* views_=nullptr;
};
} //namespace hd2::game
#endif
