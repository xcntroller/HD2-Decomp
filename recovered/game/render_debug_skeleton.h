#ifndef HD2_GAME_RENDER_DEBUG_SKELETON_H
#define HD2_GAME_RENDER_DEBUG_SKELETON_H
#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
namespace hd2::game {
//0079B760's process buffers0098AF90/94. No retained frame/actor borrowers.
class RenderDebugSkeleton {
    struct alignas(16) Vertex{std::array<float,4> position;};
    static_assert(sizeof(Vertex)==16);
public:
    std::uint32_t colour=0x00ff00ffu; //Actual00863620 bytes FF00FF00.
    void clear()noexcept{vertices_.reset();indices_.reset();}
private:
    template<class Geometry,class Frames>void append(const Geometry& geometry,Frames& frames,std::uint16_t& count){ //0079AC40.
        if(!geometry.model())return;
        constexpr std::array<std::array<unsigned,2>,20> pairs{{
            {22,9},{9,13},{13,8},{8,4},{4,5},{5,6},{8,14},{14,16},{16,18},{18,20},
            {8,15},{15,17},{17,19},{19,21},{22,11},{11,3},{3,1},{22,10},{10,2},{2,0}}};
        if(count+40u>920u)throw std::runtime_error("Debug skeleton exceeds original buffers");
        for(const auto& pair:pairs){
            //Read/update both frames for EACH line, including repeated endpoints.
            for(unsigned end=0;end!=2;++end){
                const auto frame=geometry.skeleton().bones().at(pair[end]).frame;
                if(!frame)throw std::runtime_error("Debug skeleton requires its actual bone frame");
                const auto matrix=frames.car_world_matrix(frame);
                vertices_[count+end].position={matrix[12],matrix[13],matrix[14],matrix[15]};
                indices_[count+end]=static_cast<std::uint16_t>(count+end);
            }
            count=static_cast<std::uint16_t>(count+2);
        }
    }
public:
    template<class Services>void render(Services& services){
        //Original allocates lazily even if the registry is empty; keep partial
        //allocation available for retry and free on process shutdown.
        if(!vertices_)vertices_=std::make_unique<Vertex[]>(0x3980/sizeof(Vertex));
        if(!indices_)indices_=std::make_unique<std::uint16_t[]>(0x730/sizeof(std::uint16_t));
        std::uint16_t count=0;unsigned records=0;
        for(auto n=services.debug_actor_count();n;){
            if(services.visit_debug_geometry(--n,[&](const auto& geometry,auto& frames){append(geometry,frames,count);}))
                if(++records>20)break; //Original accepts21 qualifying records.
        }
        if(!count)return;
        services.debug_identity_transform();
        const std::array<float,3> rgb{
            static_cast<float>(static_cast<long double>((colour>>16)&255u)*.003921568859368563f),
            static_cast<float>(static_cast<long double>((colour>>8)&255u)*.003921568859368563f),
            static_cast<float>(static_cast<long double>(colour&255u)*.003921568859368563f)};
        services.debug_draw_lines(vertices_.get(),count,indices_.get(),rgb,colour>>24);
    }
private:
    std::unique_ptr<Vertex[]> vertices_;
    std::unique_ptr<std::uint16_t[]> indices_;
};
}
#endif
