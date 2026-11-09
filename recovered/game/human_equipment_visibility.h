#ifndef HD2_GAME_HUMAN_EQUIPMENT_VISIBILITY_H
#define HD2_GAME_HUMAN_EQUIPMENT_VISIBILITY_H

#include "game/human_equipment_state.h"
#include "game/effects_shadow_models.h"
#include "frontend/pre_mission_data.h"

namespace hd2::game {

//007920B0: temporary canonical equipment names feed the actual shadow builder,
//then original names/visibility restore before the final shadow-mode decision.
//Services supplies actual identity lookup, player/view state and DD0 mode/1D8.
//A found item is its real RuntimeItem, never a decoded competing catalogue.
template<class Frames,class Shadow,class Services>
class HumanEquipmentVisibility {
public:
    using Frame=typename Frames::Frame;
    HumanEquipmentVisibility(Frames& frames,Shadow& shadow,Services& services,
            const frontend::PreMissionItemCatalogue& catalogue,HumanEquipmentState<Frame>& state)
        :frames_(frames),shadow_(shadow),services_(services),catalogue_(catalogue),state_(state) {}
    void refresh(Frame human_model) {
        if(!human_model)throw std::runtime_error("Equipment visibility requires the actual Human model");
        struct Restore {
            HumanEquipmentVisibility& owner;Frame human;std::array<std::string,3> names{};
            std::array<bool,3> renamed{};bool visible=false,visibility_changed=false,armed=true;
            ~Restore(){if(armed)try{finish(false);}catch(...){owner.frames_.record_actor_cleanup_failure(std::current_exception());}}
            void finish(bool success) {
                armed=false;frontend::CleanupErrors errors;
                constexpr std::array<unsigned,3> models{{0,4,3}};
                for(unsigned i=0;i!=3;++i)if(success||renamed[i])if(const auto frame=owner.state_.model(models[i]))
                    errors.attempt([&]{owner.frames_.human_equipment_name(frame,names[i]);});
                if(visibility_changed)errors.attempt([&]{owner.frames_.object_visible(human,visible);});
                errors.rethrow();
            }
        } restore{*this,human_model};
        const auto rename=[&](unsigned index,Frame frame,const char* name) {
            restore.renamed[index]=true;frames_.human_equipment_name(frame,name);
        };
        if(const auto weapon=state_.model(0)) {
            const auto id=state_.words().template get<std::int32_t>(0x34);
            if(id>=0) {
                restore.names[0]=frames_.frame_name(weapon);
                if(const auto* definition=catalogue_.item(id))switch(definition->kind()) {
                case 0:case 1:case 2:rename(0,weapon,"sm");break;
                case 3:rename(0,weapon,"me");break;
                case 4:case 5:case 7:case 12:rename(0,weapon,"lo");break;
                case 6:rename(0,weapon,"panzer");break;
                default:break;
                }
            }
        }
        if(const auto bag=state_.model(4))if(const auto* definition=catalogue_.item(state_.words().template get<std::int32_t>(0x38))) {
            restore.names[1]=frames_.frame_name(bag);
            const auto& payload=definition->payloads[0];
            if(payload_kind(payload)==7) {
                const auto scalar=frontend::pre_mission_detail::Bytes::real(payload_word(payload,0x0c)); //Native+10.
                rename(1,bag,scalar<=30.f?"smlbag":"bigbag"); //0080FA08; unordered chooses bigbag.
            }
        }
        const auto identity=state_.words().template get<std::int32_t>(0x44);
        if(state_.model(3)&&identity>=0)if(auto* item=services_.equipment_item(identity)) {
            const auto model=item->model();
            if(!model)throw std::runtime_error("Head equipment identity has no actual model");
            restore.names[2]=frames_.frame_name(model);
            const auto& definition=item->definition();const auto& payload=definition.payloads[0];
            if(payload_kind(payload)==8)switch(payload_word(payload,8)) { //Native+0C.
            case 0:rename(2,model,"helm");break;
            case 1:rename(2,model,"brig");break;
            case 2:rename(2,model,"lod");break;
            default:break;
            }
        }
        restore.visible=frames_.car_frame_visible(human_model);
        const bool visible=restore.visible||services_.equipment_visibility_override();
        restore.visibility_changed=true;frames_.object_visible(human_model,true);
        shadow_.refresh(services_.mode_flags(),human_model);
        restore.finish(true);
        bool show=false;
        if(visible) {
            const auto mode=services_.equipment_mode();
            show=mode<1||(mode>5&&services_.equipment_animation_identity()!=159);
        }
        if(const auto shadow=shadow_.frame())frames_.object_visible(shadow,show);
    }
private:
    static std::uint32_t payload_word(const frontend::PreMissionItemPayload& payload,unsigned offset) {
        if(offset>payload.bytes.size()||payload.bytes.size()-offset<4)throw std::runtime_error("Truncated equipment definition payload");
        return frontend::pre_mission_detail::Bytes::word(payload.bytes.data()+offset);
    }
    static std::int32_t payload_kind(const frontend::PreMissionItemPayload& payload) {
        if(!payload.type)return -1;
        if(payload.type==10&&payload.bytes.empty())return 9;
        return frontend::pre_mission_detail::Bytes::integer(payload_word(payload,4));
    }
    Frames& frames_;Shadow& shadow_;Services& services_;
    const frontend::PreMissionItemCatalogue& catalogue_;HumanEquipmentState<Frame>& state_;
};
} //namespace hd2::game
#endif
