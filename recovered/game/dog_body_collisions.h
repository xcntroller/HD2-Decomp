#ifndef HD2_GAME_DOG_BODY_COLLISIONS_H
#define HD2_GAME_DOG_BODY_COLLISIONS_H
#include "game/rigid_body.h"
#include "game/dog_body_orders.h"
#include <optional>
namespace hd2::game {
template<class Frame>class DogBodyCollisionLifetime {
public:
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;using Vector=std::array<float,4>;
    virtual ~DogBodyCollisionLifetime()=default;
    virtual void update()=0;
    virtual Descriptor* descriptor(unsigned)=0;
    virtual void remove(unsigned,bool required)=0;
    virtual const Vector& anchor(unsigned)const=0;
    virtual float radius(unsigned)const=0;
};
//The005C7D30 collision portion,005C1DC0 anchor producer and005C8B30's
//post-model descriptor refresh. Body=Actor+90, first two1DC/1E0 are physical,
//last three1E4..1EC are material66 damage targets. All borrow the SAME actor,
//model and live grid; this owner alone does not enable Dog activation.
template<class Frames,class Collisions,class Actor>
class DogBodyCollisions final : public DogBodyCollisionLifetime<typename Frames::Frame> {
public:
    using Frame=typename Frames::Frame;using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    using Vector=std::array<float,4>;
    DogBodyCollisions(Frames& frames,Collisions& collisions,Actor& actor)
        :frames_(frames),collisions_(collisions),actor_(actor){
        try{
            constexpr std::array<const char*,3> names{{"col dummy1","col dummy2","col dummy"}};
            for(unsigned n=0;n!=3;++n){dummy_[n]=frames_.actor_find(actor_.base().frame(),names[n],0xffffu);if(dummy_[n]&&frames_.frame_kind(dummy_[n])!=6)dummy_[n]=Frame{};}
            refresh_anchors();
            for(unsigned n=0;n!=5;++n){
                auto owner=std::make_unique<Descriptor>();auto& d=*owner;
                d.flags=n<2?0x42u:0x42000042u;d.mask=n<2?0x100001u:0x100002u;
                d.frame=actor_.base().frame();d.actor=&actor_;populate(d,n);
                //Publish the pointer BEFORE registration, as the original;
                //catch cleanup prevents a partially inserted dangling borrower.
                descriptors_[n]=std::move(owner);collisions_.insert(*descriptors_[n]);
            }
        }catch(...){clear();throw;}
    }
    ~DogBodyCollisions()override{clear();}
    Descriptor* descriptor(unsigned n)override{return descriptors_.at(n).get();}
    const Vector& anchor(unsigned n)const override{const auto& value=centers_.at(n);if(!value)throw std::runtime_error("Dog motion requires actual collision anchor producer");return *value;}
    float radius(unsigned n)const override{const auto& value=radii_.at(n);if(!value)throw std::runtime_error("Dog motion requires actual collision radius producer");return *value;}
    const auto& dummies()const{return dummy_;}
    const auto& centers()const{return centers_;}
    const auto& radii()const{return radii_;}
    void remove(unsigned n,bool required)override{
        auto& owner=descriptors_.at(n);
        if(!owner){if(required)throw std::runtime_error("Dog death reaches native NULL body descriptor");return;}
        collisions_.remove(*owner);owner.reset(); //Actual remove returns BEFORE free/null.
    }
    void clear()noexcept{
        for(auto& owner:descriptors_)if(owner){collisions_.remove_for_cleanup(*owner);owner.reset();}
    }
    void refresh_anchors(){ //005C1DC0 deliberately leaves missing dummy outputs untouched.
        for(unsigned n=0;n!=3;++n)if(dummy_[n]){
            const auto matrix=frames_.actor_world_matrix(dummy_[n]);
            centers_[n]=Vector{{matrix[12],static_cast<float>(static_cast<long double>(matrix[13])-.10000000149011612f),matrix[14],0}};
            const auto bounds=frames_.car_dummy_bounds(dummy_[n]);
            radii_[n]=static_cast<float>((static_cast<long double>(bounds.maximum[0])-bounds.minimum[0])*.3499999940395355f);
        }
    }
    void update()override{ //Called only after the original containing model/sector/24 operations.
        refresh_anchors();
        if(dog_order_signed(actor_.properties.fields4a0[0])>0)for(unsigned n=0;n!=2;++n)move(n);
        for(unsigned n=2;n!=5;++n)move(n);
    }
private:
    void populate(Descriptor& d,unsigned n){
        const auto index=n<2?n:n-2;
        if(!centers_[index]||!radii_[index])throw std::runtime_error("Dog body collision requires actual dummy centre/radius producer");
        d.position=*centers_[index]; //Native XYZ only; undefined W has no claimed value.
        if(n>=2)d.position[1]=static_cast<float>(static_cast<long double>(d.position[1])+.2199999988079071f);
        d.radius=static_cast<float>(static_cast<long double>(*radii_[index])*(n<2?1.649999976158142f:2.5f));
    }
    void move(unsigned n){if(auto& owner=descriptors_[n]){populate(*owner,n);collisions_.move(*owner);}}
    Frames& frames_;Collisions& collisions_;Actor& actor_;
    std::array<Frame,3> dummy_{};
    std::array<std::optional<Vector>,3> centers_;
    std::array<std::optional<float>,3> radii_;
    std::array<std::unique_ptr<Descriptor>,5> descriptors_;
};
//005C83A0: two physical THEN three damage descriptors; all three voices stop
//before their final-1 publication. Destruction fallback is separate from this
//normal retirement path and must not hide the first required callback failure.
template<class Geometry,class Actions,class Receivers>
void retire_dog_body(Geometry& geometry,Actions& actions,Receivers& receivers){
    for(unsigned n=0;n!=5;++n)geometry.remove(n,false);
    if(actions.voice37c!=-1)receivers.dog_stop_body_voice(actions.voice37c);
    if(actions.voice380!=-1)receivers.dog_stop_body_voice(actions.voice380);
    if(actions.voice384!=-1)receivers.dog_stop_body_voice(actions.voice384);
    actions.voice380=actions.voice384=actions.voice37c=-1;
}
}
#endif
