#ifndef HD2_GAME_DOG_CONTROLLER_PROCESS_H
#define HD2_GAME_DOG_CONTROLLER_PROCESS_H
#include "game/dog_controller_vector.h"
#include <cstdint>
#include <memory>
#include <vector>
namespace hd2::game {
struct DogRoutePoint { //Native48-byte record, no cold payload writes.
    DogControllerVector position0,original10;
    std::optional<std::uint32_t> index20;
    std::optional<std::uint16_t> flags24;
    std::optional<std::uint8_t> type26;
    std::optional<float> radius28;
};
struct DogHeardRecord { //Native32-byte record; typed source14 loan is host-width.
    DogControllerVector position0;
    std::optional<std::uint32_t> kind10,clock18;
    std::optional<void*> source14;
    std::optional<std::int32_t> strength1c;
};
template<class Record>class DogControllerPool {
public:
    struct Node {Record* point=nullptr;Node* next=nullptr;};
    DogControllerPool(){refill();} //005CE2F0 static initializer:16 records per pool BEFORE World creation.
    Node* available=nullptr;Node* unused=nullptr;std::uint32_t count=0;
    Node& acquire_node(){ //005CB000: pop unused without clearing retained payload.
        if(unused){auto* node=unused;unused=node->next;return *node;}
        auto owner=std::make_unique<Node>();auto* node=owner.get();nodes_.push_back(std::move(owner));return *node;
    }
    void release(Record* point){
        if(!point)return;
        auto& node=acquire_node();node.point=point;node.next=available;++count;available=&node;
    }
    Record& take(){ //005CE4C0 pool prefix, refill only when available NULL.
        if(!available)refill();
        if(!available||!available->point)throw std::runtime_error("Dog pool reaches native NULL allocated record");
        auto* node=available;available=node->next;auto* point=node->point;
        node->next=unused;node->point=nullptr;unused=node;--count;return *point;
    }
private:
    void refill(){ //005CEF90; typed ownership replaces original aligned allocation/node memory ABI.
        for(unsigned n=0;n!=16;++n){auto owner=std::make_unique<Record>();auto* record=owner.get();records_.push_back(std::move(owner));release(record);}
    }
    std::vector<std::unique_ptr<Record>> records_;
    std::vector<std::unique_ptr<Node>> nodes_;
};
//Actual008AC610/710 scratch and008AC810 two-pool static owner: route48-byte
//records at810, heard32-byte records at828. FIRST available head814, unused818,
//growth81C=16/count820; second head82C, unused830,growth834/count838.
class DogControllerProcess {
public:
    std::array<float,64> first710{},second610{};
    DogControllerPool<DogRoutePoint> route;
    DogControllerPool<DogHeardRecord> heard;
    DogHeardRecord& acquire_heard(std::uint32_t kind,const std::array<float,4>& position,void* source,std::uint32_t clock){ //005CE560.
        auto& record=heard.take();record.kind10=kind;record.position0.assign(position);
        record.strength1c=0;record.source14=source;record.clock18=clock;return record;
    }
    DogRoutePoint& acquire_point(std::uint32_t index,const DogControllerVector& position,
            std::uint16_t flags,std::uint8_t type,std::uint8_t radius){ //005CE4C0, exact payload publication order.
        auto& point=route.take();point.index20=index;point.position0=position;point.original10=position;
        point.flags24=flags;point.type26=type;point.radius28=static_cast<float>(static_cast<long double>(radius)*.0625f);return point;
    }
    void release_route(std::vector<DogRoutePoint*>& points){ //005CB410 reverse nonNULL releases, capacity retained.
        for(auto n=points.size();n;){route.release(points[--n]);}points.clear();
    }
};
}
#endif
