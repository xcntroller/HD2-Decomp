#ifndef HD2_IO_RESOURCE_OBSERVATIONS_H
#define HD2_IO_RESOURCE_OBSERVATIONS_H
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace hd2::io {
// Diagnostic SHA-256 over bytes already returned by the real resource operation.
// No resolver queries, extra reads or file/actor/frame borrowers are retained.
class ResourceSha256 {
public:
    void append(const void* data,std::size_t size) {
        const auto* p=static_cast<const unsigned char*>(data);length_+=size;
        while(size){const auto n=std::min(size,64-used_);std::memcpy(block_.data()+used_,p,n);
            used_+=n;p+=n;size-=n;if(used_==64){compress();used_=0;}}
    }
    std::string hex()const {
        auto copy=*this;const auto bits=length_*8;const unsigned char marker=128,zero=0;
        copy.append(&marker,1);while(copy.used_!=56)copy.append(&zero,1);
        unsigned char tail[8];for(unsigned i=0;i<8;++i)tail[7-i]=static_cast<unsigned char>(bits>>(i*8));
        copy.append(tail,8);std::string out;const char* digits="0123456789abcdef";
        for(auto word:copy.state_)for(int shift=28;shift>=0;shift-=4)out+=digits[(word>>shift)&15];
        return out;
    }
private:
    static std::uint32_t rotate(std::uint32_t x,unsigned n){return (x>>n)|(x<<(32-n));}
    void compress(){
        static constexpr std::uint32_t k[]={
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
        std::uint32_t w[64];for(unsigned i=0;i<16;++i){w[i]=0;for(unsigned j=0;j<4;++j)w[i]=(w[i]<<8)|block_[i*4+j];}
        for(unsigned i=16;i<64;++i){const auto x=w[i-15],y=w[i-2];w[i]=w[i-16]+(rotate(x,7)^rotate(x,18)^(x>>3))+w[i-7]+(rotate(y,17)^rotate(y,19)^(y>>10));}
        auto a=state_[0],b=state_[1],c=state_[2],d=state_[3],e=state_[4],f=state_[5],g=state_[6],h=state_[7];
        for(unsigned i=0;i<64;++i){const auto t=h+(rotate(e,6)^rotate(e,11)^rotate(e,25))+((e&f)^(~e&g))+k[i]+w[i];
            const auto u=(rotate(a,2)^rotate(a,13)^rotate(a,22))+((a&b)^(a&c)^(b&c));h=g;g=f;f=e;e=d+t;d=c;c=b;b=a;a=t+u;}
        const std::uint32_t v[]={a,b,c,d,e,f,g,h};for(unsigned i=0;i<8;++i)state_[i]+=v[i];
    }
    std::array<std::uint32_t,8> state_{{0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19}};
    std::array<unsigned char,64> block_{};std::size_t used_=0;std::uint64_t length_=0;
};
inline std::string resource_sha256(const void* data,std::size_t size){ResourceSha256 hash;hash.append(data,size);return hash.hex();}

struct DataObservation {
    std::string level,operation,detail;std::uint64_t value=0;
};
struct ResourceObservation {
    std::string name,origin="unavailable: rw_data exports no selected-origin query";
    std::uint32_t flags=0;bool opened=false,closed=false,io_fault=false,size_known=false,diagnostic_lost=false,short_read=false;
    std::uint32_t size=0;std::uint64_t returned_bytes=0;
    std::string sha256; //Only a complete contiguous [0,size) read, never inferred from a filename.
};
// Opt-in owning store, not a resource cache. Replacement affects future opens;
// live handles finish in the store selected by their open. No callbacks exist.
// Diagnostic allocation failures are counted and cannot replace native failures.
class ResourceObservations {
public:
    const auto& records()const noexcept{return records_;}
    const auto& events()const noexcept{return events_;}
    void event(const char* level,const char* operation,const char* detail="",std::uint64_t value=0)noexcept{
        try{if(events_.size()>=65536){++dropped_;return;}events_.push_back({level,operation,detail,value});}catch(...){++dropped_;}
    }
    std::uint64_t dropped()const noexcept{return dropped_;}
    void note_drop()noexcept{++dropped_;}
    std::size_t opened(const char* name,std::uint32_t flags,std::uint32_t handle)noexcept {
        try{if(records_.size()>=65536){++dropped_;return invalid;}ResourceObservation record;record.name=name;records_.push_back(std::move(record));auto& r=records_.back();r.flags=flags;r.opened=handle!=0xffffffffu;return records_.size()-1;}
        catch(...){++dropped_;return invalid;}
    }
    template<class F>void update(std::size_t index,F&& f)noexcept {if(index==invalid)return;try{f(records_.at(index));}catch(...){++dropped_;if(index<records_.size())records_[index].diagnostic_lost=true;}}
    static constexpr std::size_t invalid=static_cast<std::size_t>(-1);
private:std::vector<DataObservation> events_;std::vector<ResourceObservation> records_;std::uint64_t dropped_=0;
};
// Composition used by ChunkFiles after each actual operation. It never invokes
// the resolver itself and cannot alter search precedence or loading ordering.
class ResourceOperationObserver {
    struct Live{std::shared_ptr<ResourceObservations> store;std::size_t index;std::uint32_t position=0,size=0;bool known=false,contiguous=true;std::uint64_t hashed=0;ResourceSha256 hash;};
public:
    void bind(std::shared_ptr<ResourceObservations> store)noexcept{store_=std::move(store);}
    auto store()const noexcept{return store_;}
    void opened(const char* name,std::uint32_t flags,std::uint32_t handle)noexcept{
        if(!store_)return;
        const auto index=store_->opened(name,flags,handle);if(handle==0xffffffffu||index==ResourceObservations::invalid)return;
        try{live_.insert_or_assign(handle,Live{store_,index,0,0,false,true,0,ResourceSha256{}});}catch(...){store_->note_drop();store_->update(index,[](auto& r){r.diagnostic_lost=true;});}
    }
    void sought(std::uint32_t handle,std::uint32_t offset,std::uint32_t origin,std::uint32_t result)noexcept{
        auto it=live_.find(handle);if(it==live_.end())return;auto& l=it->second;
        l.store->update(l.index,[&](auto& r){if(result==0xffffffffu){r.io_fault=true;l.contiguous=false;return;}
            if(origin==2&&offset==0){l.size=result;l.known=true;r.size=result;r.size_known=true;}
            if(result==0){l.hash=ResourceSha256{};l.hashed=0;l.contiguous=true;}
            else if(result!=l.hashed)l.contiguous=false;
            l.position=result;
            if(l.known&&l.size==0)r.sha256=l.hash.hex();});
    }
    void read(std::uint32_t handle,const void* bytes,std::uint32_t count,std::uint32_t result)noexcept{
        auto it=live_.find(handle);if(it==live_.end())return;auto& l=it->second;
        l.store->update(l.index,[&](auto& r){if(result>count){r.io_fault=true;l.contiguous=false;return;}
            r.returned_bytes+=result;if(result<count)r.short_read=true;
            if(l.contiguous&&l.position==l.hashed){l.hash.append(bytes,result);l.hashed+=result;
                if(l.known&&l.hashed==l.size)r.sha256=l.hash.hex();}
            l.position+=result;});
    }
    void closed(std::uint32_t handle)noexcept{auto it=live_.find(handle);if(it==live_.end())return;auto& l=it->second;
        l.store->update(l.index,[](auto& r){r.closed=true;});live_.erase(it);}
private:std::shared_ptr<ResourceObservations> store_;std::map<std::uint32_t,Live> live_;
};
//Owner must outlive this stack scope. Replacing the observer during a scope
//wins over restoration; retained stores never retain the owner itself.
template<class Owner>class ResourceObservationScope {
public:
    ResourceObservationScope(Owner& owner,std::shared_ptr<ResourceObservations> store)
        :owner_(owner),previous_(owner.resource_observations()),installed_(std::move(store)){owner_.observe_resources(installed_);}
    ~ResourceObservationScope(){if(owner_.resource_observations()==installed_)owner_.observe_resources(std::move(previous_));}
    ResourceObservationScope(const ResourceObservationScope&)=delete;
    ResourceObservationScope& operator=(const ResourceObservationScope&)=delete;
private:Owner& owner_;std::shared_ptr<ResourceObservations> previous_,installed_;
};
//Generic parsers can also be used by native endpoints without observations.
//Only a concrete endpoint with observe_data binds this optional diagnostic.
template<class T>auto observe_bound_data_impl(T& endpoint,const char* level,const char* operation,const char* detail,std::uint64_t value,int)
    noexcept -> decltype(endpoint.observe_data(level,operation,detail,value),void()) {endpoint.observe_data(level,operation,detail,value);}
template<class T>void observe_bound_data_impl(T&,const char*,const char*,const char*,std::uint64_t,long)noexcept{}
template<class T>void observe_bound_data(T& endpoint,const char* level,const char* operation,const char* detail="",std::uint64_t value=0)noexcept{
    observe_bound_data_impl(endpoint,level,operation,detail,value,0);
}
}
#endif
