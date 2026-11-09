#ifndef HD2_GAME_CAR_ANCILLARY_MODEL_H
#define HD2_GAME_CAR_ANCILLARY_MODEL_H

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace hd2::game {

//004E19C0: first case-sensitive substring, then atoi after the key. A missing
//key/frame returns -1; a present nonnumeric suffix returns0.
inline std::int32_t car_attribute_number(const std::string& text,const std::string& key) {
    const auto offset=text.find(key);
    if(offset==std::string::npos)return -1;
    const auto value=std::strtoll(text.c_str()+offset+key.size(),nullptr,10);
    if(value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())
        throw std::runtime_error("Car numeric attribute overflow");
    return static_cast<std::int32_t>(value);
}

//004E0930/004E09D0's DM model and metadata owners. The real optional load is
//attempted; a missing/rejected model leaves this owner empty as original.
template<class Frames>
class CarAncillaryModel {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().create_car_ancillary_model());
    struct Zone {Frame frame{};std::int32_t hit_points=-1,material=-1;};
    struct Explosion {Frame frame{};std::int32_t delay=0,part=-1;};
    void load(Frames& frames,const std::string& resource,std::size_t seat_count) {
        if(model_)throw std::logic_error("Car ancillary model loaded twice");
        auto model=frames.create_car_ancillary_model();
        if(!model)return;
        if(!frames.load_car_ancillary_model(model.get(),"DM"+resource))return;
        model_.emplace(std::move(model));
        for(std::size_t seat=0;seat<seat_count;++seat) {
            const auto number=std::to_string(seat);
            const auto name=std::string("SEAT")+(number.size()<2?"0":"")+number;
            seats_.push_back(frames.car_find(model_->get(),name.c_str(),1));
        }
        const auto children=frames.car_model_frames(model_->get());
        for(auto frame=children.rbegin();frame!=children.rend();++frame) {
            const auto name=frames.car_short_name(*frame);
            if(name.find("HOR")!=std::string::npos||name.find("VERT")!=std::string::npos) {
                planes_.push_back(*frame);
            } else if(name.find("D_ZONE")!=std::string::npos) {
                const auto user=frames.car_user_data(*frame);
                zones_.push_back({*frame,car_attribute_number(user,"HP"),car_attribute_number(user,"matid")});
            } else if(name.find("EXPL_D")!=std::string::npos) {
                const auto user=frames.car_user_data(*frame);
                const auto bits=static_cast<std::uint32_t>(car_attribute_number(user,"EXPL"))*1000u;
                std::int32_t delay;std::memcpy(&delay,&bits,sizeof(delay));
                explosions_.push_back({*frame,delay,car_attribute_number(user,"PARTID")});
            }
        }
        particle_=frames.car_find(model_->get(),"particle",0x10020);
        if(particle_) {
            const auto user=frames.car_user_data(particle_);
            sound_=car_attribute_number(user,"SID");fire_=car_attribute_number(user,"PFIRE");smoke_=car_attribute_number(user,"PSMOKE");
        }
        if(fire_==-1)fire_=0x60;
        if(smoke_==-1)smoke_=0x5f;
        frames.object_visible(model_->get(),false);
    }
    Frame model() const {return model_?model_->get():Frame{};}
    const std::vector<Frame>& seats() const {return seats_;}
    const std::vector<Frame>& planes() const {return planes_;}
    const std::vector<Zone>& zones() const {return zones_;}
    const std::vector<Explosion>& explosions() const {return explosions_;}
    Frame particle() const {return particle_;}
    std::int32_t sound() const {return sound_;}
    std::int32_t fire() const {return fire_;}
    std::int32_t smoke() const {return smoke_;}
private:
    std::optional<Owner> model_;
    std::vector<Frame> seats_,planes_;
    std::vector<Zone> zones_;
    std::vector<Explosion> explosions_;
    Frame particle_{};
    std::int32_t sound_=-1,fire_=-1,smoke_=-1;
};

} //namespace hd2::game
#endif
