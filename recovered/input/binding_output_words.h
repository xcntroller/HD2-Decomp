#ifndef HD2_INPUT_BINDING_OUTPUT_WORDS_H
#define HD2_INPUT_BINDING_OUTPUT_WORDS_H
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
namespace hd2::input {
//006C7480 appends zero words one-by-one via0045CCC0. Native allocation grows
//size+max(size,1);006C72B0 reports LOGICAL size, while gameplay's64-word views
//may also write the already allocated spare word. Keep those extents separate.
//Spare bytes retain their actual allocation contents until a native producer or
//consumer writes them. No extra binding, neutral word or copied input view.
class BindingOutputWords {
public:
    using Word=std::uint32_t;
    BindingOutputWords()=default;
    BindingOutputWords(std::initializer_list<Word> words){for(auto word:words)push_back(word);}
    BindingOutputWords(const BindingOutputWords& other){
        if(other.capacity_){
            storage_.reset(new Word[other.capacity_]); //Default initialization deliberately leaves spare bytes unwritten.
            std::memcpy(storage_.get(),other.storage_.get(),other.capacity_*sizeof(Word));
        }
        size_=other.size_;capacity_=other.capacity_;
    }
    BindingOutputWords& operator=(const BindingOutputWords& other){if(this!=&other){BindingOutputWords next(other);swap(next);}return *this;}
    BindingOutputWords(BindingOutputWords&& other)noexcept{swap(other);}
    BindingOutputWords& operator=(BindingOutputWords&& other)noexcept{if(this!=&other){BindingOutputWords next(std::move(other));swap(next);}return *this;}
    BindingOutputWords& operator=(std::initializer_list<Word> words){clear();for(auto word:words)push_back(word);return *this;}
    std::size_t size()const noexcept{return size_;}
    std::size_t capacity()const noexcept{return capacity_;}
    std::size_t writable_size()const noexcept{return capacity_;}
    bool empty()const noexcept{return size_==0;}
    Word* data()noexcept{return storage_.get();}const Word* data()const noexcept{return storage_.get();}
    Word* begin()noexcept{return data();}const Word* begin()const noexcept{return data();}
    Word* end()noexcept{return data()?data()+size_:nullptr;}const Word* end()const noexcept{return data()?data()+size_:nullptr;}
    Word& operator[](std::size_t n){return at(n);}const Word& operator[](std::size_t n)const{return at(n);}
    Word& at(std::size_t n){if(n>=size_)throw std::out_of_range("Binding output logical word");return data()[n];}
    const Word& at(std::size_t n)const{if(n>=size_)throw std::out_of_range("Binding output logical word");return data()[n];}
    void clear()noexcept{size_=0;}
    void swap(BindingOutputWords& other)noexcept{storage_.swap(other.storage_);std::swap(size_,other.size_);std::swap(capacity_,other.capacity_);}
    void push_back(Word word){
        if(size_==capacity_){
            constexpr auto maximum=std::numeric_limits<std::size_t>::max()/sizeof(Word);
            if(size_>maximum/2)throw std::length_error("Binding output allocation exceeds native safe extent");
            const auto next=size_?size_*2:1;
            std::unique_ptr<Word[]> allocation(new Word[next]); //No value initialization of unused allocation bytes.
            if(size_)std::memcpy(allocation.get(),data(),size_*sizeof(Word));
            allocation[size_]=word;storage_=std::move(allocation);capacity_=next;
        }else data()[size_]=word;
        ++size_;
    }
    void resize(std::size_t count,Word word=0){
        if(count<size_){size_=count;return;}
        while(size_<count)push_back(word); //Exact end-insertion growth sequence; do not bulk value-initialize spare capacity.
    }
private:
    std::unique_ptr<Word[]> storage_;std::size_t size_=0,capacity_=0;
};
}
#endif
