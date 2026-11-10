#ifndef HD2_GAME_CAR_COLLISION_LOADING_H
#define HD2_GAME_CAR_COLLISION_LOADING_H
#include "game/rigid_body.h"
#include "game/car_auxiliary_geometry.h"
#include <cstring>
namespace hd2::game {
//004EA780 descriptor section,004F5620/004F5840 and004EC5C0 registration tail.
//This is the Car runtime's actual pose/descriptor owner, not an alternative
//physics service. The containing solver must update THIS pose and retain model,
//reference frames and SAME world buckets until close(). No settling is supplied.
template<class Frames,class Collisions>class CarCollisionLoading {
public:
    using Frame=typename Frames::Frame;using Matrix=typename Frames::Matrix;using Vector=typename Frames::Vector;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;using Special=typename CarAuxiliaryGeometry<Frames>::SpecialCollision;
    struct Extra {Descriptor descriptor;Frame reference{};Vector position{};Matrix inverse{};bool enabled=true;};
    CarCollisionLoading(Frames& frames,Collisions& collisions):frames_(frames),collisions_(collisions){}
    ~CarCollisionLoading(){close();}
    CarCollisionLoading(const CarCollisionLoading&)=delete;CarCollisionLoading& operator=(const CarCollisionLoading&)=delete;
    void initialize(Frame model,const typename Frames::Bounds& bounds,const std::vector<Special>& special,void* actor,std::uint32_t flags=0x40) {
        if(initialized_)throw std::logic_error("Car runtime collision initialization repeated");
        if(!model||!actor)throw std::runtime_error("Car runtime collision requires actual model/actor borrowers");
        initialized_=true;model_=model;world_=frames_.actor_world_matrix(model);inverse_=frames_.car_inverse_matrix(world_);
        main_={};secondary_={};
        for(auto* d:{&main_,&secondary_}){d->flags=flags;d->frame=model;d->actor=actor;d->matrix=&world_;d->inverse=&inverse_;d->position=origin();}
        main_.mask=0x201;main_.minimum=bounds.minimum;main_.maximum=bounds.maximum;main_.radius=radius(bounds);
        for(unsigned a=0;a!=3;++a){main_.minimum[a]=stored(static_cast<long double>(bounds.minimum[a])+.10000000149011612f);main_.maximum[a]=stored(static_cast<long double>(bounds.maximum[a])-.10000000149011612f);}
        auto lower=bounds;lower.minimum[1]=stored(static_cast<long double>(lower.minimum[1])-.5f);
        secondary_.mask=0x202;secondary_.minimum=lower.minimum;secondary_.maximum=lower.maximum;secondary_.radius=radius(lower);
        extras_.reserve(special.size());
        for(const auto& value:special){
            auto owner=std::make_unique<Extra>();auto& e=*owner;e.reference=value.reference;e.position=value.position;e.inverse=value.inverse_reference;e.enabled=value.enabled;
            auto& d=e.descriptor;d.flags=value.flags;d.mask=0x201;d.frame=model;d.actor=actor;d.minimum=value.bounds.minimum;d.maximum=value.bounds.maximum;d.radius=radius(value.bounds);
            if(e.reference){d.matrix=frames_.actor_world_matrix_pointer(e.reference);d.inverse=&e.inverse;d.position=frames_.car_transform(e.position,*d.matrix);}
            else{d.matrix=&world_;d.inverse=&inverse_;d.position=origin();}
            extras_.push_back(std::move(owner)); //Stable descriptor/inverse addresses BEFORE any bucket publication.
        }
        //004EA780 resets22C3/4/5 then004F5840(1,1); caller later chooses special mode.
        special_=first_enabled_=second_enabled_=false;set_enabled(true,true);
    }
    bool use_special(bool value) { //004F5620; empty special list genuinely returns false without changing22C3.
        require();if(value==special_)return true;if(extras_.empty())return false;
        if(!special_)collisions_.remove(main_);else for(auto n=extras_.size();n;)collisions_.remove(extras_[--n]->descriptor);
        special_=value;if(first_enabled_)publish_first();return true;
    }
    void set_enabled(bool first,bool second) { //004F5840, first group then independent secondary490.
        require();
        if(first!=first_enabled_){first_enabled_=first;
            if(!first){if(!special_)collisions_.remove(main_);else for(auto n=extras_.size();n;)collisions_.remove(extras_[--n]->descriptor);}
            else publish_first();
        }
        if(second!=second_enabled_){second_enabled_=second;
            if(second){secondary_.position=origin();collisions_.insert(secondary_);collisions_.move(secondary_);}
            else collisions_.remove(secondary_);
        }
    }
    //The solver owns the producing matrices; all descriptors borrow this SAME
    //stable pose. This does not advance physics or create a successful tick.
    void pose(const Matrix& world,const Matrix& inverse){require();world_=world;inverse_=inverse;}
    void refresh_active(){ //0050B180's600&1 descriptor/bucket refresh.
        require();if(first_enabled_){
            if(!special_){main_.position=origin();collisions_.move(main_);}
            else for(auto n=extras_.size();n;){auto& extra=*extras_[--n];auto& descriptor=extra.descriptor;
                if(extra.reference){descriptor.position=frames_.car_transform(extra.position,*descriptor.matrix);
                    extra.inverse=frames_.car_inverse_matrix(*descriptor.matrix);}
                else descriptor.position=origin();
                if(extra.enabled)collisions_.move(descriptor);
            }
        }
        if(second_enabled_){secondary_.position=origin();collisions_.move(secondary_);}
    }
    const Matrix& world()const{return world_;}const Matrix& inverse()const{return inverse_;}
    const Descriptor& main()const{return main_;}const Descriptor& secondary()const{return secondary_;}
    Descriptor& secondary(){return secondary_;}
    std::vector<Descriptor*> query_exclusions(){
        require();if(!special_)return {&main_};
        std::vector<Descriptor*> result;result.reserve(extras_.size());
        for(auto& extra:extras_)result.push_back(&extra->descriptor);
        return result; //Actual225C contains every special descriptor, including disabled ones.
    }
    const auto& extras()const{return extras_;}bool special()const{return special_;}
    void close()noexcept {
        if(!initialized_)return;
        //Failure/destructor guards retract every actually published descriptor,
        //including a failed switch/insert; never free a borrowed matrix first.
        collisions_.remove_for_cleanup(main_);collisions_.remove_for_cleanup(secondary_);
        for(auto& e:extras_)collisions_.remove_for_cleanup(e->descriptor);
        extras_.clear();model_=Frame{};initialized_=false;special_=first_enabled_=second_enabled_=false;
    }
private:
    static float stored(long double value){return static_cast<float>(value);}
    static float radius(const typename Frames::Bounds& bounds) {
        float maximum=0;
        for(unsigned corner=0;corner!=8;++corner){
            const float x=(corner&1)?bounds.maximum[0]:bounds.minimum[0],y=(corner&2)?bounds.maximum[1]:bounds.minimum[1],z=(corner&4)?bounds.maximum[2]:bounds.minimum[2];
            const float squared=stored((static_cast<long double>(x)*x+static_cast<long double>(z)*z)+static_cast<long double>(y)*y);
            if(!std::isfinite(squared))throw std::runtime_error("Car collision radius exceeds native finite geometry");
            std::int32_t word;std::memcpy(&word,&squared,4);
            const auto distance=word<0x322bcc77?0.L:std::abs(static_cast<long double>(squared)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(squared));
            if(maximum<distance)maximum=stored(distance);
        }
        return stored(static_cast<long double>(maximum)+.10000000149011612f);
    }
    Vector origin()const{return {{world_[12],world_[13],world_[14],world_[15]}};}
    void require()const{if(!initialized_)throw std::logic_error("Car collisions require actual initialized geometry/pose");}
    void publish_first() {
        if(!special_){main_.position=origin();collisions_.insert(main_);collisions_.move(main_);return;}
        for(auto n=extras_.size();n;){auto& e=*extras_[--n];auto& d=e.descriptor;
            if(e.reference){d.position=frames_.car_transform(e.position,*d.matrix);e.inverse=frames_.car_inverse_matrix(*d.matrix);}
            else d.position=origin();
            if(e.enabled){collisions_.insert(d);collisions_.move(d);}
        }
    }
    Frames& frames_;Collisions& collisions_;Frame model_{};Matrix world_{},inverse_{};Descriptor main_{},secondary_{};
    std::vector<std::unique_ptr<Extra>> extras_;bool initialized_=false,special_=false,first_enabled_=false,second_enabled_=false;
};
}
#endif
