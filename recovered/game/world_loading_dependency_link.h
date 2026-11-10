#ifndef HD2_GAME_WORLD_LOADING_DEPENDENCY_LINK_H
#define HD2_GAME_WORLD_LOADING_DEPENDENCY_LINK_H

#include <stdexcept>

namespace hd2::game {

//Native containing-world construction guard, not an original game service.
//Mutually borrowing consumers may be constructed before the actual dependency
//aggregate. Bind the ONE completed aggregate before invoking them; retire every
//consumer/lifetime before unbinding it. No reference to unconstructed storage
//and no successful unbound method substitutes for a real owner.
template<class Dependencies>class WorldLoadingDependencyLink {
public:
    WorldLoadingDependencyLink()=default;
    explicit WorldLoadingDependencyLink(Dependencies& dependencies):dependencies_(&dependencies){}
    WorldLoadingDependencyLink(const WorldLoadingDependencyLink&)=delete;
    WorldLoadingDependencyLink& operator=(const WorldLoadingDependencyLink&)=delete;
    void bind(Dependencies& dependencies){
        if(dependencies_)throw std::logic_error("World loading dependencies already bound");
        dependencies_=&dependencies;
    }
    void unbind(Dependencies& dependencies)noexcept{if(dependencies_==&dependencies)dependencies_=nullptr;}
    Dependencies& get()const{
        if(!dependencies_)throw std::logic_error("World loading consumer has no completed containing dependency owner");
        return *dependencies_;
    }
    bool bound()const noexcept{return dependencies_!=nullptr;}
private:
    Dependencies* dependencies_=nullptr;
};

} //namespace hd2::game
#endif
