#ifndef HD2_GAME_MESH_MOVIE_H
#define HD2_GAME_MESH_MOVIE_H

#include "game/serialized_bytes.h"
#include "frontend/cleanup.h"
#include <algorithm>
#include <optional>
#include <utility>

namespace hd2::game {

//00484260/004847A0/00484AE0. The actual movie texture replaces every LOD0
//material texture of eligible visuals; original textures are retained/restored.
//Services owns checked video/material/frame endpoints. Scene and graphics must
//outlive this owner. It is separate from full-screen movie playback.
template<class Services>
class MeshMovie {
public:
    using Frame=typename Services::Frame;
    using MovieOwner=decltype(std::declval<Services&>().create_mesh_movie());
    using TextureOwner=decltype(std::declval<Services&>().retain_mesh_movie_texture(std::declval<Frame>()));
    explicit MeshMovie(Services& services) : services_(services) {}
    ~MeshMovie() {try{clear();}catch(...){services_.record_actor_cleanup_failure(std::current_exception());}}
    MeshMovie(const MeshMovie&)=delete;
    MeshMovie& operator=(const MeshMovie&)=delete;
    const std::string& resource() const {return resource_;}
    std::size_t size() const {return bindings_.size();}
    bool read(SerializedBytes bytes) {
        clear();auto root=next_mission_chunk(bytes);
        if(!root)return false;
        std::string resource;std::vector<Frame> frames;
        while(auto field=next_mission_chunk(root->bytes)) {
            if(field->id==0x44c)resource=text(field->bytes);
            else if(field->id==0x4b0) {
                auto frame=services_.find_scene_frame(text(field->bytes),0xffff);
                if(frame)frames.push_back(frame);
            }
        }
        return initialize(frames,resource); //Original performs this even after missing/invalid file open.
    }
    template<class Files>bool load(Files& files,const std::string& resource) {
        clear();const auto path=std::string("missions\\")+resource+"\\meshplayer.bin";
        const auto handle=files.open(path.c_str(),0);if(handle==0xffffffffu)return false;
        struct Close {Files& files;std::uint32_t handle;~Close(){files.close(handle);}} close{files,handle};
        const auto size=files.seek(handle,0,2);
        if(size==0xffffffffu||size>128u*1024u*1024u)throw std::runtime_error("Invalid mesh-player file size");
        if(files.seek(handle,0,0)!=0)throw std::runtime_error("Cannot rewind mesh player");
        std::vector<unsigned char> bytes(size);
        if(size&&files.read(handle,bytes.data(),size)!=size)throw std::runtime_error("Cannot read mesh player");
        return read(SerializedBytes(bytes));
    }
    bool initialize(const std::vector<Frame>& frames,const std::string& resource) {
        clear();if(frames.empty())return false;
        movie_.emplace(services_.create_mesh_movie());
        if(!movie_->get())throw std::runtime_error("Actual mesh movie factory returned null");
        if(services_.open_mesh_movie(movie_->get(),"GameMovie\\"+resource,0)!=0) {movie_.reset();return false;}
        bindings_.reserve(frames.size());
        try {
            for(auto frame:frames)if(services_.mesh_movie_visual(frame)) {
                auto texture=services_.retain_mesh_movie_texture(services_.mesh_movie_original_texture(frame));
                //Publish before changing materials so failure still restores the
                //retained original texture, including an original null texture.
                bindings_.push_back({frame,std::move(texture)});
                services_.set_mesh_movie_texture(frame,services_.mesh_movie_texture(movie_->get()));
            }
            services_.play_mesh_movie(movie_->get());resource_=resource;return true;
        } catch(...) {
            frontend::CleanupErrors errors(std::current_exception());errors.attempt([&]{clear();});errors.rethrow();
        }
        return false;
    }
    void update() { //00484A70: stack delta is ignored; current visibility epochs gate decoding.
        if(!movie_)return;
        if(services_.mesh_movie_status(movie_->get())==1)services_.play_mesh_movie(movie_->get());
        for(auto i=bindings_.size();i!=0;) {
            const auto frame=bindings_[--i].frame;
            //Read the scene epoch afresh for each original reverse traversal.
            const auto frame_epoch=services_.mesh_movie_frame_epoch(frame);
            if(frame_epoch==services_.mesh_movie_scene_epoch()) {
                services_.update_mesh_movie_texture(movie_->get(),false);return;
            }
        }
    }
    void clear() {
        frontend::CleanupErrors errors;
        if(movie_) {
            for(auto i=bindings_.size();i!=0;) {
                auto& binding=bindings_[--i];
                errors.attempt([&]{services_.set_mesh_movie_texture(binding.frame,binding.texture.get());});
                binding.texture.reset();
            }
            errors.attempt([&]{services_.stop_mesh_movie(movie_->get());});movie_.reset();
        }
        bindings_.clear();resource_.clear();errors.rethrow();
    }
private:
    struct Binding {Frame frame;TextureOwner texture;};
    static std::string text(SerializedBytes& bytes) {
        return bytes.section(std::min<std::size_t>(bytes.remaining(),999)).text();
    }
    Services& services_;
    std::optional<MovieOwner> movie_;
    std::vector<Binding> bindings_;
    std::string resource_;
};

} //namespace hd2::game
#endif
