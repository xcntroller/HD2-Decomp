#ifndef HD2_GAME_MISSION_NAVIGATION_H
#define HD2_GAME_MISSION_NAVIGATION_H

#include "game/mission_tables.h"
#include "game/table_storage.h"
#include "game/serialized_bytes.h"
#include "game/mission_table_reader.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace hd2::game {

//0051D5A0/0051D5C0/0051CCD0/0051CBA0 over the SAME decoded table set. Native
//records are ordered by signed ten-unit X band(prefix0C), then positionZ. No
//duplicate navigation owner, guessed spatial grid or copied point list is used.
class MissionNavigation {
public:
    explicit MissionNavigation(MissionTableSet& table) : table_(table) {}
    void reset_runtime_links(){ //0051F230 via00523E30: FIRST dataset only; retains vector capacity.
        table_.end34=table_.begin30;
        for(std::uint32_t n=0;n<table_.count0;++n){
            auto* point=record(static_cast<std::uint16_t>(n));
            point->flags&=0xddffu;point->reserved17[3]=0; //Native byte1A, NOT navigation point byte16.
            if((point->flags&0xc0u)==0xc0u){ //00524870: remove final synthesized sentinel.
                const auto count=point->flags&0x3fu;
                if(count){
                    if(!point->payload)throw std::runtime_error("Navigation reset lost actual synthesized link payload");
                    auto* last=static_cast<unsigned char*>(point->payload)+(count-1)*4;
                    last[0]=last[1]=0xff;last[2]=last[3]=0;--point->flags;
                }
                point->flags&=0xff7fu;
            }
        }
    }
    std::vector<std::uint16_t> actor_references(const void* actor)const { //0051E6D0, reverse second-family references.
        if(table_.count1c&&!table_.data20)throw std::runtime_error("Navigation table has no actor references");
        const auto* references=static_cast<const MissionTableReference*>(table_.data20);
        std::vector<std::uint16_t> result;
        for(auto n=table_.count1c;n;--n)if(references[n-1].object==actor)result.push_back(references[n-1].value);
        return result;
    }
    void* object_for_reference(std::uint16_t value)const { //0051E520, sorted SECOND-family references.
        if(!table_.count1c)return nullptr;
        if(!table_.data20)throw std::runtime_error("Navigation table has no reference records");
        const auto* entries=static_cast<const MissionTableReference*>(table_.data20);
        std::uint32_t low=0,high=table_.count1c-1;
        if(value>entries[high].value)return nullptr;
        if(value==entries[high].value)return entries[high].object; //Last boundary is tested FIRST, including duplicates.
        if(value<entries[0].value)return nullptr;
        if(value==entries[0].value)return entries[0].object;
        while(high-low>1) {
            const auto middle=(low+high)/2;
            if(entries[middle].value==value)return entries[middle].object;
            if(value<entries[middle].value)high=middle;else low=middle;
        }
        return nullptr;
    }
    bool append_reachable(std::uint16_t start,float radius,std::vector<std::uint16_t>& output)const { //00520830/00520860/00520530.
        if(!record(start)||!table_.count0||radius<0)return false;
        //Native wrapper accepts zero AND unordered. Inner walk does nothing
        //for zero; NaN still appends the origin, rejecting all ordered edges.
        if(radius==0)return true;
        struct Visit {std::uint16_t index;float distance;};
        std::vector<Visit> queue{{start,0.f}};
        for(std::size_t n=0;n<queue.size();++n) {
            const auto* point=record(queue[n].index);if(!point)continue;
            const auto count=point->flags&0x3fu;
            if(!point->payload) {
                if(count)throw std::runtime_error("Navigation radius walk has missing required link storage");
                continue;
            }
            const auto* links=static_cast<const unsigned char*>(point->payload);
            for(unsigned link=0;link<count;++link) {
                const auto* bytes=links+link*4;
                const auto index=static_cast<std::uint16_t>(bytes[0]|(std::uint16_t(bytes[1])<<8));
                const auto weight=static_cast<std::uint16_t>(bytes[2]|(std::uint16_t(bytes[3])<<8));
                const auto distance=static_cast<long double>(weight)*.00390625f+queue[n].distance; //00810164=1/256.
                if(!(distance<=radius))continue; //Compare extended sum BEFORE storing the float.
                bool found=false;for(const auto& visit:queue)if(visit.index==index){found=true;break;}
                if(!found)queue.push_back({index,static_cast<float>(distance)}); //First discovery wins; no relaxation.
            }
        }
        for(const auto& visit:queue)output.push_back(visit.index);
        return true;
    }
    bool append_marked_reachable(std::uint16_t start,float radius,std::vector<std::uint16_t>& output)const { //005208E0/00520910.
        std::vector<std::uint16_t> points;
        if(!append_reachable(start,radius,points))return false;
        for(auto index:points)if(const auto* point=record(index))
            if(point->reserved17[0]|point->reserved17[1]|point->reserved17[2])output.push_back(index); //00524B10(7).
        return true;
    }
    //005223B0 -> 00521B60. Search scratch is indexed by the original point
    //number; all positions, flags and edges still belong to the loaded table.
    bool find_route(std::uint16_t start,std::uint16_t destination,std::uint32_t mask,
                    std::vector<std::uint16_t>& route) const {
        if(!record(start)||!record(destination))return false;
        route.clear();
        if(start==destination){route.push_back(start);return true;}

        const auto target=position(destination);
        const auto estimate=[&](std::uint16_t index){
            const auto point=position(index);
            const auto x=std::abs(static_cast<long double>(target[0])-point[0]);
            const auto y=std::abs(static_cast<long double>(target[1])-point[1]);
            const auto z=std::abs(static_cast<long double>(target[2])-point[2]);
            const auto scaled=((z+y)+x)*256.f; //008109C4, then __ftol.
            return route_integer(scaled);
        };
        return search_route(start,destination,mask,route,estimate);
    }
    //00521460 -> 00521480. End at the first discovered point outside every
    //target's exclusion radius with zero truncated distance penalty.
    bool find_escape_route(std::uint16_t start,const std::vector<std::array<float,3>>& targets,
                           float exclusion,float range,std::uint32_t mask,std::vector<std::uint16_t>& route)const {
        if(!record(start)||targets.empty())return false;
        route.clear();
        const auto estimate=[&](std::uint16_t index){
            const auto point=position(index);
            float penalty=0;
            for(auto target=targets.rbegin();target!=targets.rend();++target){
                const auto x=static_cast<long double>(static_cast<float>(point[0]-(*target)[0]));
                const auto y=static_cast<long double>(static_cast<float>(point[1]-(*target)[1]));
                const auto z=static_cast<long double>(static_cast<float>(point[2]-(*target)[2]));
                const float square=static_cast<float>((x*x+y*y)+z*z);
                const auto distance=square<1.e-8f?0.L:
                    std::abs(static_cast<long double>(square)-1)<1.e-8f?1.L:std::sqrt(static_cast<long double>(square));
                if(!(distance>exclusion))return 0xffffffffu;
                const auto value=static_cast<long double>(range)/distance-1;
                if(penalty<value)penalty=static_cast<float>(value);
            }
            return route_integer(static_cast<long double>(penalty)*256.f);
        };
        return search_route(start,0xffff,mask,route,estimate);
    }
    MissionTableRecord* record(std::uint16_t index) const {
        if(index>=table_.count0)return nullptr;
        if(!table_.records4)throw std::runtime_error("Navigation table has no records");
        return static_cast<MissionTableRecord*>(table_.records4)+index;
    }
    auto& movement_floor_candidates(){return movement_floor_candidates_;}
    std::uint16_t index_of(const MissionTableRecord& point)const{
        //The reference comes from this table's actual A4/record() borrower.
        return static_cast<std::uint16_t>(&point-record(0));
    }
    std::uint16_t escape_restricted_point(const MissionTableRecord* start)const { //0051E8A0.
        if(!table_.count0||!start)return 0;
        const auto index=index_of(*start);
        const auto origin=position(index);
        for(float radius=5;radius<=20;radius+=5){
            std::vector<std::uint16_t> nearby;
            append_reachable(index,radius,nearby);
            std::uint16_t selected=0xffff;
            float best=10000.f;
            for(auto candidate:nearby){
                const auto* point=record(candidate);
                if(!point)throw std::runtime_error("Navigation escape link exceeds the point table");
                if(point->flags&0x2000u)continue;
                const auto xyz=position(candidate);
                const float x=xyz[0]-origin[0];
                const auto y=static_cast<long double>(xyz[1])-origin[1];
                const auto z=static_cast<long double>(xyz[2])-origin[2];
                const auto squared=(static_cast<long double>(x)*x+y*y)+z*z;
                if(squared<best){best=static_cast<float>(squared);selected=candidate;}
            }
            if(selected!=0xffff)return selected;
        }
        return 0xffff;
    }
    std::array<float,3> position(std::uint16_t index) const {
        const auto* value=record(index);if(!value)throw std::runtime_error("Navigation point is unavailable");
        return {{real(*value,0),real(*value,4),real(*value,8)}};
    }
    void point_byte(std::uint16_t index,std::uint8_t value) {
        auto* point=record(index);if(!point)throw std::runtime_error("Navigation point is unavailable");
        point->byte16=value; //005E2420 writes the actual table record, not an actor-local flag.
    }
    static std::int32_t x_band(float x) { //00524750: truncate x*float(.1), THEN decrement for any negative X.
        const auto scaled=static_cast<long double>(x)*.100000001490116119384765625f;
        if(!std::isfinite(scaled)||scaled<std::numeric_limits<std::int32_t>::min()+1.L||
           scaled>std::numeric_limits<std::int32_t>::max())throw std::runtime_error("Navigation X band exceeds bounds");
        auto band=static_cast<std::int32_t>(scaled);if(x<0)--band;return band;
    }
    std::uint16_t nearest(const std::array<float,3>& point,float radius,float minimum) const {
        if(!table_.count0)return 0xffff;
        for(auto value:point)if(!std::isfinite(value))throw std::runtime_error("Non-finite navigation query");
        if(point[1]>100000.f||point[1]<-100000.f)return 0xffff;
        if(!std::isfinite(radius)||!std::isfinite(minimum))throw std::runtime_error("Non-finite navigation range");
        validate();
        const float lower=static_cast<float>((minimum>0?static_cast<long double>(minimum)*minimum:
            static_cast<long double>(minimum))+.001000000047497451305389404296875f);
        float best=static_cast<float>(static_cast<long double>(radius)*radius);
        if(!std::isfinite(best)||!std::isfinite(lower))throw std::runtime_error("Navigation squared range exceeds bounds");
        auto left_band=x_band(point[0]),right_band=left_band;
        float left=static_cast<float>(static_cast<long double>(point[0])-static_cast<long double>(left_band)*10);
        float right=static_cast<float>(10.L-left);
        std::uint16_t selected=0xffff;
        visit_band(left_band,point,selected,best,lower);
        if(best==0)return selected;
        unsigned iterations=0;
        for(;;) {
            if(++iterations>1000000u)throw std::runtime_error("Navigation query exceeds traversal bounds");
            bool visited_left=false;
            if(static_cast<long double>(left)*left<best) {
                left=static_cast<float>(static_cast<long double>(left)+10);
                if(left_band==std::numeric_limits<std::int32_t>::min())throw std::runtime_error("Navigation band underflow");
                --left_band;visited_left=true;visit_band(left_band,point,selected,best,lower);
            }
            if(static_cast<long double>(right)*right<best) {
                right=static_cast<float>(static_cast<long double>(right)+10);
                if(right_band==std::numeric_limits<std::int32_t>::max())throw std::runtime_error("Navigation band overflow");
                ++right_band;visit_band(right_band,point,selected,best,lower);continue;
            }
            if(!visited_left)return selected;
        }
    }
    //0051D780/0051D160: Human state binding searches distance to a navigation
    //record's volume (byte13/16 radius), not0051CCD0's point-centre metric.
    std::uint16_t nearest_volume(const std::array<float,3>& point,float radius,float minimum) const {
        return nearest_volume_impl(point,radius,minimum,false);
    }
    //0051D970/0051CE70: type0F only and X/Z volume distance, ignoring Y.
    //Used after0051D780 fails during formation leader placement.
    std::uint16_t nearest_floor_volume(const std::array<float,3>& point,float radius,float minimum) const {
        return nearest_volume_impl(point,radius,minimum,true);
    }
private:
    static std::uint32_t route_integer(long double value){
        if(!(value<0x1p63L))return 0; //Low DWORD of the x87 indefinite.
        return static_cast<std::uint32_t>(static_cast<std::int64_t>(value));
    }
    template<class Estimate>
    bool search_route(std::uint16_t start,std::uint16_t destination,std::uint32_t mask,
                      std::vector<std::uint16_t>& route,Estimate estimate)const {
        constexpr std::uint16_t none=0xffff;
        struct SearchPoint {
            std::uint32_t cost=0,estimate=0,score=0;
            std::uint16_t parent=0xffff,next=0xffff;
            bool closed=false;
        };
        std::vector<SearchPoint> points(table_.count0);
        points[start].parent=start;
        points[start].estimate=points[start].score=estimate(start);
        if(destination==none&&points[start].estimate==0xffffffffu)return false;
        for(auto head=start;head!=none;head=points[head].next){
            auto& current=points[head];
            current.closed=true;
            const auto& point=*record(head);
            auto remaining=point.flags&0x3fu;
            if(remaining&&!point.payload)throw std::runtime_error("Navigation route has missing link storage");
            const auto* links=static_cast<const unsigned char*>(point.payload);
            while(remaining){
                SerializedBytes link(links+(--remaining)*4u,4);
                const auto index=link.half();
                const auto weight=link.half();
                const auto* neighbor=record(index);
                if(!neighbor)throw std::runtime_error("Navigation route link exceeds the point table");
                if(neighbor->flags&mask&0xfb40u)continue;

                auto& next=points[index];
                const bool pending=next.parent!=none;
                const float cost=static_cast<float>(current.cost+weight); //Original DWORD add then FLOAT spill.
                if(!pending||(!next.closed&&static_cast<long double>(cost)<next.cost)){
                    next.parent=head;
                    next.cost=static_cast<std::uint32_t>(static_cast<std::uint64_t>(cost));
                    if(!pending)next.estimate=estimate(index);
                    if(destination==none&&next.estimate==0xffffffffu){
                        next.closed=true;
                        continue;
                    }
                    next.score=next.cost+next.estimate;
                    //00521120/00521160 keep the expanding head in place and
                    //insert before equal scores. Find the insertion position
                    //before unlinking an existing node, as in 00521160.
                    auto* insert=&current.next;
                    while(*insert!=none&&points[*insert].score<next.score)insert=&points[*insert].next;
                    if(pending){
                        auto* old=insert;
                        while(*old!=index)old=&points[*old].next;
                        *old=next.next;
                    }
                    next.next=*insert;
                    *insert=index;
                }
                if(destination==none?(!pending&&next.estimate==0):index==destination){
                    //The original accepts a goal on discovery, before popping it.
                    for(auto node=index;;node=points[node].parent){
                        route.push_back(node);
                        if(node==start)break;
                    }
                    std::reverse(route.begin(),route.end());
                    return true;
                }
            }
        }
        return false;
    }
    std::uint16_t nearest_volume_impl(const std::array<float,3>& point,float radius,float minimum,bool floor_only) const {
        if(!table_.count0)return 0xffff;
        for(auto value:point)if(!std::isfinite(value))throw std::runtime_error("Non-finite navigation volume query");
        if(point[1]>100000.f||point[1]<-100000.f)return 0xffff;
        if(!std::isfinite(radius)||!std::isfinite(minimum))throw std::runtime_error("Non-finite navigation volume range");
        validate();
        const float lower=static_cast<float>((minimum>0?static_cast<long double>(minimum)*minimum:
            static_cast<long double>(minimum))+.001000000047497451305389404296875f);
        float best=static_cast<float>(static_cast<long double>(radius)*radius);
        if(!std::isfinite(best)||!std::isfinite(lower))throw std::runtime_error("Navigation volume range exceeds bounds");
        auto left_band=x_band(point[0]),right_band=left_band;
        float left=static_cast<float>((static_cast<long double>(point[0])-static_cast<long double>(left_band)*10)-16);
        float right=static_cast<float>((10.L-left)-32);
        std::uint16_t selected=0xffff;
        visit_volume_band(left_band,point,selected,best,lower,floor_only);
        if(best==0)return selected;
        unsigned iterations=0;
        for(;;) {
            if(++iterations>1000000u)throw std::runtime_error("Navigation volume query exceeds traversal bounds");
            bool visited_left=false;
            if(left<0||static_cast<long double>(left)*left<best) {
                left=static_cast<float>(static_cast<long double>(left)+10);
                if(left_band==std::numeric_limits<std::int32_t>::min())throw std::runtime_error("Navigation band underflow");
                --left_band;visited_left=true;visit_volume_band(left_band,point,selected,best,lower,floor_only);
            }
            if(right<0||static_cast<long double>(right)*right<best) {
                right=static_cast<float>(static_cast<long double>(right)+10);
                if(right_band==std::numeric_limits<std::int32_t>::max())throw std::runtime_error("Navigation band overflow");
                ++right_band;visit_volume_band(right_band,point,selected,best,lower,floor_only);continue;
            }
            if(!visited_left)return selected;
        }
    }
public:
    void validate() const {
        const MissionTableRecord* previous=nullptr;
        for(std::uint32_t i=0;i!=table_.count0;++i) {
            const auto* value=record(static_cast<std::uint16_t>(i));
            for(unsigned axis=0;axis!=3;++axis)if(!std::isfinite(real(*value,axis*4)))
                throw std::runtime_error("Non-finite navigation point");
            if(previous&&(band(*value)<band(*previous)||
                (band(*value)==band(*previous)&&real(*value,8)<real(*previous,8))))
                throw std::runtime_error("Navigation points are not in original search order");
            previous=value;
        }
    }
private:
    void visit_volume_band(std::int32_t cell,const std::array<float,3>& point,
                          std::uint16_t& selected,float& best,float minimum,bool floor_only=false) const {
        std::uint16_t lower=0xffff,upper=0xffff;
        if(selected!=0xffff) {
            if(band(*record(selected))<cell)lower=selected;else upper=selected;
        }
        const auto found=search(cell,point[2],lower,upper);
        const auto pivot=found==0xffff?std::uint32_t(table_.count0):std::uint32_t(found);
        const auto visit=[&](std::uint32_t index,bool reverse) {
            const auto& value=*record(static_cast<std::uint16_t>(index));
            if(band(value)!=cell)return false;
            if(floor_only&&value.byte16!=15)return true;
            const auto z=static_cast<long double>(point[2])-real(value,8);
            auto edge=(reverse?z:-z)-16;
            if(edge>=0)edge*=edge;
            if(best<=edge)return false;
            auto x=static_cast<long double>(point[0])-real(value,0);
            const auto y=static_cast<long double>(point[1])-real(value,4);
            //Reverse scan stores X and stores/reuses Y before its product.
            const auto floor_x_squared=reverse?x*static_cast<float>(x):x*x; //0051D00A FST, not FSTP.
            if(reverse)x=static_cast<float>(x);
            const auto y_squared=reverse?y*static_cast<float>(y):y*y;
            const auto distance=std::sqrt(floor_only?floor_x_squared+z*z:(y_squared+x*x)+z*z);
            const auto far_edge=(distance-16)-10;
            if(far_edge>0&&best<=far_edge*far_edge)return false;
            auto metric=distance-static_cast<unsigned char>(value.prefix[0x13])*.0625L;
            metric=metric<=0?0:metric*metric;
            if((floor_only?minimum<=metric:minimum<metric)&&metric<best){best=static_cast<float>(metric);selected=static_cast<std::uint16_t>(index);}
            return true;
        };
        for(auto i=pivot;i<table_.count0;++i)if(!visit(i,false)||best==0)break;
        if(best==0)return;
        for(auto i=static_cast<std::int32_t>(pivot)-1;i>=0;--i)if(!visit(static_cast<std::uint32_t>(i),true)||best==0)break;
    }
    static std::uint32_t word(const MissionTableRecord& value,unsigned offset) {
        SerializedBytes bytes(value.prefix.data()+offset,4);return bytes.word();
    }
    static float real(const MissionTableRecord& value,unsigned offset) {
        const auto bits=word(value,offset);float result;std::memcpy(&result,&bits,4);return result;
    }
    static std::int32_t band(const MissionTableRecord& value) {
        const auto bits=word(value,12);std::int32_t result;std::memcpy(&result,&bits,4);return result;
    }
    std::uint16_t search(std::int32_t cell,float z,std::uint16_t lower,std::uint16_t upper) const {
        if(!table_.count0)return 0xffff;
        std::uint32_t lo=lower==0xffff?0:lower,hi=upper==0xffff?table_.count0-1u:upper;
        if(hi<lo)return 0xffff;
        if(hi>=table_.count0)throw std::runtime_error("Navigation search anchor exceeds bounds");
        const auto* last=record(static_cast<std::uint16_t>(hi));
        if(band(*last)<cell||(band(*last)==cell&&real(*last,8)<=z))return 0xffff;
        const auto* first=record(static_cast<std::uint16_t>(lo));
        if(cell<band(*first)||(band(*first)==cell&&z<=real(*first,8)))return 0; //Original returns0, not lo.
        for(;;) {
            const auto middle=(lo+1+hi)/2;
            if(hi==middle)return static_cast<std::uint16_t>(hi);
            const auto* value=record(static_cast<std::uint16_t>(middle));
            if(band(*value)<=cell) {
                if(band(*value)<cell){lo=middle;continue;}
                if(real(*value,8)<=z) {
                    if(z<=real(*value,8))return static_cast<std::uint16_t>(middle);
                    lo=middle;continue;
                }
            }
            hi=middle;
        }
    }
    void visit_band(std::int32_t cell,const std::array<float,3>& point,
                    std::uint16_t& selected,float& best,float minimum) const {
        std::uint16_t lower=0xffff,upper=0xffff;
        if(selected!=0xffff) {
            if(band(*record(selected))<cell)lower=selected;else upper=selected;
        }
        const auto found=search(cell,point[2],lower,upper);
        const auto pivot=found==0xffff?std::uint32_t(table_.count0):std::uint32_t(found);
        const auto visit=[&](std::uint32_t index) {
            const auto* value=record(static_cast<std::uint16_t>(index));
            if(band(*value)!=cell)return false;
            const auto z=static_cast<long double>(point[2])-real(*value,8);
            const auto z_squared=z*z;
            if(!(z_squared<best))return false;
            const auto x=static_cast<long double>(point[0])-real(*value,0);
            const auto y=static_cast<long double>(point[1])-real(*value,4);
            const auto squared=(x*x+y*y)+z_squared;
            if(minimum<=squared&&squared<best) {best=static_cast<float>(squared);selected=static_cast<std::uint16_t>(index);}
            return true;
        };
        for(auto i=pivot;i<table_.count0;++i)if(!visit(i))break;
        for(auto i=static_cast<std::int32_t>(pivot)-1;i>=0;--i)if(!visit(static_cast<std::uint32_t>(i)))break;
    }
    MissionTableSet& table_;
    std::vector<MissionTableRecord*> movement_floor_candidates_; //00569890's retained navigation search buffer.
};

} //namespace hd2::game
#endif
