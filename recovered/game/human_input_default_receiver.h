#ifndef HD2_GAME_HUMAN_INPUT_DEFAULT_RECEIVER_H
#define HD2_GAME_HUMAN_INPUT_DEFAULT_RECEIVER_H
#include <cstdint>
#include <stdexcept>
namespace hd2::game {
//0072D1C0/0072D1F0 labels (absent frozen function inventory). Each default
//getter reads SAME Scene2 identity registry afresh. Native callback addresses
//are represented by the actual typed HumanInputCallbacks bundle; the record's
//first/second lane selects its actual0053B760/0053B970 method. No Car callback
//is fabricated: required004C14B0/004D0610 transport decoding stays explicit.
template<class Scene,class Callbacks>
Callbacks* human_input_default_receiver(Scene& scene,Callbacks& callbacks,std::int32_t identity,bool second){
    if(identity>31)if(auto* actor=scene.scene_actor_identity(identity)){
        if(actor->base().type()!=3){
            if(actor->base().type()==4)
                throw std::runtime_error(second?"Car input requires actual004D0610 receiver":"Car input requires actual004C14B0 receiver");
            return nullptr;
        }
    }
    return &callbacks;
}
} //namespace hd2::game
#endif
