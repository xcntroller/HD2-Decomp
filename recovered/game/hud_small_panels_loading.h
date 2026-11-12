#ifndef HD2_GAME_HUD_SMALL_PANELS_LOADING_H
#define HD2_GAME_HUD_SMALL_PANELS_LOADING_H
#include "game/hud_text_loading.h"
#include <memory>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <utility>
namespace hd2::game {
//Required small0049B280 panel loading/lifetimes. Factory supplies SAME actual
//font/text/localization owners. Native TextState constructor is already recovered;
//draw consumers share the original text owner; populated updates remain separate.
template<class Frames,class Factory>
class HudSmallPanelsLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;
    struct TextPanel {Frame visual{};std::unique_ptr<Text> text;};
    struct Message {std::unique_ptr<Text> text;bool enabled=false;std::uint32_t time=0;};
    struct Status {Frame scene{};std::array<Frame,8> frames{};std::unique_ptr<Text> text;
        std::array<std::uint32_t,4> words28{{0,0,0xffffffffu,0}};};
    HudSmallPanelsLoading(Frames& frames,Factory& factory):frames_(frames),factory_(factory) {}
    void bind_hint(Frame scene) { //004AD4E0.
        bind_text(hint_,scene,"Htext",false);set_hint(false,0xffffffffu);
    }
    void set_hint(bool enabled,std::uint32_t id) { //004AD880, clear even when hiding.
        auto& text=required(hint_.text);text.text.clear();
        if(enabled)text.text.append(factory_.hud_localized(id),0x3f);
        show(hint_.visual,enabled);
    }
    void bind_voting(Frame scene) { //004AF2E0; absent visual is tolerated by native binder.
        bind_text(voting_,scene,"voting",true);
        if(voting_.visual)show(voting_.visual,false);
    }
    void bind_carnage(Frame scene) { //004AFC00.
        bind_text(carnage_,scene,"carnage_stats",false);carnage_.text->text.inset=0;
        carnage_.text->text.append(std::make_shared<const std::string>(),0x49);
    }
    void bind_server_message(Frame scene) { //004AFE50, text borrows injury geometry without retaining frame.
        bind_message(server_,scene);server_.text->text.append(factory_.hud_localized(0xa46),0x4a);
        server_.enabled=false;server_.time=0;
    }
    void bind_injury_message(Frame scene) { //004B0000 localizes then004B01A0 clears; do not skip localization.
        bind_message(injury_,scene);injury_.text->text.append(factory_.hud_localized(0xa46),0x4a);
        set_injury_message(false,0xffffffffu,0xffffffffu);injury_.time=0;
    }
    void set_injury_message(bool enabled,std::uint32_t id,std::uint32_t time) { //004B01A0.
        auto& text=required(injury_.text);text.text.clear();
        if(!enabled){injury_.time=0;injury_.enabled=false;return;}
        injury_.time=time;text.text.append(factory_.hud_localized(id),0x4a);injury_.enabled=true;
    }
    void bind_status(Frame scene) { //004AAAC0.
        cold(status_.text);require_scene(scene);status_.scene=scene;
        constexpr std::array<const char*,8> names{{"tabl01","pkneel","play","mwalk","mrun","msprint","mpaxis","mpallied"}};
        for(unsigned n=0;n!=names.size();++n)status_.frames[n]=frames_.hud_find(scene,names[n],0xffff);
        for(auto n:{7u,6u,1u,2u,3u,4u,5u})show(status_.frames[n],false);
        status_.words28={{0,0,0xffffffffu,0}};status_.text=make_text();status_.text->text.inset=0;
        bind_hud_text_geometry(frames_,*status_.text,scene,status_.frames[0]);
    }
    void bind_camera(Frame scene) { //004AF4A0→004AF600(0).
        require_scene(scene);camera_=frames_.hud_find(scene,"camera",0xffff);show_camera(false);camera_time_=0;camera_flag_=false;
    }
    void refresh_status_geometry(){bind_hud_text_geometry(frames_,required(status_.text),status_.scene,status_.frames[0]);} //004AAD20.
    void show_camera(bool enabled) { //004AF600 updates timer/flag even when artwork absent.
        if(camera_)show(camera_,enabled);
        if(enabled){camera_time_=2000;camera_flag_=true;}
    }
    template<class Presentation>void update_camera(std::uint32_t elapsed,Presentation& presentation){ //004AF4F0.
        if(!camera_time_)return;
        camera_time_-=elapsed; //Signed comparisons after wrapping DWORD subtraction.
        if(signed_word(camera_time_)<1000&&camera_flag_){
            presentation.snapshot_sound();camera_flag_=false; //Flag clears AFTER actual SOUND receiver returns.
        }
        if(signed_word(camera_time_)<950&&signed_word(camera_time_)>800)presentation.snapshot_flash();
        if(signed_word(camera_time_)<=0){camera_time_=0;show_camera(false);}
    }
    template<class Players,class Team>void update_status(std::uint32_t elapsed,Players& players,std::uint32_t mode,std::int32_t style,Team&& team){ //004AAD30.
        auto& words=status_.words28;
        if(signed_word(words[3])>0){
            auto& text=required(status_.text);text.text.clear();words[3]-=elapsed;
            if(signed_word(words[3])>0){
                const auto seconds=signed_word(words[3])/1000;std::array<char,32> caption{};
                std::snprintf(caption.data(),caption.size(),"%02d:%02d",seconds/60,seconds%60);
                text.text.append(std::make_shared<const std::string>(caption.data()),0x3e);
            }
        }
        auto* human=players.current_player();if(!human)return;
        auto& state=players.state(*human); //Same lifetime-checked Human B0.
        const auto posture=state.words().template get<std::uint32_t>(0x50)&7u; //005A5D10.
        if(posture!=words[0]){
            if(posture&2u)show(status_.frames[1],true);else if(posture&1u)show(status_.frames[2],true);
            if(words[0]&2u)show(status_.frames[1],false);else if(words[0]&1u)show(status_.frames[2],false);
            words[0]=posture;
        }
        const auto movement=players.state(*human).words().template get<std::uint32_t>(0x5c); //005A5C80, fresh after visibility calls.
        if(movement!=words[1]){
            show(status_.frames[3],false);show(status_.frames[4],false);show(status_.frames[5],false);
            const auto flags=movement&0x1f000u;
            if(flags==0x10000u)show(status_.frames[5],true);
            if(flags==0x10000u||flags==0x4000u)show(status_.frames[4],true);
            if(flags==0x10000u||flags==0x4000u||flags==0x2000u)show(status_.frames[3],true);
            words[1]=movement;
        }
        if((mode&7u)!=7u&&style!=0){const auto faction=team(*human);if(static_cast<std::uint32_t>(faction)!=words[2]){
            show(status_.frames[7],faction==1);show(status_.frames[6],faction!=1);words[2]=static_cast<std::uint32_t>(faction);
        }} //Original7=mpallied then6=mpaxis; spectator/unknown uses axis, not a fabricated neutral icon.
    }
    void update_injury_message(std::uint32_t elapsed) { //004B01E0, signed wrapped countdown, -1 is indefinite.
        if(injury_.time==0xffffffffu||injury_.time==0)return;
        injury_.time-=elapsed;
        if(signed_word(injury_.time)<1){injury_.time=0;injury_.enabled=false;}
    }
    void server_message_visible(bool enabled){server_.enabled=enabled;} //0049BD50 native6B6BC publication, same message drawn by004AFFF0.
    void update_carnage(std::uint32_t count) { //004AFDB0, caller supplies SAME Scene2 summary50.
        if(!carnage_.visual)throw std::logic_error("HUD carnage update before artwork binding");
        if(!frames_.hud_frame_visible(carnage_.visual))return;
        required(carnage_.text).text.set(std::make_shared<const std::string>(std::to_string(count)),0x49,0);
    }
    void update_voting()noexcept {} //004AF470 is a genuine RET, not a missing positive receiver.
    //004AD640/004AF450/004AFDA0/004AFFC0/004B0180/004AAD10 release text
    //owners. Nulling stale native fields is a defined ownership guard.
    void close_hint()noexcept{hint_.text.reset();}
    void close_voting()noexcept{voting_.text.reset();}
    void close_carnage()noexcept{carnage_.text.reset();}
    void close_server_message()noexcept{server_.text.reset();}
    void close_injury_message()noexcept{injury_.text.reset();}
    void close_status()noexcept{status_.text.reset();}
    void close_camera()noexcept{} //004AF4E0 is an actual RET; artwork remains borrowed.
    void draw_hint(){draw_visible(hint_,false);} //004AD8D0 requires root; checks root visibility.
    void draw_voting(){draw_visible(voting_,true);} //004AF480 tolerates null root.
    void draw_carnage(){draw_visible(carnage_,false);} //004AFE30.
    void draw_status(){ //004AAD00 tests field34=words28[3], not root visibility.
        if(status_.words28[3])draw_required(status_.text);
    }
    void draw_server_message(){if(server_.enabled)draw_required(server_.text);} //004AFFF0.
    void draw_injury_message(){if(injury_.enabled)draw_required(injury_.text);} //004B0210.
    void discard_borrowers()noexcept { //Containing HUD must call before releasing its scene.
        hint_.visual=voting_.visual=carnage_.visual=camera_=Frame{};status_.scene=Frame{};status_.frames.fill(Frame{});
    }
    const auto& hint()const{return hint_;}const auto& voting()const{return voting_;}
    const auto& carnage()const{return carnage_;}const auto& server_message()const{return server_;}
    auto& status(){return status_;}
    const auto& injury_message()const{return injury_;}const auto& status()const{return status_;}
    Frame camera()const{return camera_;}std::uint32_t camera_time()const{return camera_time_;}bool camera_flag()const{return camera_flag_;}
private:
    static std::int32_t signed_word(std::uint32_t word){std::int32_t value;std::memcpy(&value,&word,4);return value;}
    static void require_scene(Frame scene){if(!scene)throw std::runtime_error("HUD panel requires actual artwork scene");}
    static void cold(const std::unique_ptr<Text>& owner){if(owner)throw std::logic_error("HUD panel rebound without actual teardown");}
    static Text& required(std::unique_ptr<Text>& owner){if(!owner)throw std::logic_error("HUD panel text is not initialized");return *owner;}
    void draw_required(std::unique_ptr<Text>& owner){factory_.hud_draw_text(required(owner));}
    void draw_visible(TextPanel& panel,bool optional){if(!panel.visual){if(optional)return;throw std::runtime_error("Required HUD draw artwork is missing");}
        if(frames_.hud_frame_visible(panel.visual))draw_required(panel.text);}
    std::unique_ptr<Text> make_text(){auto result=factory_.make_hud_text();if(!result)throw std::runtime_error("HUD panel text factory returned null");return result;}
    void show(Frame frame,bool enabled){if(!frame)throw std::runtime_error("Required HUD panel artwork is missing");frames_.object_visible(frame,enabled);}
    void bind_text(TextPanel& panel,Frame scene,const char* name,bool zero_inset) {
        cold(panel.text);require_scene(scene);panel.visual=frames_.hud_find(scene,name,0xffff);panel.text=make_text();
        if(zero_inset)panel.text->text.inset=0;
        bind_hud_text_geometry(frames_,*panel.text,scene,panel.visual);
    }
    void bind_message(Message& message,Frame scene) {
        cold(message.text);require_scene(scene);const auto visual=frames_.hud_find(scene,"injury",0xffff);
        message.text=make_text();bind_hud_text_geometry(frames_,*message.text,scene,visual);message.text->text.inset=0;
    }
    Frames& frames_;Factory& factory_;TextPanel hint_,voting_,carnage_;Message server_,injury_;Status status_;
    Frame camera_{};std::uint32_t camera_time_=0;bool camera_flag_=false;
};
} //namespace hd2::game
#endif
