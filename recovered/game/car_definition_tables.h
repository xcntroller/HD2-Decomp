#ifndef HD2_GAME_CAR_DEFINITION_TABLES_H
#define HD2_GAME_CAR_DEFINITION_TABLES_H

#include "game/car_particles.h"
#include "game/serialized_bytes.h"
#include <optional>
#include <algorithm>

namespace hd2::game {

struct CarMaterialDefinition {
    std::array<unsigned char,0x100> bytes{};
    float scalar(unsigned offset) const {
        if(offset>bytes.size()-4)throw std::out_of_range("Car material definition scalar");
        float result;std::memcpy(&result,bytes.data()+offset,4);return result;
    }
    std::uint32_t word(unsigned offset) const {
        if(offset>bytes.size()-4)throw std::out_of_range("Car material definition word");
        return SerializedBytes(bytes.data()+offset,4).word();
    }
};

//0071F9B0's process-level serialized table owners. Production loads through the
//actual rw_data resolver. Fixed-record counts deliberately ignore trailing bytes
//as original; bounded native reads reject short reads rather than use undefined
//memory. If the second file is absent the first table is already replaced.
class CarDefinitionTables {
public:
    const std::vector<CarMaterialDefinition>& materials() const {return materials_;}
    const std::vector<CarParticleDefinition>& particles() const {return particles_;}
    void read_materials(SerializedBytes bytes) {read_records(bytes,materials_);}
    void read_particles(SerializedBytes bytes) {read_records(bytes,particles_);}
    template<class Files> bool load(Files& files) {
        const auto materials=read_file(files,"tables\\hdmaterials.def");
        if(!materials)return false;
        read_materials(SerializedBytes(*materials));
        const auto particles=read_file(files,"tables\\effects_car.def");
        if(!particles)return false;
        read_particles(SerializedBytes(*particles));return true;
    }
    void clear() noexcept {materials_.clear();particles_.clear();}
    void finish_mission() noexcept { //00698F80 frees actual0097B8A4 BEFORE0097B89C and zeroes both capacities.
        std::vector<CarMaterialDefinition>().swap(materials_);std::vector<CarParticleDefinition>().swap(particles_);
    }
private:
    template<class Record> static void read_records(SerializedBytes bytes,std::vector<Record>& destination) {
        std::vector<Record> next(bytes.remaining()/sizeof(Record));
        for(auto& record:next)std::memcpy(record.bytes.data(),bytes.take(record.bytes.size()),record.bytes.size());
        destination=std::move(next);
    }
    template<class Files> static std::optional<std::vector<unsigned char>> read_file(Files& files,const char* path) {
        const auto handle=files.open(path,0);if(handle==0xffffffffu)return std::nullopt;
        struct Close {Files& files;std::uint32_t handle;~Close(){files.close(handle);}} close{files,handle};
        const auto size=files.seek(handle,0,2);
        if(size==0xffffffffu||size>128u*1024u*1024u)throw std::runtime_error("Car definition resource exceeds bounds");
        if(files.seek(handle,0,0)!=0)throw std::runtime_error("Cannot rewind Car definition resource");
        std::vector<unsigned char> bytes(size);
        if(size&&files.read(handle,bytes.data(),size)!=size)throw std::runtime_error("Short Car definition resource read");
        return bytes;
    }
    std::vector<CarMaterialDefinition> materials_;
    std::vector<CarParticleDefinition> particles_;
};

struct CarMaterialResponse { //004D57A0, defined fields of the native48-byte value.
    std::uint32_t id=0;
    float field4=100,field8=100,fieldc=100,field10=100,field14=0,field18=.35f,field1c=1;
    std::array<std::uint32_t,3> words20{};
    bool flag2c=false,flag2d=false;
};

//004D57A0→004ECD60: rebuild canonical response values and the255-entry lookup.
//The real shared transient queues must clear before replacement; the containing
//world supplies that producer/owner rather than an independent successful stub.
class CarMaterialRegistry {
public:
    template<class Queues> void rebuild(const std::vector<CarMaterialDefinition>& definitions,Queues& queues) {
        std::vector<CarMaterialResponse> next;
        if(definitions.empty())next.emplace_back();
        else {
            if(definitions.size()>indices_.size())throw std::runtime_error("Car material IDs exceed original lookup storage");
            next.reserve(definitions.size());
            for(std::size_t n=0;n<definitions.size();++n) {
                const auto& input=definitions[n];CarMaterialResponse value;value.id=static_cast<std::uint32_t>(n);
                auto real=[&](unsigned offset) {
                    const auto v=input.scalar(offset);
                    if(!std::isfinite(v))throw std::runtime_error("Non-finite Car material response");
                    return v;
                };
                value.field18=std::clamp(real(0xd8),0.f,1.f);value.field1c=std::clamp(real(0xdc),1.f,2.f);
                value.field8=real(0xe0);value.field4=std::min(real(0xe4),value.field8);
                value.field10=real(0xe8);value.fieldc=std::min(real(0xec),value.field10);
                value.field14=std::clamp(real(0xf0),0.f,.2f);
                for(unsigned word=0;word!=3;++word)value.words20[word]=input.word(0xf4+word*4);
                switch(n) {case 48:case 51:case 52:case 53:case 54:case 55:case 64:case 65:case 95:
                    value.flag2c=value.flag2d=true;break;default:break;}
                next.push_back(value);
            }
        }
        queues.clear_game_queues(); //004ECDE0 before replacing values/lookup.
        indices_.fill(0);
        responses_=std::move(next);
        for(std::size_t n=0;n<responses_.size();++n)indices_.at(responses_[n].id)=static_cast<std::uint32_t>(n);
    }
    const std::vector<CarMaterialResponse>& responses() const {return responses_;}
    const std::array<std::uint32_t,255>& indices() const {return indices_;}
private:
    std::vector<CarMaterialResponse> responses_;
    std::array<std::uint32_t,255> indices_{};
};

} //namespace hd2::game
#endif
