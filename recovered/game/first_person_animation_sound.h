#ifndef HD2_GAME_FIRST_PERSON_ANIMATION_SOUND_H
#define HD2_GAME_FIRST_PERSON_ANIMATION_SOUND_H
#include "frontend/pre_mission_data.h"
#include "frontend/spatial_sound.h"
namespace hd2::game {
//004933C0. Sound must supply actual shared SOUND+54 emission, not an optional
//notification. Player is the SAME view38 borrower, represented by Models.
template<class View,class Models,class Camera,class Sound>
class FirstPersonAnimationSound {
public:
    FirstPersonAnimationSound(View& view,Models& models,const frontend::PreMissionItemCatalogue& catalogue,Camera& camera,Sound& sound)
        :view_(view),models_(models),catalogue_(catalogue),camera_(camera),sound_(sound) {}
    void first_person_animation(std::int32_t animation) {
        if((!view_.bytes58[0]&&!view_.bytes58[1])||animation<2)return;
        if(animation>5&&animation!=8)return;
        const auto* definition=catalogue_.item(models_.definition());
        if(animation<6&&(!definition||definition->kind()!=9))return;
        if(!definition)throw std::runtime_error("First-person reload sound has no actual item definition");
        frontend::SpatialSoundRequest request;request.flags=0x41;request.relative=nullptr;request.source=models_.player();
        if(animation==8) {request.group=3;request.entry=frontend::pre_mission_detail::Bytes::integer(definition->fields38[2]);}
        else {
            request.group=2;
            const auto& payload=definition->payloads[0];
            //Native48 virtual18 is007E0290 for actual tags1..4: native14,
            //serialized payload10 because serialization starts at native04.
            if(payload.type<1||payload.type>4||payload.bytes.size()<0x14)
                throw std::runtime_error("First-person shot sound requires the actual weapon payload getter");
            request.entry=frontend::pre_mission_detail::Bytes::integer(frontend::pre_mission_detail::Bytes::word(payload.bytes.data()+0x10));
        }
        (void)camera_.query_vector(0,request.position); //Native ignores false; constructor position stays intact.
        (void)sound_.emit_animation_sound(request);
    }
private:
    View& view_;Models& models_;const frontend::PreMissionItemCatalogue& catalogue_;Camera& camera_;Sound& sound_;
};
} //namespace hd2::game
#endif
