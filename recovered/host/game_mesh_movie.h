#ifndef HD2_HOST_GAME_MESH_MOVIE_H
#define HD2_HOST_GAME_MESH_MOVIE_H

#include "host/game_actor_services.h"
#include "game/mesh_movie.h"

namespace hd2::host {

class GameMeshMovieServices : public GameActorServices {
public:
    using GameActorServices::GameActorServices;
    struct MovieRelease {
        HMODULE module=nullptr;
        void operator()(Frame movie) const noexcept {
            if(movie) {
                using Release=std::int32_t(WINAPI*)(Frame);
                scene_method<Release>(movie,module,4,0x70fd0)(movie);
            }
        }
    };
    using MovieOwner=std::unique_ptr<EngineFrame,MovieRelease>;
    using TextureOwner=GameCarFrames::ResourceOwner;
    MovieOwner create_mesh_movie() {
        using Factory=Frame(WINAPI*)(Frame);
        auto* graphics=reinterpret_cast<Frame>(&graphics_);
        auto* movie=scene_method<Factory>(graphics,module_,0x38,0x69e80)(graphics);
        if(!movie)throw std::runtime_error("Actual mesh movie factory returned null");
        //IShow release is+04 (10070FD0), unlike texture/frame release+00.
        using Release=std::int32_t(WINAPI*)(Frame);
        scene_method<Release>(movie,module_,4,0x70fd0);
        return MovieOwner(movie,MovieRelease{module_});
    }
    std::int32_t open_mesh_movie(Frame movie,const std::string& path,std::uint32_t flags) {
        using Open=std::int32_t(WINAPI*)(Frame,const char*,std::uint32_t);
        return scene_method<Open>(movie,module_,0xc,0x71630)(movie,path.c_str(),flags);
    }
    void play_mesh_movie(Frame movie) {
        using Play=std::int32_t(WINAPI*)(Frame);
        scene_method<Play>(movie,module_,0x14,0x71c50)(movie);
    }
    void rewind_mesh_movie(Frame movie){ //004AD380, IShow virtual30(0).
        using Seek=std::int32_t(WINAPI*)(Frame,std::uint32_t);
        checked_engine_method<Seek>(movie,module_,0x30)(movie,0);
    }
    void loop_mesh_movie(Frame movie,bool enabled){call<void>(movie,0x30,static_cast<std::uint32_t>(enabled));} //004AD380's IShow+30.
    void stop_mesh_movie(Frame movie) {
        using Stop=std::int32_t(WINAPI*)(Frame);
        scene_method<Stop>(movie,module_,0x18,0x71d10)(movie);
    }
    void close_mesh_movie_stream(Frame movie) { //004AD310 explicitly closes after stop, before IShow+04 release.
        using Close=std::int32_t(WINAPI*)(Frame);
        scene_method<Close>(movie,module_,0x10,0x71b80)(movie);
    }
    std::int32_t mesh_movie_status(Frame movie) {
        using Status=std::int32_t(WINAPI*)(Frame);
        return scene_method<Status>(movie,module_,0x2c,0x72160)(movie);
    }
    void update_mesh_movie_texture(Frame movie,bool force) {
        //100725D0 waits/decodes, uploads the texture, then advances Bink. The
        //false argument preserves timing and the original last-frame stop.
        using Update=std::int32_t(WINAPI*)(Frame,std::uint8_t);
        scene_method<Update>(movie,module_,0x4c,0x725d0)(movie,static_cast<std::uint8_t>(force));
    }
    std::uint32_t mesh_movie_frame_epoch(Frame frame) const {return field<std::uint32_t>(frame,0x11c);}
    std::uint32_t mesh_movie_scene_epoch() const {return field<std::uint32_t>(scene_.frame(),0x11c);}
    Frame mesh_movie_texture(Frame movie) {
        using Texture=Frame*(WINAPI*)(Frame);
        auto** texture=scene_method<Texture>(movie,module_,0x48,0x725b0)(movie);
        if(!texture)throw std::runtime_error("Mesh movie has no texture pointer owner");
        return *texture;
    }
    bool mesh_movie_visual(Frame frame) const {
        return frame_kind(frame)==1&&(field<std::uint32_t>(frame,0x1d0)&0x800)!=0;
    }
    TextureOwner retain_mesh_movie_texture(Frame texture) {
        if(texture) {
            const auto count=field<std::uint32_t>(texture,0x14);
            if(count==0xffffffffu)throw std::runtime_error("Mesh movie texture reference overflow");
            put(texture,0x14,count+1);
        }
        return TextureOwner(texture,ResourceRelease{module_});
    }
    Frame mesh_movie_original_texture(Frame frame) {
        auto* visual_mesh=mesh(frame);if(!visual_mesh)return nullptr;
        auto* lod=call<Frame>(visual_mesh,4,std::uint32_t{0});if(!lod)return nullptr;
        //Original repeats the+04(0) lookup before following first group+5C.
        lod=call<Frame>(visual_mesh,4,std::uint32_t{0});
        if(!field<Frame>(lod,0x5c))return nullptr;
        lod=call<Frame>(visual_mesh,4,std::uint32_t{0});
        auto* material=field<Frame>(field<Frame>(lod,0x5c),4);if(!material)return nullptr;
        if(field<std::uint32_t>(material,8)&0x04000000u)call<void>(material,0x44);
        return field<Frame>(material,0xc);
    }
    void set_mesh_movie_texture(Frame frame,Frame texture) {
        auto* visual_mesh=mesh(frame);if(!visual_mesh)return;
        auto* lod=call<Frame>(visual_mesh,4,std::uint32_t{0});if(!lod)return;
        auto count=field<std::uint32_t>(lod,0x58);
        while(count) {
            --count;
            auto* group=call<Frame>(lod,0x14,count);
            if(group) {
                group=call<Frame>(lod,0x14,count);
                if(field<Frame>(group,4)) {
                    group=call<Frame>(lod,0x14,count);
                    call<void>(field<Frame>(group,4),0x1c,texture);
                }
            }
        }
    }
private:
    Frame mesh(Frame frame) {
        const auto type=visual_type(frame);
        switch(type) {
        case 0x50414d4c:case 0x4850524d:case 0x4d474e53:case 0x524d4d53:case 0x5f4a424f:
            return field<Frame>(frame,0x1e0);
        default:return nullptr;
        }
    }
};

} //namespace hd2::host
#endif
