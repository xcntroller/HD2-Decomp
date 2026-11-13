#ifndef HD2_GAME_HUD_DIALOGS_LOADING_H
#define HD2_GAME_HUD_DIALOGS_LOADING_H
#include "game/hud_signal_tables.h"
#include "game/hud_text_loading.h"
#include "game/hud_roll_policy.h"
#include "game/hud_signal_policy.h"
#include "game/hud_roll_actions.h"
#include "frontend/cleanup.h"
#include <cmath>
#include <cstring>
#include <memory>
#include <functional>
#include <stdexcept>
namespace hd2::game {
//004B0750/004B0900 roll menu and004BF2E0/004BF480 signals. HUD scene,
//actual HUD identity, font/localization factory and PROCESS signal tables are
//borrowed. Gameplay action execution remains with the actual actor/Human owners.
template<class Frames,class Factory,class Hud>
class HudDialogsLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;
    struct Roll {Frame scene{};Hud* hud=nullptr;Frame root{},cursor{};std::unique_ptr<Text> text;
        struct Row {std::int32_t action=-1,caption=-1;};
        std::array<float,4> position20{};std::uint8_t flag30=0,flag31=0;const void* field88=nullptr;
        float distance00=0;std::uint32_t ticks18=0;std::array<Row,10> rows{};std::int32_t selected84=-1;}; //Actual borrowed actor88, no duplicate menu.
    struct Signals {Hud* hud=nullptr;Frame root{},scene{};std::uint32_t field0c=50;
        HudSignalTables::Row rows{};std::int32_t selected60=0;const void* target64=nullptr;const void* human68=nullptr;
        std::uint8_t flag6c=1;std::unique_ptr<Text> text;};
    HudDialogsLoading(Frames& frames,Factory& factory,HudSignalTables& tables):frames_(frames),factory_(factory),tables_(tables) {}
    void bind_roll(Hud& hud,Frame scene,std::uint8_t flag) { //Original RET12, third BYTE argument supplies31.
        bind_roll(&hud,scene,flag);
    }
    void bind_roll(Hud* hud,Frame scene,std::uint8_t flag) { //004B4980's nested roll explicitly passes a null HUD.
        cold(roll_.text);require(scene);roll_.scene=scene;roll_.hud=hud;
        roll_.root=frames_.hud_find(scene,"rollmenu",0xffff);require(roll_.root);
        roll_.position20=frames_.actor_position(roll_.root); //All FOUR nativeB0 words, no world-matrix update.
        roll_.cursor=frames_.hud_find(scene,"kurzor",0xffff);frames_.object_visible(roll_.root,false);
        roll_.text=make_text();bind_hud_text_geometry(frames_,*roll_.text,scene,roll_.root);
        roll_.field88=nullptr;roll_.flag31=flag;roll_.flag30=0;
        //Rows/selection are defined cold guards. Original004B0750 does not
        //populate them:004B0EF0 must run the real producer before interaction.
        roll_rows_ready_=false;
    }
    bool bind_signals(Hud& hud,Frame scene,std::uint32_t mode) { //004BF2E0 RET8, final table publish only after geometry.
        cold(signals_.text);require(scene);signals_.scene=scene;
        signals_.root=frames_.hud_find(scene,"signals",0xffff);signals_.hud=&hud;
        if(signals_.root)frames_.object_visible(signals_.root,false);
        signals_.field0c=50;signals_.text=make_text();
        bind_hud_text_geometry(frames_,*signals_.text,scene,signals_.root);
        signals_.selected60=0;signals_.target64=signals_.human68=nullptr;signals_.flag6c=1;signal_rows_ready_=false;tables_.select(mode);
        return signals_.root!=Frame{};
    }
    template<class ClearSelection>void hide_signals(ClearSelection&& clear){ //004BF4A0(false,0,0).
        signals_.target64=signals_.human68=nullptr;
        clear(); //0049D340(0,5), before root visibility/update.
        require(signals_.root);frames_.object_visible(signals_.root,false);frames_.update_scene_object(signals_.root);
    }
    void close_roll()noexcept {if(roll_.text)roll_.text->text.clear();roll_.text.reset();} //004B0900 clear then release.
    void set_roll_row(std::size_t index,std::int32_t caption,std::int32_t action){ //004B13A0, actual34/38 pair.
        roll_.rows.at(index)={action,caption};
    }
    template<class Queries>void format_roll(Queries& queries){ //004B0B20: existing text clears before all queries.
        auto& text=roll_text().text;text.clear();text.append(factory_.hud_localized(0x1b69),0x2f);
        if(roll_.field88){auto& actor=*queries.resolve(roll_.field88);const auto kind=queries.kind(actor);
            if(kind==3&&queries.human_kind(actor)==1)append_roll_string(queries.short_name(actor),0x31);
            else if(kind==6&&queries.door_locked(actor)){
                const auto suffix=factory_.hud_localized(0x1b8c);const auto name=queries.caption(actor);
                append_roll_string(name+" - "+(suffix?*suffix:std::string{}),0x31);
            }else append_roll_string(queries.caption(actor),0x31);
        }
        for(std::size_t index=0;index!=9;++index){const auto row=roll_.rows[index];
            if(row.action==-1){append_roll_string("",0x32);continue;}
            append_roll_string(std::to_string(index+1)+". ",0x32);
            if(row.caption<0){
                const auto identity=std::uint32_t{0}-10u-static_cast<std::uint32_t>(row.caption);std::int32_t signed_identity;std::memcpy(&signed_identity,&identity,4);
                if(auto* human=queries.human_by_identity(signed_identity))append_roll_string(queries.short_name(*human),roll_style(index,row.action));
            }else{
                if(row.action>7&&row.action<14){
                    if(!roll_.field88)throw std::logic_error("HUD roll seat row requires actual target actor");
                    auto& target=*queries.resolve(roll_.field88);auto* human=queries.occupant(target,static_cast<std::size_t>(row.action-8));
                    if(roll_.selected84!=static_cast<std::int32_t>(index)&&human){append_roll_string(queries.short_name(*human),0x33);continue;}
                }
                text.append(factory_.hud_localized(static_cast<std::uint32_t>(row.caption)),roll_style(index,row.action));
            }
        }
        append_roll_string("0. ",0x32);text.append(factory_.hud_localized(static_cast<std::uint32_t>(roll_.rows[9].caption)),roll_.selected84==9?0x35:0x33);
    }
    template<class Queries>void cycle_roll(Queries& queries){ //004B2F50 never wraps; click occurs even when no next row exists.
        require_roll_rows();if(roll_.selected84<9){
            for(auto next=roll_.selected84+1;next<=9;++next){const auto action=roll_.rows.at(static_cast<std::size_t>(next)).action;
                if(action!=-1&&action!=0x32){roll_.selected84=next;format_roll(queries);break;}
            }
        }
        factory_.hud_click();
    }
    template<class Queries>void previous_roll(Queries& queries){ //004B2F00 formats only on a changed valid selection, always clicks.
        require_roll_rows();if(roll_.selected84<0||roll_.selected84>9)throw std::logic_error("Roll previous requires actual selected row");
        for(auto next=roll_.selected84;next>0;){--next;const auto action=roll_.rows[static_cast<std::size_t>(next)].action;
            if(action!=-1&&action!=50){roll_.selected84=next;format_roll(queries);break;}}
        factory_.hud_click();
    }
    template<class Queries>void activate_roll(Queries& queries){ //004B2830 common tail runs only after successful action publication.
        require_roll_rows();dispatch_hud_roll(roll_,queries);roll_.field88=nullptr;hide_roll();
    }
    template<class Input,class Queries>void process_roll_controls(Input& input,Queries& queries){ //004B2DC0, ordinary SECOND snapshot only.
        if(roll_.flag31)return;require_roll_rows();const auto scan=input.next_scan(),character=input.scan_character(scan)&0xffu;
        if(character>=0x30&&character<=0x39){const auto row=character==0x30?9:character-0x31;input.consume_scan(scan);
            const auto action=roll_.rows[row].action;if(action==-1||action==50)return;
            roll_.selected84=static_cast<std::int32_t>(row);input.second(19,1.f);
        }
        if(input.second(19)!=0.f){activate_roll(queries);return;} //Native leaves activation binding19 set here.
        if(input.second(54)>0.f||input.second(0)!=0.f){input.consume_action(54);previous_roll(queries);}
        else if(input.second(54)<0.f||input.second(1)!=0.f){input.consume_action(54);cycle_roll(queries);}
    }
    bool roll_row_valid(){ //004B2BE0: strict cursor WORLD-position versus root WORLD-bound tests, not row eligibility.
        require(roll_.cursor);require(roll_.root);const auto cursor=frames_.actor_world_matrix(roll_.cursor);
        const auto bounds=frames_.hud_text_world_bounds(roll_.root);
        return bounds[0]<cursor[12]&&cursor[12]<bounds[4]&&cursor[13]<bounds[5]&&bounds[1]<cursor[13];
    }
    template<class Queries>bool choose_roll_row(unsigned row,Queries& queries){ //004B2CA0, changed valid row alone reformats.
        require_roll_rows();if(row>=roll_.rows.size())return false;const auto action=roll_.rows[row].action;
        if(action==-1||action==50)return false;
        if(roll_.selected84!=static_cast<std::int32_t>(row)){roll_.selected84=static_cast<std::int32_t>(row);format_roll(queries);}return true;
    }
    template<class Queries>void hover_roll(const std::array<float,4>& pointer,Queries& queries){ //004B2CE0, FLOAT constants then007E5E00.
        require_roll_rows();require(roll_.root);const auto bounds=frames_.hud_text_world_bounds(roll_.root);
        //Native x87 tests admit unordered coordinates into its masked-invalid
        //conversion. Preserve that policy instead of inventing a NaN clamp.
        if(pointer[0]<bounds[0]||pointer[0]>bounds[4]||pointer[1]>bounds[5]||pointer[1]<bounds[1])return;
        const auto value=std::trunc((-static_cast<long double>(pointer[1])-(-static_cast<long double>(bounds[5]))-.04500000178813934f)*58.823524475097656f);
        std::uint32_t low=0;if(std::isfinite(value)&&value>=-0x1p63L&&value<0x1p63L)low=static_cast<std::uint32_t>(static_cast<std::int64_t>(value));
        std::int32_t index;std::memcpy(&index,&low,4);
        //Original has no upper bound before indexing. Reject invalid artwork
        //geometry rather than borrowing memory past the actual ten rows.
        if(index<0||index>=10){roll_.selected84=-1;return;}
        const auto action=roll_.rows[static_cast<std::size_t>(index)].action;
        if(action==-1||action==0x32){roll_.selected84=-1;return;}
        if(roll_.selected84!=index){roll_.selected84=index;format_roll(queries);}
    }
    void refresh_signal_geometry(){bind_hud_text_geometry(frames_,signal_text(),signals_.scene,signals_.root);} //004C10D0.
    void refresh_roll_geometry(){ //004B3010.
        bind_hud_text_geometry(frames_,roll_text(),roll_.scene,roll_.root);
    }
    void position_roll(const std::array<float,4>& position){ //004B3030; frame08 copies its ordinary position contract.
        require(roll_.root);frames_.hud_scroll_position(roll_.root,position);frames_.update_scene_object(roll_.root);refresh_roll_geometry();
    }
    void restore_roll_position(){ //004B3060; direct protected XYZ guard, always update and geometry refresh.
        require(roll_.root);frames_.hud_message_position(roll_.root,roll_.position20[0],roll_.position20[1],roll_.position20[2]);
        frames_.update_scene_object(roll_.root);refresh_roll_geometry();
    }
    template<class Queries>void advance_roll(std::uint32_t elapsed,Queries& queries){ //004B2FC0, command31 skips the entire body.
        if(roll_.flag31)return;auto* human=queries.selected_human();if(!human||queries.health(*human)<1)hide_roll();
        if(roll_.ticks18)roll_.ticks18-=elapsed; //No clamp; zero does not restart a timer.
    }
    void hide_roll(){require(roll_.root);frames_.object_visible(roll_.root,false);roll_.flag30=0;} //004B27B0(false).
    template<class Queries>void rebuild_roll(Queries& queries){ //004B0EF0, real row policies then selection/click.
        roll_rows_ready_=false;
        if(roll_.flag31)populate_hud_roll_command(*this,queries);else populate_hud_roll_ordinary(*this,queries);
        roll_rows_ready_=true;roll_.selected84=-1;cycle_roll(queries);
    }
    template<class Queries>void show_roll(bool enabled,Queries& queries){ //004B27B0.
        if(!enabled){hide_roll();return;}
        auto* actor=roll_.field88?queries.resolve(roll_.field88):nullptr;
        const bool admitted=roll_.flag31?hud_roll_command_eligible(queries,actor):hud_roll_ordinary_eligible(queries,actor,roll_.distance00);
        if(!admitted){roll_.flag30=0;return;} //Native rejection does not call frame30(false).
        roll_.flag30=1;rebuild_roll(queries);format_roll(queries);roll_.ticks18=400;
        require(roll_.root);frames_.object_visible(roll_.root,true);
    }
    template<class Actor,class Queries>void publish_roll_target(Actor* actor,float distance,Queries& queries){ //004B0920.
        if(roll_.flag31){
            auto* human=queries.previous_human();if(!human){roll_.distance00=0;roll_.field88=nullptr;roll_.distance00=distance;return;}
            if(!actor)throw std::logic_error("Command roll target reached native null actor dereference");
            if(queries.kind(*actor)==21){if(queries.occupant(*actor,0)){
                auto* occupant=queries.occupant(*actor,0);if(!occupant)throw std::logic_error("StaticWeapon occupant retired during roll query");
                if(queries.hostile(*occupant,*human)){auto* third=queries.occupant(*actor,0);if(!third)throw std::logic_error("StaticWeapon occupant retired during roll publication");
                    roll_.field88=queries.identity(*third);roll_.distance00=distance;return;}
            }}
            if(actor==queries.focus(*human)){roll_.field88=queries.identity(*actor);roll_.distance00=distance;return;}
            if(actor==queries.attachment(*human)){roll_.field88=queries.identity(*actor);roll_.distance00=distance;return;}
            roll_.field88=queries.identity(*actor);roll_.distance00=distance;return;
        }
        auto* human=queries.selected_human();const auto previous=roll_.field88;
        if(!human){roll_.distance00=0;roll_.field88=nullptr;}
        else if(auto* focus=queries.focus(*human)){roll_.field88=queries.identity(*focus);roll_.distance00=.2f;}
        else{
            auto* attached=queries.attachment(*human);
            if(attached&&hud_roll_vehicle_kind(queries.kind(*attached))){roll_.field88=queries.identity(*attached);roll_.distance00=.2f;}
            else{roll_.field88=actor?queries.identity(*actor):nullptr;roll_.distance00=distance;
                if(!(distance<=1.7999999523162842f)){if(roll_.flag30)show_roll(false,queries);roll_.field88=nullptr;}
            }
        }
        if(previous==roll_.field88)return;
        if(roll_.flag30){if(roll_.field88&&roll_.distance00<=1.7999999523162842f){show_roll(false,queries);show_roll(true,queries);}else show_roll(false,queries);}
        else if(!hud_roll_ordinary_eligible(queries,roll_.field88?queries.resolve(roll_.field88):nullptr,roll_.distance00))roll_.field88=nullptr;
    }
    void roll_target_identity(const void* identity){roll_.field88=identity;} //Only checked in-call queries publish borrowed ActorBase identities.
    void draw_roll(){ //004B2FA0 checks ROOT0C; the caller separately checks flag30.
        require(roll_.root);if(!frames_.hud_frame_visible(roll_.root))return;
        if(!roll_.text)throw std::logic_error("HUD roll draw before text initialization");
        factory_.hud_draw_text(*roll_.text);
    }
    bool signals_active(){require(signals_.root);return frames_.hud_frame_visible(signals_.root);} //004C05F0.
    template<class Queries>void populate_signals(Queries& queries){signal_rows_ready_=false;const auto* table=tables_.active();if(!table)throw std::logic_error("Signals require actual process table publication");populate_hud_signals(signals_,*table,queries);signal_rows_ready_=true;}
    template<class Queries>void format_signals(Queries& queries){ //004C0370, text rebuild ends by showing root.
        if(!signal_rows_ready_)throw std::logic_error("Signal text requires actual row population");auto& text=signal_text().text;text.clear();
        if(signals_.field0c==50)text.append(factory_.hud_localized(0x1c66),0x2f);
        else{const auto* table=tables_.active();if(!table||signals_.field0c<51||signals_.field0c>59)throw std::logic_error("Signal heading exceeds actual process table");
            text.append(factory_.hud_localized(0x1c67),0x2f);text.append(factory_.hud_localized((*table)[0][(signals_.field0c-51)*2]),0x30);}
        if(signals_.field0c==53||signals_.field0c==56){
            if(!signals_.target64)throw std::logic_error("Signal target heading requires actual actor64");append_signal_string(queries.caption(*queries.resolve(signals_.target64)),0x31);
        }else if(signals_.human68)append_signal_string(queries.short_name(*queries.resolve(signals_.human68)),0x31);
        else text.append(factory_.hud_localized(0x1c6c),0x31);
        for(unsigned row=0;row!=9;++row){const auto caption=signals_.rows[row*2],action=signals_.rows[row*2+1];
            if(caption==0xffffffffu){append_signal_string("",0x32);continue;}
            append_signal_string(std::to_string(row+1)+". ",0x32);text.append(factory_.hud_localized(caption),action==72?0x34:signals_.selected60==static_cast<std::int32_t>(row)?0x35:0x33);
        }
        append_signal_string("0. ",0x32);text.append(factory_.hud_localized(signals_.rows[18]),signals_.selected60==9?0x35:0x33);
        require(signals_.root);frames_.object_visible(signals_.root,true);
    }
    template<class Queries>void cycle_signals(Queries& queries){ //004C06E0 always clicks THEN formats, unlike roll cycling.
        if(!signal_rows_ready_)throw std::logic_error("Signal cycle requires actual row population");
        if(signals_.selected60< -1)throw std::logic_error("Signal selection precedes native first row");
        if(signals_.selected60<9)for(auto next=signals_.selected60+1;next<=9;++next){const auto action=signals_.rows.at(static_cast<std::size_t>(next)*2+1);if(action!=0xffffffffu&&action!=72){signals_.selected60=next;break;}}
        factory_.hud_click();format_signals(queries);
    }
    template<class Queries>void previous_signal(Queries& queries){ //004C0720, descending search does not wrap.
        if(!signal_rows_ready_||signals_.selected60<0||signals_.selected60>9)throw std::logic_error("Signal previous requires an actual selected row");
        for(auto next=signals_.selected60;next>0;){--next;const auto action=signals_.rows[static_cast<std::size_t>(next)*2+1];
            if(action!=0xffffffffu&&action!=72){signals_.selected60=next;break;}}
        factory_.hud_click();format_signals(queries);
    }
    template<class Queries>void select_signal(Queries& queries){ //004C0600: execute, hide, click; navigation cycles AND clicks.
        if(!signal_rows_ready_||signals_.selected60<0||signals_.selected60>9)throw std::logic_error("Signal activation requires an actual selected row");
        const auto action=signals_.rows[static_cast<std::size_t>(signals_.selected60)*2+1];
        if((action>=51&&action<=59)||action==71){signals_.field0c=action==71?50:action;signals_.selected60=-1;populate_signals(queries);cycle_signals(queries);}
        else if(action!=0xffffffffu&&action!=72){
            if(action!=70)queries.dispatch_signal(signals_);
            using Actor=typename Queries::Actor;show_signals(false,static_cast<Actor*>(nullptr),static_cast<Actor*>(nullptr),queries);
        }
        factory_.hud_click();
    }
    template<class Queries>void select_signal_row(std::uint32_t row,Queries& queries){ //004C0760's digit branch, after scan consumption.
        if(!signal_rows_ready_||row>9)throw std::out_of_range("Signal digit exceeds actual populated rows");
        const auto action=signals_.rows[row*2+1];if(action==0xffffffffu||action==72)return;
        signals_.selected60=static_cast<std::int32_t>(row);select_signal(queries);
    }
    template<class Input,class Queries>void process_signal_controls(Input& input,Queries& queries){ //004C0760 RET8, reads SECOND snapshot.
        const auto scan=input.next_scan(),character=input.scan_character(scan)&0xffu;
        if(character>=0x30&&character<=0x39){input.consume_scan(scan);select_signal_row(character==0x30?9:character-0x31,queries);return;}
        if(input.second(19)!=0.f){input.consume_action(19);select_signal(queries);return;}
        if(input.second(54)>0.f){input.consume_action(54);previous_signal(queries);return;}
        if(input.second(54)<0.f){input.consume_action(54);cycle_signals(queries);}
    }
    template<class Queries>void reset_signal_target(Queries& queries){ //004C11C0; active50/53/56 only, row selection becomes0.
        if(!signals_active()||(signals_.field0c!=50&&signals_.field0c!=53&&signals_.field0c!=56))return;
        signals_.field0c=50;signals_.selected60=0;populate_signals(queries);format_signals(queries);
    }
    template<class Actor,class Queries>void publish_signal_target(Actor* target,Queries& queries){ //004C0EF0.
        if(!signals_active())return;const auto identity=target?queries.identity(*target):nullptr;const auto old=signals_.target64;if(old==identity)return;
        signals_.target64=identity;
        if(!target&&signals_.human68){auto& human=*queries.resolve(signals_.human68);
            auto* fallback=queries.attachment(human);if(!fallback)fallback=queries.focus(human);if(fallback)signals_.target64=queries.identity(*fallback);}
        if(old==signals_.target64)return;
        if(signals_.field0c!=50){if(signals_.field0c!=53&&signals_.field0c!=56)return;signals_.field0c=50;}
        populate_signals(queries);format_signals(queries);
    }
    template<class Actor,class Queries>void show_signals(bool enabled,Actor* target,Actor* human,Queries& queries){ //004BF4A0.
        signals_.target64=target?queries.identity(*target):nullptr;signals_.human68=human?queries.identity(*human):nullptr;
        if(!enabled)queries.publish_signal_selection(nullptr,5);
        else{
            if(!queries.offline()&&(human||queries.hud_style()==0))return;
            if(!target&&human){auto* fallback=queries.attachment(*human);if(!fallback)fallback=queries.focus(*human);if(fallback)signals_.target64=queries.identity(*fallback);}
            auto* selected=queries.selected_human();if(!selected||queries.health(*selected)<1)return;
            if(queries.offline()){
                const auto recipients=queries.signal_recipients(0);
                if(!human)signals_.flag6c=recipients.empty();
                else for(auto n=recipients.size();n;)if(recipients[--n]==human)signals_.flag6c=0;
            }else signals_.flag6c=0;
            signals_.field0c=50;signals_.selected60=-1;populate_signals(queries);cycle_signals(queries);
        }
        require(signals_.root);frames_.object_visible(signals_.root,enabled);frames_.update_scene_object(signals_.root);
    }
    template<class Queries>void advance_signals(std::uint32_t elapsed,Queries& queries){ //004C0F70, elapsed is unused.
        (void)elapsed;using Actor=typename Queries::Actor;auto* selected=queries.selected_human();
        if(!selected||queries.health(*selected)<1){show_signals(false,static_cast<Actor*>(nullptr),static_cast<Actor*>(nullptr),queries);return;}
        if(!queries.offline())return;
        if(signals_.human68&&queries.health(*queries.resolve(signals_.human68))<1)show_signals(false,static_cast<Actor*>(nullptr),static_cast<Actor*>(nullptr),queries);
        if(queries.live_humans()<2){show_signals(false,static_cast<Actor*>(nullptr),static_cast<Actor*>(nullptr),queries);return;}
        queries.publish_signal_selection(signals_.human68?queries.resolve(signals_.human68):nullptr,signals_.field0c==50?0u:signals_.field0c==58?2u:1u);
        const auto recipients=queries.signal_recipients(0);
        if(signals_.flag6c){if(!recipients.empty())show_signals(true,signals_.target64?queries.resolve(signals_.target64):nullptr,static_cast<Actor*>(nullptr),queries);}
        else if(recipients.empty())show_signals(true,static_cast<Actor*>(nullptr),static_cast<Actor*>(nullptr),queries);
    }
    void draw_signals(){ //004C0880 rereads the SAME root visibility before text70.
        require(signals_.root);if(!frames_.hud_frame_visible(signals_.root))return;
        if(!signals_.text)throw std::logic_error("HUD signals draw before text initialization");
        factory_.hud_draw_text(*signals_.text);
    }
    void close_signals()noexcept {if(signals_.text)signals_.text->text.clear();signals_.text.reset();signal_rows_ready_=false;} //004BF480; active process table stays.
    void check_retirement_binding()const{if(retirement_owner_)throw std::logic_error("HUD dialog already has its actual UI retirement receiver");}
    void bind_retirement(void* owner,std::function<void(const void*,bool)> receiver){
        check_retirement_binding();if(!owner||!receiver)throw std::invalid_argument("HUD retirement requires actual owner/receiver");
        retirement_=std::move(receiver);retirement_owner_=owner;
    }
    void unbind_retirement(void* owner)noexcept{if(retirement_owner_==owner){retirement_={};retirement_owner_=nullptr;discard_actor_borrowers();}}
    void discard_actor_borrowers()noexcept{roll_.field88=nullptr;roll_.flag30=0;roll_rows_ready_=false;signals_.target64=signals_.human68=nullptr;}
    void retire_roll_actor(const void* actor,bool reset_signals) { //0049EF70's004B0920/004C11C0 consumers.
        if(roll_.field88!=actor&&signals_.target64!=actor&&signals_.human68!=actor)return;
        frontend::CleanupErrors errors;
        if(retirement_)errors.attempt([&]{retirement_(actor,reset_signals);});
        else errors.attempt([]{throw std::runtime_error("Populated HUD dialog requires its actual UI retirement binding");});
        //Actor cleanup can aggregate a failed required producer and continue.
        //Retract any remaining identity even on failure; never keep a dead loan.
        if(roll_.field88==actor){roll_.field88=nullptr;roll_.flag30=0;roll_rows_ready_=false;if(roll_.text)roll_.text->text.clear();}
        if(signals_.target64==actor)signals_.target64=nullptr;if(signals_.human68==actor)signals_.human68=nullptr;
        errors.rethrow();
    }
    void discard_borrowers()noexcept {
        roll_.scene=roll_.root=roll_.cursor=Frame{};roll_.hud=nullptr;
        signals_.root=signals_.scene=Frame{};signals_.hud=nullptr;
        roll_.field88=nullptr;signals_.target64=signals_.human68=nullptr;
        roll_rows_ready_=false;
        signal_rows_ready_=false;
    }
    const auto& roll()const{return roll_;}const auto& signals()const{return signals_;}
private:
    Text& signal_text(){if(!signals_.text)throw std::logic_error("Signal service before actual text initialization");return *signals_.text;}
    void append_signal_string(std::string value,std::uint32_t style){signal_text().text.append(std::make_shared<const std::string>(std::move(value)),style);}
    Text& roll_text(){if(!roll_.text)throw std::logic_error("HUD roll service before actual text initialization");return *roll_.text;}
    void require_roll_rows()const{if(!roll_rows_ready_)throw std::logic_error("HUD roll interaction requires actual row population");}
    void append_roll_string(std::string value,std::uint32_t style){roll_text().text.append(std::make_shared<const std::string>(std::move(value)),style);}
    std::uint32_t roll_style(std::size_t index,std::int32_t action)const{return action==0x32?0x34:roll_.selected84==static_cast<std::int32_t>(index)?0x35:0x33;}
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD dialog artwork is missing");}
    static void cold(const std::unique_ptr<Text>& owner){if(owner)throw std::logic_error("HUD dialog rebound before teardown");}
    std::unique_ptr<Text> make_text(){auto result=factory_.make_hud_text();if(!result)throw std::runtime_error("HUD dialog text factory returned null");return result;}
    Frames& frames_;Factory& factory_;HudSignalTables& tables_;Roll roll_;Signals signals_;bool roll_rows_ready_=false,signal_rows_ready_=false;
    void* retirement_owner_=nullptr;std::function<void(const void*,bool)> retirement_;
};
} //namespace hd2::game
#endif
