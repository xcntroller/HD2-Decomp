#ifndef HD2_GAME_SHOT_HIT_SORT_H
#define HD2_GAME_SHOT_HIT_SORT_H
#include <vector>
#include <algorithm>
#include <stdexcept>
namespace hd2::game::shot_hit_sort {
//0044A910/0044AE00 with00455650 comparator. Native all-hit insertion already
//orders parameters, but this later median/partition pass REORDERS equal hits
//above16. std::stable_sort or assuming the incoming order would change damage.
template<class Hit>bool before(const Hit&a,const Hit&b){return a.parameter<b.parameter;}
template<class Hit>const Hit& median(const Hit&a,const Hit&b,const Hit&c){
    if(before(a,b)){if(before(b,c))return b;return before(a,c)?c:a;}
    if(before(a,c))return a;
    return before(b,c)?c:b;
}
template<class Hit>void partition_loop(std::vector<Hit>& hits,std::size_t first,std::size_t last){
    while(last-first>16){
        const Hit pivot=median(hits[first],hits[first+(last-first)/2],hits[last-1]);
        auto left=first,right=last;
        for(;;){
            while(left<last&&before(hits[left],pivot))++left;
            do{if(!right)throw std::runtime_error("Shot hit partition lost native comparator sentinel");--right;}while(before(pivot,hits[right]));
            if(right<=left)break;
            std::swap(hits[left],hits[right]);++left;
        }
        if(left==first||left==last)throw std::runtime_error("Shot hit partition reached invalid native comparator range");
        if(left-first<last-left){partition_loop(hits,first,left);first=left;}
        else{partition_loop(hits,left,last);last=left;}
    }
}
template<class Hit>void insertion(std::vector<Hit>& hits,std::size_t first,std::size_t last,bool guarded){
    for(auto index=first;index<last;++index){
        const Hit value=hits[index];auto target=index;
        if(guarded&&before(value,hits[0])){while(target){hits[target]=hits[target-1];--target;}}
        else{while(target&&before(value,hits[target-1])){hits[target]=hits[target-1];--target;}}
        hits[target]=value;
    }
}
template<class Hit>void sort(std::vector<Hit>& hits){
    if(hits.size()<2)return;
    if(hits.size()<=16){insertion(hits,1,hits.size(),true);return;}
    partition_loop(hits,0,hits.size());insertion(hits,1,16,true);insertion(hits,16,hits.size(),false);
}
} //namespace hd2::game::shot_hit_sort
#endif
