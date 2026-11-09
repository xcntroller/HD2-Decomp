#ifndef HD2_GAME_CAR_TYRES_H
#define HD2_GAME_CAR_TYRES_H

#include "game/serialized_bytes.h"
#include <array>
#include <cmath>
#include <utility>

namespace hd2::game {

struct CarTyreSurface {
    std::uint32_t id;
    std::array<float,12> coefficients;
};
struct CarTyreType {
    std::uint32_t id;
    std::string name;
    std::vector<CarTyreSurface> surfaces;
};

//004D6900 reads tables\tyres.bin, then004EC980 replaces the global tyre
//registry with a develop record followed by a deep copy of the loaded types.
//004D6BF0 converts the first12 serialized floats of each surface; later bytes
//in that length-prefixed payload do not participate. IDs are integers, not
//floats, and duplicate tyre IDs remain in file order (last wins in004ED270).
class CarTyres {
public:
    CarTyres() { clear(); }
    void clear() { types_={{0,"develop",{}}}; } //004ECCD0/004EC980.
    const std::vector<CarTyreType>& types() const { return types_; }
    std::vector<std::int32_t> material_ids() const {
        std::vector<std::int32_t> result;
        result.reserve(types_.size());
        for(const auto& type:types_){std::int32_t id;std::memcpy(&id,&type.id,4);result.push_back(id);}
        return result;
    }
    bool read(SerializedBytes bytes) {
        clear(); //Original installs the develop entry before opening/reading.
        if(bytes.word()!=1)return false;
        const auto count=bytes.word();
        if(count>bytes.remaining()/44)throw std::runtime_error("Truncated tyre type table");
        std::vector<CarTyreType> loaded;
        loaded.reserve(std::size_t(count)+1);
        loaded.push_back({0,"develop",{}});
        for(std::uint32_t i=0;i!=count;++i) {
            bytes.word(); //Original reads this record marker without testing it.
            auto name=bytes.section(32).text();
            const auto id=bytes.word(),surfaces=bytes.word();
            if(surfaces>bytes.remaining()/56)throw std::runtime_error("Truncated tyre surface table");
            CarTyreType type{id,std::move(name),{}};
            type.surfaces.reserve(surfaces);
            for(std::uint32_t surface=0;surface!=surfaces;++surface) {
                const auto surface_id=bytes.word();
                auto payload=bytes.section(bytes.word());
                CarTyreSurface values{surface_id,{}};
                for(std::size_t field=0;field!=values.coefficients.size();++field) {
                    const auto value=payload.real();
                    if(!std::isfinite(value))throw std::runtime_error("Non-finite tyre coefficient");
                    values.coefficients[field]=value*(field==0||field==8?1000.f:field==1||field==2?.01745329238474369f:1.f);
                }
                type.surfaces.push_back(values);
            }
            loaded.push_back(std::move(type));
        }
        //The original closes the handle after its declared records; trailing
        //bytes are ignored, just as each surface's extension bytes are ignored.
        types_=std::move(loaded);return true;
    }
    template<class Files> bool load(Files& files) {
        clear();
        const auto handle=files.open("tables\\tyres.bin",0);
        if(handle==0xffffffffu)return false;
        struct Close {Files& files;std::uint32_t handle;~Close(){files.close(handle);}} close{files,handle};
        const auto size=files.seek(handle,0,2);
        if(size==0xffffffffu||size>128u*1024u*1024u)throw std::runtime_error("Invalid tyre resource size");
        if(files.seek(handle,0,0)!=0)throw std::runtime_error("Cannot rewind tyre resource");
        std::vector<unsigned char> bytes(size);
        if(size&&files.read(handle,bytes.data(),size)!=size)throw std::runtime_error("Short tyre resource read");
        return read(SerializedBytes(bytes));
    }
private:
    std::vector<CarTyreType> types_;
};

} //namespace hd2::game
#endif
