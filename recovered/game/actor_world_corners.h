#ifndef HD2_GAME_ACTOR_WORLD_CORNERS_H
#define HD2_GAME_ACTOR_WORLD_CORNERS_H
#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>
namespace hd2::game {
namespace actor_corners_detail {
inline bool visual_kind(std::uint32_t kind,bool weapon){
    return kind==0x5f4a424fu||(!weapon&&(kind==0x50414d4cu||kind==0x4d474e53u||kind==0x524d4d53u));
}
template<class Bounds>auto corners(const Bounds& bounds){
    std::array<std::array<float,4>,8> result{};
    //006115B0/00605E50/005A85F0: Xmax first, Ymax first, Zmin first.
    //Only XYZ is written natively; zero fourth lanes keep the C++ view defined.
    std::size_t n=0;
    for(unsigned x=0;x!=2;++x)for(unsigned y=0;y!=2;++y)for(unsigned z=0;z!=2;++z)
        result[n++]={{x?bounds.minimum[0]:bounds.maximum[0],y?bounds.minimum[1]:bounds.maximum[1],z?bounds.maximum[2]:bounds.minimum[2],0}};
    return result;
}
template<class Frames>void fold(Frames& frames,typename Frames::Frame frame,const typename Frames::Matrix& parent,
                               typename Frames::Bounds& aggregate,bool weapon){
    //006112B0/00602BC0 computes LOCAL*parent before checking visual eligibility,
    //then visits descendants even when this frame is excluded.
    const auto local=frames.car_local_matrix(frame);
    const auto matrix=frames.rigid_matrix_product(local,parent);
    if(visual_kind(frames.visual_type(frame),weapon)&&(!weapon||!(frames.actor_visual_flags(frame)&0x1000u))){
        const auto bounds=frames.rigid_visual_bounds(frame);
        //Native FCOMP/test41/JP admits equality and rejects unordered values.
        if(bounds.minimum[0]<=bounds.maximum[0]){
            const auto points=corners(bounds);
            for(const auto n:{6u,2u,0u,4u,7u,3u,1u,5u}){ //Native face order, starting at minimum XYZ.
                const auto& point=points[n];
                const auto transformed=frames.car_transform(point,matrix);
                for(unsigned axis=0;axis!=3;++axis){
                    if(transformed[axis]<aggregate.minimum[axis])aggregate.minimum[axis]=transformed[axis];
                    if(aggregate.maximum[axis]<transformed[axis])aggregate.maximum[axis]=transformed[axis];
                }
            }
        }
    }
    for(auto child=frames.rigid_first_child(frame);child;child=frames.rigid_next_sibling(child))fold(frames,child,matrix,aggregate,weapon);
}
template<class Frames>auto local_bounds(Frames& frames,typename Frames::Frame frame,bool weapon){
    //006111F0/00602B20 root uses its own untransformed visual bounds. The
    //StaticWeapon1000 exclusion applies to descendants, not this root.
    typename Frames::Bounds result{};
    if(visual_kind(frames.visual_type(frame),weapon))result=frames.rigid_visual_bounds(frame);
    else{
        const std::uint32_t bits=0x5a0e1bcau;float limit;std::memcpy(&limit,&bits,4);
        for(unsigned axis=0;axis!=3;++axis){result.minimum[axis]=limit;result.maximum[axis]=-limit;}
    }
    constexpr typename Frames::Matrix identity{{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}};
    for(auto child=frames.rigid_first_child(frame);child;child=frames.rigid_next_sibling(child))fold(frames,child,identity,result,weapon);
    return result;
}
} //namespace actor_corners_detail
//Actor virtualDC. Dispatch is the confirmed loading-family vtable map; no
//unknown actor receives guessed common bounds. This borrows actual frames.
template<class Frames,class Actor>auto actor_world_corners(Frames& frames,Actor& actor){
    auto frame=actor.frame();if(!frame)throw std::runtime_error("Actor world corners require its actual frame");
    typename Frames::Bounds bounds{};
    switch(actor.type()){
    case 3: //005A85F0: EXACT Human visual "a", not whole-model aggregation.
        frame=frames.actor_find(frame,"a",1);
        if(!frame)throw std::runtime_error("Human world corners require its original visual a");
        bounds=frames.rigid_visual_bounds(frame);break;
    case 21: //00605E50.
        frames.update_scene_object(frame);bounds=actor_corners_detail::local_bounds(frames,frame,true);break;
    case 4:case 6:case 8:case 9:case 10:case 13:case 22:case 25:case 28:case 29:case 30:case 33: //006115B0.
        frames.update_scene_object(frame);bounds=actor_corners_detail::local_bounds(frames,frame,false);break;
    default:throw std::runtime_error("Actor world corners require its original virtualDC receiver");
    }
    auto result=actor_corners_detail::corners(bounds);const auto world=frames.actor_world_matrix(frame);
    for(auto& point:result)frames.car_transform_in_place(point,world); //Actual S_vector::operator*=.
    return result;
}
} //namespace hd2::game
#endif
