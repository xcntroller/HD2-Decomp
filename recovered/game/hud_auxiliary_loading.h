#ifndef HD2_GAME_HUD_AUXILIARY_LOADING_H
#define HD2_GAME_HUD_AUXILIARY_LOADING_H
#include "game/hud_artwork_loading.h"
#include "game/hud_text_loading.h"
#include "frontend/cleanup.h"
#include <cmath>
#include <cstring>
#include <memory>
#include <stdexcept>
namespace hd2::game {
//005A5F50's required borrowed Human-order publication. No task/list owner or
//native pointer is copied into HUD storage; each record is consumed in-call.
struct HudOrderDisplay {std::uint32_t kind=0,parameter=0;bool selected=false;};
//004AB860 world-pointer,004AB230 info icons,004AAF20 progress loading and
//matching004AB950/004AB2D0/004AB0F0. These are real HUD scene borrowers;
//containing HUD must close progress/discard borrowers before scene release.
template<class Frames,class Factory>
class HudAuxiliaryLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;
    struct Pointer {
        Frame field00{},hud_scene{},world_scene{};std::int32_t field0c=-1;
        std::array<float,4> position10{},position20{};std::array<Frame,16> frames30{};
        float field70=0,field74=0;const void* clip78=nullptr; //Borrowed eight-FLOAT rectangle bytes; no C++ array object is invented inside an engine frame.
    };
    struct Info {
        const void* human=nullptr;const void* actor_identity=nullptr;std::array<Frame,10> frames{};
        const std::array<Frame,20>* states=nullptr;const std::array<Frame,39>* actions=nullptr;Frame selected{};
    };
    struct Progress {Frame background{},bar{};std::uint32_t field08=0,field0c=0,field10=0xffffffffu;std::unique_ptr<Text> text;};
    HudAuxiliaryLoading(Frames& frames,Factory& factory):frames_(frames),factory_(factory) {}
    void bind_pointer(Frame hud_scene,Frame world_scene) { //004AB860; literal table repeats k_move at slots1/14/15.
        require(hud_scene);pointer_.hud_scene=hud_scene;pointer_.world_scene=world_scene;
        constexpr std::array<const char*,16> names{{"kurzor","k_move","k_rotate","t_rotleft","t_rotright","t_rotup","t_rotdown","t_left","t_right","t_forward","t_backward","t_up","t_down","k_movepos","k_move","k_move"}};
        for(unsigned i=0;i!=names.size();++i){pointer_.frames30[i]=frames_.hud_find(hud_scene,names[i],1);show(pointer_.frames30[i],false);}
        pointer_.field70=pointer_.field74=1;pointer_.field00=0;pointer_.field0c=-1;pointer_.clip78=nullptr;
        for(auto* position:{&pointer_.position20,&pointer_.position10}) {
            (*position)[0]=.5f;(*position)[1]=-.375f;(*position)[2]=-10;
            //Both native fourth stack lanes are unwritten. Keep known lanes;
            //their cold zeroes are guards, not constructor recovery credit.
        }
    }
    void bind_info(Frame scene,const HudArtworkLoading<Frames>& artwork) { //004AB230.
        require(scene);
        for(unsigned i=0;i!=info_.frames.size();++i) {
            std::array<char,32> name{};std::snprintf(name.data(),name.size(),"info%02u",i);
            auto& frame=info_.frames[i];frame=frames_.hud_find(scene,name.data(),0xffff);require(frame);
            frames_.hud_group18(frame);show(frame,false);
        }
        info_.human=info_.actor_identity=nullptr;info_.states=&artwork.first_states();info_.actions=&artwork.actions();
        info_.selected=frames_.hud_find(scene,"infosel",0xffff);show(info_.selected,false);
    }
    void bind_progress(Frame scene) { //004AAF20.
        if(progress_.text)throw std::logic_error("HUD progress rebound before teardown");
        require(scene);progress_.background=frames_.hud_find(scene,"prgbrbk",1);
        progress_.bar=frames_.hud_find(scene,"prgbr01",1);progress_.field08=progress_.field0c=0;
        require(progress_.background);
        frames_.hud_position(progress_.background,{{.5f,-.579999983310699462890625f,-10}});
        frames_.update_scene_object(progress_.background);
        const auto name=frames_.actor_find(progress_.background,"progress_name",0xffff);show(name,false);
        progress_.field10=0xffffffffu;progress_.text=factory_.make_hud_text();
        if(!progress_.text)throw std::runtime_error("HUD progress text factory returned null");
        bind_hud_text_geometry(frames_,*progress_.text,scene,progress_.background);show(progress_.background,false);
    }
    void update_pointer(std::uint32_t elapsed){ //004ABE60, no model24/visibility gate.
        if(pointer_.field70==pointer_.field74)return;
        //Native IMUL wraps BEFORE signed integer-to-float conversion.
        const auto bits=elapsed*5u;std::int32_t product;std::memcpy(&product,&bits,4);
        const auto step=static_cast<long double>(product)*.001f;
        if(!(pointer_.field70<pointer_.field74)){
            const auto value=static_cast<long double>(pointer_.field70)-step;
            pointer_.field70=static_cast<float>(value);
            if(value<pointer_.field74)pointer_.field70=pointer_.field74;
        }else{
            const auto value=static_cast<long double>(pointer_.field70)+step;
            pointer_.field70=static_cast<float>(value);
            if(value>pointer_.field74)pointer_.field70=pointer_.field74;
        }
        require(pointer_.field00);
        frames_.hud_info_scale(pointer_.field00,pointer_.field70,pointer_.field70,pointer_.field70);
    }
    template<class Query>void update_progress(Query&& query){ //004AB110; SAME current-Human query, never advances its action timer.
        const auto value=query();if(!value){show(progress_.background,false);return;}
        //Original FILD/FIDIV retains signed integer precision until ONE FLOAT store.
        const auto ratio=static_cast<long double>(value->current)/value->maximum;
        require(progress_.bar);frames_.hud_info_scale(progress_.bar,1.f,1.f,static_cast<float>(ratio));
        show(progress_.background,true);frames_.update_scene_object(progress_.bar);
        const auto caption=static_cast<std::uint32_t>(value->caption);
        if(caption!=progress_.field10){
            if(!progress_.text)throw std::logic_error("HUD progress frame requires its actual text owner");
            progress_.text->text.clear();progress_.field10=caption;
            if(value->caption>=0)progress_.text->text.append(factory_.hud_localized(caption),0x42);
        }
    }
    template<class Queries>void update_order_display(const void* human,Queries& queries){ //004AB2E0.
        if(!human)throw std::logic_error("HUD order display requires actual current/previous Human");
        if(info_.human==human&&!queries.changed())return;
        info_.human=human;info_.actor_identity=queries.actor_identity();show(info_.selected,false);
        Frame texture{};std::uint32_t index=0;
        if(queries.count()>0){do{
            if(index>=info_.frames.size())throw std::runtime_error("HUD orders exceed native ten-icon buffer");
            const auto order=queries.order(index);const auto frame=info_.frames[index];require(frame);
            //Unknown kinds retain the previous texture, as the native stack
            //slot initialized ONCE before the loop does. A zero texture skips binding.
            switch(order.kind){
            case 1:texture=state_texture(10);break;case 2:texture=state_texture(11);break;
            case 3:if(!info_.actions)throw std::logic_error("HUD orders lost SAME action artwork table");texture=info_.actions->at(order.parameter);break;
            case 4:texture=state_texture(19);break;case 6:texture=state_texture(13);break;case 7:texture=state_texture(14);break;default:break;
            }
            const float scale=order.selected?1.1f:1.f;frames_.hud_info_scale(frame,scale,scale,scale);
            if(order.selected){
                const auto world=frames_.actor_world_matrix(frame);
                std::array<float,4> position{{world[12],world[13],static_cast<float>(static_cast<long double>(world[14])-1.f),world[15]}};
                frames_.hud_info_scale(info_.selected,1.1f,1.1f,1.1f);frames_.hud_scroll_position(info_.selected,position);
                show(info_.selected,true);frames_.update_scene_object(info_.selected);
            }
            if(texture)frames_.first_person_material_texture(frames_.hud_first_material(frame),texture);
            show(frame,true);frames_.update_scene_object(frame);++index;
        }while(static_cast<std::int64_t>(index)<queries.count());} //Fresh count AFTER each material/visibility/update callback.
        for(;index<info_.frames.size();++index)show(info_.frames[index],false);
    }
    void close_pointer()noexcept{} //004AB950 actual RET.
    void select_pointer(std::int32_t index){ //004ABAF0, hide/store/borrow/position/show/update order.
        if(pointer_.field0c==index)return;
        if(pointer_.field00)frames_.object_visible(pointer_.field00,false);
        pointer_.field0c=index;
        if(index==-1)return; //Original retains field00; no update on this branch.
        if(index<0||static_cast<std::size_t>(index)>=pointer_.frames30.size())throw std::out_of_range("HUD pointer index exceeds native artwork table");
        pointer_.field00=pointer_.frames30[static_cast<std::size_t>(index)];require(pointer_.field00);
        frames_.hud_pointer_position(pointer_.field00,pointer_.position20);
        frames_.object_visible(pointer_.field00,true);require(pointer_.field00);frames_.update_scene_object(pointer_.field00);
    }
    std::int32_t pointer_kind()const{return pointer_.field0c;} //004ABB70.
    void rotate_command_pointer(float angle){ //004B66C1, frame virtual1C; no scene-object update follows.
        require(pointer_.field00);frames_.hud_pointer_direction(pointer_.field00,{{0,0,1,0}},angle);
    }
    template<class Queries>std::int32_t selected_order_index(Queries& queries){ //004AB820, fresh Human count after each miss.
        if(!info_.human)return -1;
        for(std::uint32_t index=0;static_cast<std::int64_t>(index)<queries.count();++index)
            if(queries.order(index).selected)return static_cast<std::int32_t>(index);
        return -1;
    }
    template<class Queries>void select_order_index(std::int32_t selected,Queries& queries){ //004AB700, owner supplies actual Human order4A mutation.
        show(info_.selected,false);
        for(std::uint32_t n=0;static_cast<std::int64_t>(n)<queries.count();++n){queries.mark_selected(n,static_cast<std::int64_t>(n)==selected);
            if(static_cast<std::int64_t>(n)==selected){const auto frame=info_.frames.at(n);require(frame);const auto world=frames_.actor_world_matrix(frame);
                frames_.hud_scroll_position(info_.selected,{{world[12],world[13],static_cast<float>(static_cast<long double>(world[14])-1.f),world[15]}});show(info_.selected,true);frames_.update_scene_object(info_.selected);}}
    }
    template<class Queries>void select_order_token(const void* token,Queries& queries){ //004AB610's loop after containing current-owner refresh.
        show(info_.selected,false);for(std::uint32_t n=0;static_cast<std::int64_t>(n)<queries.count();++n){const bool selected=queries.order_identity(n)==token;queries.mark_selected(n,selected);
            if(selected){const auto frame=info_.frames.at(n);require(frame);const auto world=frames_.actor_world_matrix(frame);
                frames_.hud_scroll_position(info_.selected,{{world[12],world[13],static_cast<float>(static_cast<long double>(world[14])-1.f),world[15]}});show(info_.selected,true);frames_.update_scene_object(info_.selected);}}
    }
    const auto& pointer_position()const{return pointer_.position20;} //004AB990, borrowed only during the call.
    std::array<float,4> pointer_delta()const{ //004AB960 writes XYZ only; the native fourth lane is unwritten.
        auto value=pointer_.position20;for(unsigned n=0;n!=3;++n)value[n]=static_cast<float>(static_cast<long double>(value[n])-pointer_.position10[n]);return value;
    }
    bool pointer_accepts_input(){return pointer_.field00&&frames_.hud_frame_visible(pointer_.field00)&&pointer_.field0c!=2;}
    template<class Input>void process_pointer(Input& input){ //004ABC50, FIRST buffer only; no model24 or new polling.
        if(!pointer_accepts_input())return;pointer_.position10=pointer_.position20;auto& value=pointer_.position20;
        value[0]=static_cast<float>(static_cast<long double>(input.first(52))+value[0]);value[1]=static_cast<float>(static_cast<long double>(value[1])-input.first(53));
        std::array<float,8> clip_value{};if(pointer_.clip78){std::memcpy(clip_value.data(),pointer_.clip78,sizeof(clip_value));const auto* clip=&clip_value;constexpr float edge=.0020000000949949026f;
            const auto left=static_cast<long double>((*clip)[0])+edge,right=static_cast<long double>((*clip)[4])-edge;
            if(value[0]<left)value[0]=static_cast<float>(left);else if(right<value[0])value[0]=static_cast<float>(right);
            const auto low=static_cast<long double>((*clip)[1])+edge,high=static_cast<long double>((*clip)[5])-edge;
            if(value[1]<low)value[1]=static_cast<float>(low);else if(high<value[1])value[1]=static_cast<float>(high);
        }else{
            if(value[0]<0){value[0]=0;input.first(52,0);}if(value[1]>0){value[1]=0;input.first(53,0);}
            if(.9950000047683716f<value[0]){value[0]=.9950000047683716f;input.first(52,0);}
            if(value[1]<-.7450000047683716f){input.first(53,0);value[1]=-.7450000047683716f;}
        }
        require(pointer_.field00);frames_.hud_message_position(pointer_.field00,value[0],value[1],value[2]);
    }
    void clip_pointer_to(Frame visual){require(visual);pointer_.clip78=frames_.hud_text_world_bounds_borrower(visual);}
    void clear_pointer_clip()noexcept{pointer_.clip78=nullptr;}
    template<class Input>void restrict_pointer_delta(Input& input){ //004AC090 filters FIRST52/53 without moving/copying the pointer or consuming aliases.
        const auto x=static_cast<long double>(pointer_.position20[0])+input.first(52); //Native X stays extended; Y is stored FLOAT before testing bounds.
        const float y=static_cast<float>(static_cast<long double>(pointer_.position20[1])-input.first(53));
        if(!pointer_.clip78){if(x<0)input.first(52,0);if(y>0)input.first(53,0);if(.9950000047683716f<x)input.first(52,0);if(y<-.7450000047683716f)input.first(53,0);}
        else {std::array<float,8> clip{};std::memcpy(clip.data(),pointer_.clip78,sizeof(clip));constexpr float edge=.0020000000949949026f;
            if(x<static_cast<long double>(clip[0])+edge)input.first(52,0);if(static_cast<long double>(clip[4])-edge<x)input.first(52,0);
            if(y<static_cast<long double>(clip[1])+edge)input.first(53,0);if(static_cast<long double>(clip[5])-edge<y)input.first(53,0);}
    }
    std::uint32_t pointer_hit_tag(){ //004ABDD0; hit-test HUD scene04, not the containing World08.
        require(pointer_.hud_scene);auto origin=pointer_.position20;
        origin[2]=static_cast<float>(static_cast<long double>(origin[2])+.5f);
        return frames_.hud_pointer_hit_tag(pointer_.hud_scene,origin,{{0,0,1,0}});
    }
    std::array<std::int32_t,2> pointer_screen_position(){ //004AB9A0, two007E5E00 truncations; width stored to FLOAT first.
        const float width=static_cast<float>(factory_.hud_display_width());const auto height=factory_.hud_display_height();
        return {{screen_integer(static_cast<long double>(pointer_.position20[0])*width),
                 screen_integer(static_cast<long double>(pointer_.position20[1])*-1.3333333333333333*height)}}; //DOUBLE00810508.
    }
    static bool pointer_in_command_area(const std::array<float,4>& position){ //004BF230; unordered coordinates reject.
        return position[0]>.01f&&position[0]<.99f&&position[1]<-.01f&&position[1]>-.61f;
    }
    void project_pointer(const std::array<float,4>& position){ //004ABF50, receiver is SAME HUD pointer from command44.
        require(pointer_.world_scene);
        const auto projected=frames_.hud_project_world_point(pointer_.world_scene,position);
        const auto width=static_cast<float>(factory_.hud_display_width());
        const auto height=factory_.hud_display_height(); //Original height remains extended after signed FILD.
        const float x=static_cast<float>(static_cast<long double>(projected[0])/width);
        const auto y=static_cast<long double>(projected[1])/height*-.75; //DOUBLE00810518.
        pointer_.position20[0]=static_cast<float>(x);pointer_.position20[1]=static_cast<float>(y);
        std::array<float,8> clip_value{};if(pointer_.clip78){std::memcpy(clip_value.data(),pointer_.clip78,sizeof(clip_value));const auto* clip=&clip_value;
            const auto left=static_cast<long double>((*clip)[0])+.002f,right=static_cast<long double>((*clip)[4])-.002f;
            const auto top=static_cast<long double>((*clip)[1])+.002f,bottom=static_cast<long double>((*clip)[5])-.002f;
            if(x<left)pointer_.position20[0]=static_cast<float>(left);else if(x>right)pointer_.position20[0]=static_cast<float>(right);
            const float stored_y=static_cast<float>(y); //Original clamp compares the stored screen-coordinate local.
            if(stored_y<top)pointer_.position20[1]=static_cast<float>(top);else if(stored_y>bottom)pointer_.position20[1]=static_cast<float>(bottom);
        }
        require(pointer_.field00);frames_.hud_message_position(pointer_.field00,pointer_.position20[0],pointer_.position20[1],pointer_.position20[2]);
        frames_.update_scene_object(pointer_.field00);
    }
    void hide_selected_pointer() { //004ABAF0(-1), reached by004A5630(false,false) on multiplayer map bind.
        select_pointer(-1);
    }
    void close_info()noexcept{} //004AB2D0 actual RET; texture tables are borrowed.
    void retire_order_actor(const void* identity)noexcept{if(info_.actor_identity==identity)info_.human=info_.actor_identity=nullptr;} //Defined guard before actual Human owner destruction.
    void close_progress() { //004AB0F0 hides before releasing text, with defined failure cleanup.
        frontend::CleanupErrors errors;errors.attempt([&]{show(progress_.background,false);});
        progress_.text.reset();errors.rethrow();
    }
    void draw_progress(){ //004AB220 unconditional text virtual+04.
        if(!progress_.text)throw std::logic_error("HUD progress draw before text initialization");
        factory_.hud_draw_text(*progress_.text);
    }
    void discard_borrowers()noexcept {
        pointer_.field00=pointer_.hud_scene=pointer_.world_scene=Frame{};pointer_.frames30.fill(Frame{});pointer_.clip78=nullptr;
        info_.frames.fill(Frame{});info_.selected=Frame{};info_.states=nullptr;info_.actions=nullptr;info_.human=info_.actor_identity=nullptr;
        progress_.background=progress_.bar=Frame{};
    }
    auto& pointer(){return pointer_;}
    const auto& pointer()const{return pointer_;}const auto& info()const{return info_;}const auto& progress()const{return progress_;}
private:
    static std::int32_t screen_integer(long double value){
        value=std::trunc(value);std::uint32_t low=0; //Masked-invalid FISTP QWORD indefinite has zero low DWORD.
        if(std::isfinite(value)&&value>=-0x1p63L&&value<0x1p63L)low=static_cast<std::uint32_t>(static_cast<std::uint64_t>(static_cast<std::int64_t>(value)));
        std::int32_t result;std::memcpy(&result,&low,sizeof result);return result;
    }
    Frame state_texture(std::size_t index){if(!info_.states)throw std::logic_error("HUD orders lost SAME first-state artwork table");return info_.states->at(index);}
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD auxiliary artwork is missing");}
    void show(Frame frame,bool value){require(frame);frames_.object_visible(frame,value);}
    Frames& frames_;Factory& factory_;Pointer pointer_;Info info_;Progress progress_;
};
} //namespace hd2::game
#endif
