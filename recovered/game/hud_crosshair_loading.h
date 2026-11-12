#ifndef HD2_GAME_HUD_CROSSHAIR_LOADING_H
#define HD2_GAME_HUD_CROSSHAIR_LOADING_H
#include "game/hud_text_loading.h"
#include <array>
#include <memory>
#include <stdexcept>
namespace hd2::game {
//004A9740/004A9F00/004A9F10/004AA760. Settings MUST resolve actual006B66D0
//current application24 manager274 or editor application20 profile73; no default
//enabled preference or fake manager is supplied by this source receiver.
template<class Frames,class Factory,class Settings>
class HudCrosshairLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;
    HudCrosshairLoading(Frames& frames,Factory& factory,Settings& settings):frames_(frames),factory_(factory),settings_(settings) {}
    ~HudCrosshairLoading(){close();}
    void bind(Frame scene) {
        if(text_)throw std::logic_error("HUD crosshair rebound before teardown");
        require(scene);target_=nullptr;flag34_=0;root_=frames_.hud_find(scene,"cross",0xffff);require(root_);
        frames_.hud_message_position(root_,.5f,-.375f,-.14000000059604644775390625f);frames_.update_scene_object(root_);
        constexpr std::array<const char*,4> names{{"cross_L","cross_R","cross_T","cross_B"}};
        for(unsigned n=0;n!=4;++n)parts_[n]=frames_.actor_find(root_,names[n],1);
        constexpr std::array<float,4> zero{{0,0,0,0}},one{{1,1,1,0}}; //Fourth native vector lanes unwritten; zero guards.
        for(auto part:parts_) {
            require(part);frames_.hud_group18(part);
            frames_.hud_material_flags(indexed_material(part),0x08000000);
            frames_.hud_material_vector(indexed_material(part),0x0c,zero);
            frames_.hud_material_vector(indexed_material(part),0x10,zero);
            frames_.hud_material_vector(indexed_material(part),0x08,one);
            frames_.hud_material14(indexed_material(part),0.f);
            frames_.hud_visual_a4(part,.60000002384185791015625f);
        }
        text_=factory_.make_hud_text();if(!text_)throw std::runtime_error("HUD crosshair text factory returned null");
        name_=frames_.actor_find(root_,"cross_name",1);
        if(name_){frames_.hud_crosshair_name_scale(name_);bind_hud_text_geometry(frames_,*text_,scene,name_);frames_.object_visible(name_,false);}
        else bind_hud_text_geometry(frames_,*text_,scene,root_);
        field14_=0;frames_.update_scene_object(root_);show(1);
        colour20_[0]=colour20_[1]=colour20_[2]=0;set_colour(one);
    }
    void show(std::uint8_t visible) { //Stores request before current preference query, no equality shortcut.
        visible35_=visible;
        if(visible&&!settings_.hud_crosshair_allowed())visible35_=0;
        require(root_);frames_.object_visible(root_,visible35_!=0);frames_.update_scene_object(root_);
    }
    void set_colour(const std::array<float,4>& value) { //Root E0 bit1 (visibility) gates; NaN differs and causes assignment.
        require(root_);if(!frames_.hud_frame_visible(root_))return;
        if(value[0]==colour20_[0]&&value[1]==colour20_[1]&&value[2]==colour20_[2])return;
        colour20_=value; //All FOUR words cached before material calls, even if one later fails.
        for(auto n:{2u,3u,1u,0u})frames_.hud_material_vector(frames_.hud_first_material(parts_[n]),0x0c,value);
        frames_.update_scene_object(root_);
    }
    template<class Query>void update_spread(std::uint32_t elapsed,Query&& query){ //004AA840.
        require(root_);if(!frames_.hud_frame_visible(root_))return;
        const auto spread=query();if(!spread)return; //Actual current-player absence.
        const auto target=*spread*.025f+.01f;if(target==field14_)return;
        std::int32_t delta;std::memcpy(&delta,&elapsed,4);
        const auto step=static_cast<long double>(delta)*.001f*.05f;
        const bool increasing=target>field14_;
        const auto value=increasing?static_cast<long double>(field14_)+step:static_cast<long double>(field14_)-step;
        field14_=static_cast<float>(value);
        if(increasing?value>target:value<target)field14_=static_cast<float>(target);
        publish_spread_parts();
    }
    template<class Actor,class Queries>void publish_target(Actor* actor,std::uint8_t allowed,std::uint32_t mode,Queries& queries){ //004A9F50.
        const void* identity=actor?queries.identity(*actor):nullptr;
        if(target_==identity&&flag34_==allowed)return;
        if(!text_)throw std::logic_error("Crosshair target publication requires SAME initialized text");
        text_->text.clear();target_=identity;flag34_=allowed;
        auto* current=queries.current_player();
        const std::array<float,4> white{{1,1,1,0}},blue{{0,0,1,0}};
        if(!actor){set_colour(white);return;}
        if(!current)throw std::logic_error("Crosshair target publication requires actual current Human");
        if(actor==queries.attachment(*current)||actor==queries.focus(*current)||queries.health(*current)<1){set_colour(white);return;}
        const auto caption=[&](auto& value){text_->text.append(std::make_shared<const std::string>(queries.caption(value)),0x3d);};
        const auto friendly=[&](auto& human){
            set_colour(blue);
            if(queries.in_roster(human)){
                const auto name=queries.full_name(human); //005A8320 BEFORE fresh00461A60 roster index.
                const auto bits=static_cast<std::uint32_t>(queries.roster_index(human))+1u;std::int32_t number;std::memcpy(&number,&bits,4);
                text_->text.append(std::make_shared<const std::string>(std::to_string(number)+". "+name),0x3d);
            }
            else caption(human);
        };
        switch(queries.kind(*actor)){
        case 2:case 6:case 7:case 8:case 9:case 11:case 13:case 15:case 22:case 23:case 35:
            if(allowed){set_colour(white);caption(*actor);}return;
        case 3:
            if((mode&7u)==7u){
                if(queries.hostile(*current,*actor)){set_colour(white);if(queries.health(*actor)<1)text_->text.append(factory_.hud_localized(0x1bf7),0x3d);return;}
                friendly(*actor);return;
            }
            if(queries.hostile(*current,*actor)){set_colour(white);if(queries.health(*actor)<=0)caption(*actor);return;}
            set_colour(blue);
            if(queries.human_kind(*actor)!=1){caption(*actor);return;}
            {
                const auto rank=queries.rank_text(*actor);
                if(queries.medic_skill(*current)>=50){const auto percent=queries.health_percent(*actor);const auto name=queries.full_name(*actor);
                    text_->text.append(std::make_shared<const std::string>(rank+" "+name+" - "+std::to_string(percent)+"%"),0x3d);
                }else text_->text.append(std::make_shared<const std::string>(rank+" "+queries.full_name(*actor)),0x3d);
            }return;
        case 4:
            for(auto n=queries.occupant_count(*actor);n;){--n;
                if(queries.occupant(*actor,n)){
                    auto* human=queries.occupant(*actor,n);if(!human)throw std::logic_error("Crosshair occupant vanished between original seat reads");
                    if(queries.health(*human)>0){if(queries.car_hostile(*actor,*current)){set_colour(white);return;}
                        friendly(*human);return;
                    }
                }
            }
            set_colour(white);caption(*actor);return;
        case 20:set_colour(white);caption(*actor);return;
        case 21:
            if(auto* human=queries.occupant(*actor,0)){
                if(queries.hostile(*current,*human)){set_colour(white);return;}
                friendly(*human);return;
            }
            set_colour(white);caption(*actor);return;
        case 30:set_colour(white);return;
        default:return;
        }
    }
    void close()noexcept{text_.reset();} //004A9F00, no preceding record clear.
    void draw(){ //004AAAB0 checks field30, independently of root visibility.
        if(!target_)return;
        if(!text_)throw std::logic_error("HUD crosshair draw before text initialization");
        factory_.hud_draw_text(*text_);
    }
    void discard_borrowers()noexcept{root_=name_=Frame{};parts_.fill(Frame{});target_=nullptr;}
    void retire_target(const void* identity){ //Source lifetime guard at actual Actor retirement, before its base/model dies.
        if(target_!=identity)return;
        target_=nullptr;flag34_=0;if(text_)text_->text.clear();
    }
    const void* target()const{return target_;}std::uint8_t target_allowed()const{return flag34_;}
    Frame root()const{return root_;}Frame name()const{return name_;}const auto& parts()const{return parts_;}const auto& text()const{return text_;}
    const auto& colour()const{return colour20_;}std::uint8_t visible()const{return visible35_;}std::uint32_t field14()const{std::uint32_t bits;std::memcpy(&bits,&field14_,4);return bits;}float spread_offset()const{return field14_;}
private:
    void publish_spread_parts(){
        for(unsigned n=0;n<4;++n){require(parts_[n]);
            frames_.hud_crosshair_axis_position(parts_[n],n<2?0u:1u,(n==0||n==3)?-field14_:field14_);
            frames_.update_scene_object(parts_[n]); //Even protected frames still receive24.
        }
    }
    static void require(Frame value){if(!value)throw std::runtime_error("Required HUD crosshair artwork is missing");}
    Frame indexed_material(Frame visual){const auto lod=frames_.first_person_material_lod(visual);require(lod);return frames_.first_person_group_material(lod,0);}
    Frames& frames_;Factory& factory_;Settings& settings_;Frame root_{},name_{};std::array<Frame,4> parts_{};
    std::unique_ptr<Text> text_;std::array<float,4> colour20_{};float field14_=0;const void* target_=nullptr;std::uint8_t flag34_=0,visible35_=0;
};
} //namespace hd2::game
#endif
