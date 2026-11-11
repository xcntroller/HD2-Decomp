#ifndef HD2_IO_DATA_OPERATION_H
#define HD2_IO_DATA_OPERATION_H
#include "io/resource_observations.h"
#include <exception>
#include <type_traits>
#include <utility>
namespace hd2::io {
//Records source exceptions verbatim and rethrows the SAME exception. Diagnostics
//do not classify these as native engine failures or turn false into success.
template<class F>decltype(auto) observe_data_operation(const std::shared_ptr<ResourceObservations>& store,const char* operation,F&& call){
    if(store)store->event("execution","entered",operation);
    try{
        if constexpr(std::is_void_v<std::invoke_result_t<F>>){std::forward<F>(call)();if(store)store->event("execution","completed",operation);return;}
        else {auto result=std::forward<F>(call)();if(store)store->event("execution","returned",operation,(sizeof(result)<=4?std::uint64_t(static_cast<std::uint32_t>(result)):static_cast<std::uint64_t>(result)));return result;}
    }catch(const std::exception& e){if(store)store->event("source-exception",operation,e.what());throw;}
    catch(...){if(store)store->event("source-exception",operation,"non-standard exception");throw;}
}
}
#endif
