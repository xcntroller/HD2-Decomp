#ifndef HD2_GAME_HUMAN_AI_ORDER_MARKER_H
#define HD2_GAME_HUMAN_AI_ORDER_MARKER_H
#include "game/human_ai_order_pool.h"
#include "game/rigid_body.h"

namespace hd2::game {
//0058C5A0/0058D6C0. SAME actual HUD marker and dynamic buckets. Destruction
//unregisters the descriptor before clearing EC/returning the model; error
//reporting uses the existing frame cleanup channel, never successful omission.
template<class Frames,class Collisions,class Command,class Order>
class HumanAIOrderMarker final:public HumanAIOrderMarkerLifetime<typename Frames::Frame> {
public:
    using Frame=typename Frames::Frame;using Owner=typename Command::Owner;using Matrix=std::array<float,16>;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    HumanAIOrderMarker(Frames& frames,Collisions& collisions,Command& command,Order& order)
        :frames_(frames),collisions_(collisions),command_(command),order_(order){}
    ~HumanAIOrderMarker()override{clear();}
    //Acquire completes004B9240 BEFORE the caller publishes84. Initialization
    //then runs with the marker visible through the actual order owner.
    void acquire(){model_=command_.acquire_order_marker(order_);if(!model_)throw std::runtime_error("AI order requires loaded HUD marker template748");}
    Frame frame()const override{return model_?model_->get():Frame{};}
    const Descriptor& descriptor()const{return descriptor_;}
    void initialize(){
        const auto model=frame();if(!model)throw std::logic_error("AI order marker has no actual model84");
        frames_.human_order_marker_data(model,&order_);
        const auto position=order_.state.template get<HumanVector>(0x30);
        frames_.hud_position(model,{{position[0],position[1],position[2]}});
        frames_.human_order_marker_link(model);frames_.update_scene_object(model);
        descriptor_={};descriptor_.flags=0x40;descriptor_.position=position;
        descriptor_.minimum={{-.2f,0,-.2f,0}};descriptor_.maximum={{.2f,1.6f,.2f,100000.f}};
        descriptor_.radius=1.6f;descriptor_.actor=&order_;
        descriptor_.matrix=frames_.actor_world_matrix_pointer(model); //First fresh world read is a borrower.
        const auto world=frames_.actor_world_matrix(model); //Second fresh read copied BEFORE inverse.
        inverse_=frames_.car_inverse_matrix(world);descriptor_.inverse=&inverse_;
        descriptor_.frame=frame();descriptor_.mask=0x8000;
        collisions_.insert(descriptor_);
    }
    void update()override{command_.update_order_marker(frame(),order_);} //0058D720's004B9B10; no position move here.
    void retire()noexcept override{clear();}
private:
    void clear()noexcept{
        if(!model_)return;
        collisions_.remove_for_cleanup(descriptor_);
        try{frames_.human_order_marker_data(frame(),nullptr);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
        try{command_.recycle_order_marker(*model_);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
        model_.reset();descriptor_={};
    }
    Frames& frames_;Collisions& collisions_;Command& command_;Order& order_;
    std::optional<Owner> model_;Descriptor descriptor_;Matrix inverse_{};
};
template<class Order,class Frames,class Collisions,class Command>
void initialize_human_ai_order_marker(Order& order,Frames& frames,Collisions& collisions,Command& command){
    using Marker=HumanAIOrderMarker<Frames,Collisions,Command,Order>;
    auto marker=std::make_unique<Marker>(frames,collisions,command,order);marker->acquire();
    auto* current=marker.get();order.marker=std::move(marker);current->initialize(); //Native84 published before EC/position/link.
}
}
#endif
