#ifndef HD2_FRONTEND_CONTROL_CONNECTION_FRAGMENTS_H
#define HD2_FRONTEND_CONTROL_CONNECTION_FRAGMENTS_H

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace hd2::frontend {
// Socket30/34/38, owned by the SAME socket as receive buffer14. Native records
// are32 bytes, with id00/index04/payload08/length0C/final10/clock14. They carry
// NO sender identity. Typed byte ownership replaces the native packet pool;
// expiry/storage destruction also release buffers that native erase overwrites.
class ControlConnectionFragments {
    struct Key{std::uint32_t id,index;};
    struct Record{Key key;std::vector<unsigned char> payload;bool final;std::uint32_t clock;};
    static bool less(Key a,Key b){return a.id<b.id||(a.id==b.id&&a.index<b.index);} //00747C90, unsigned.
    static std::int32_t signed_word(std::uint32_t word){std::int32_t value;std::memcpy(&value,&word,4);return value;}
    static Key median(Key a,Key b,Key c){
        if(less(a,b))return less(b,c)?b:(less(a,c)?c:a);
        return less(a,c)?a:(less(b,c)?c:b);
    }
    //00748580/007487E0/00748760's partition, then insertion scheme. Using an
    //arbitrary library sort could select different equal-key fragment payloads.
    void partition(std::size_t first,std::size_t last){
        while(last-first>16){
            const auto pivot=median(records_[first].key,records_[first+(last-first)/2].key,records_[last-1].key);
            auto left=first,right=last;
            for(;;){
                while(less(records_[left].key,pivot))++left;
                do{--right;}while(less(pivot,records_[right].key));
                if(left>=right)break;
                std::swap(records_[left],records_[right]);++left;
            }
            if(left-first<=last-left){partition(left,last);last=left;}
            else{partition(first,left);first=left;}
        }
    }
    void sort(){ //00747DD0:512 bytes is a sort threshold, NOT a retained-record cap.
        if(records_.size()>16)partition(0,records_.size());
        for(std::size_t n=1;n<records_.size();++n){
            auto record=std::move(records_[n]);auto at=n;
            while(at&&less(record.key,records_[at-1].key)){records_[at]=std::move(records_[at-1]);--at;}
            records_[at]=std::move(record);
        }
    }
    std::optional<std::pair<std::size_t,std::size_t>> select(std::uint32_t now){ //00747CB0.
        std::size_t at=0;
        while(at<records_.size()){
            if(signed_word(now)>signed_word(records_[at].clock+20000u)){
                records_.erase(records_.begin()+static_cast<std::ptrdiff_t>(at));continue;
            }
            if(records_[at].key.index){++at;continue;}
            const auto start=at;const auto id=records_[at].key.id;std::uint32_t index=0;
            while(at<records_.size()&&records_[at].key.id==id&&records_[at].key.index==index){
                if(records_[at].final)return std::pair<std::size_t,std::size_t>{start,index+1u};
                ++at;++index;
            }
            //Native747D58 fetches next.id before its end test. At logical end
            //the result is false for either value; no invented next record.
        }
        return {};
    }
public:
    //00747A40: called only after checksum/decryption and the high-bit test.
    //The caller reads the fresh clock AFTER copying payload, before append.
    void append(std::uint32_t id,unsigned index,bool final,std::vector<unsigned char> payload,std::uint32_t clock){
        records_.push_back({{id,index},std::move(payload),final,clock});
    }
    template<class Clock>std::optional<std::size_t> assemble(unsigned char* buffer,std::size_t capacity,Clock clock){ //00747DD0.
        sort();const auto range=select(clock());if(!range)return {}; //747CB0 reads AFTER sorting.
        std::size_t size=0;
        for(std::size_t n=0;n<range->second;++n){
            auto& value=records_.at(range->first);
            if(value.payload.size()>capacity-size)throw std::runtime_error("Fragment assembly exceeds actual socket14 buffer");
            if(!buffer)throw std::logic_error("Fragment assembly requires actual socket14 storage");
            std::copy(value.payload.begin(),value.payload.end(),buffer+size);size+=value.payload.size();
            //Native copy/free/erase is per fragment, not an atomic whole run.
            records_.erase(records_.begin()+static_cast<std::ptrdiff_t>(range->first));
        }
        return size;
    }
    std::size_t size()const{return records_.size();}
    void release_storage(){std::vector<Record> empty;records_.swap(empty);}
private:
    std::vector<Record> records_;
};
} //namespace hd2::frontend
#endif
