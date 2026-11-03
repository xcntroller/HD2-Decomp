#ifndef HD2_INPUT_CLIENT_INPUT_H
#define HD2_INPUT_CLIENT_INPUT_H

#include "input/binding_defaults.h"
#include "frontend/original_abi.h"
#include <cstdlib>
#include <new>
#include <type_traits>

namespace hd2::input::client {

// Source-only original ABI entries. The checked host InputManager remains a
// separate adapter. These objects have no automatic ownership: initialize once,
// destroy explicitly, and do not copy an owning instance. Device calls below
// are real unchecked interface calls, not successful fallback implementations.
// The allocator boundary represents 007E520F/007E1CC0; its private heap is
// delegated. 007E81D9/007EF932 can RETURN NULL after a missing/false new-handler;
// throwing C++ operator new is not a substitute for that contract.
inline int (*allocation_failure)(std::size_t)=nullptr;
inline void* allocate_storage(std::size_t bytes) {
    if(bytes>0xffffffe0u) return nullptr;
    const auto rounded=((bytes ? bytes : 1)+15u)&~std::size_t{15};
    do {
        if(auto* storage=std::malloc(rounded)) return storage;
    } while(allocation_failure && allocation_failure(bytes));
    return nullptr;
}
struct Allocation {
    void* (*allocate)(std::size_t) = allocate_storage;
    void (*release)(void*) = [](void* p) { std::free(p); };
};
inline Allocation allocation;

template<class T> struct Vector {
    T* begin;
    T* end;
    T* limit;
    std::size_t size() const { return begin ? static_cast<std::size_t>(end-begin) : 0; }
    std::size_t capacity() const { return begin ? static_cast<std::size_t>(limit-begin) : 0; }
    void reset() { begin=nullptr;end=nullptr;limit=nullptr; }
    void clear() { end=begin; }
    // End insertion only: the callers in this batch never insert in the middle.
    // Original growth is size+max(size,1), with raw relocation and no AddRef.
    // A new-handler may throw before mutation. A null result instead skips only
    // copies whose destination is zero, then publishes the original raw pointer
    // values. Later low-address accesses retain original memory preconditions.
    void append(const T& value) {
        static_assert(std::is_trivially_copyable_v<T>);
        const auto count=size();
        if(end==limit) {
            const auto next=count+(count ? count : 1);
            // Original i386 byte multiplication wraps as a DWORD. Malformed
            // sizes retain the original raw-memory preconditions.
            const auto bytes=static_cast<std::uint32_t>(next*sizeof(T));
            auto* replacement=static_cast<T*>(allocation.allocate(bytes));
            const auto address=reinterpret_cast<std::uintptr_t>(replacement);
            // Use integer address arithmetic for the original null-plus-offset
            // results. C++ pointer arithmetic on null would itself be undefined.
            const auto span=(reinterpret_cast<std::uintptr_t>(end)-
                             reinterpret_cast<std::uintptr_t>(begin))/sizeof(T);
            for(std::size_t i=0;i<span;++i) {
                auto* destination=reinterpret_cast<T*>(address+i*sizeof(T));
                if(destination) {
                    ::new (static_cast<void*>(destination)) T;
                    const auto source=reinterpret_cast<const T*>(
                        reinterpret_cast<std::uintptr_t>(begin)+i*sizeof(T));
                    std::memcpy(destination,source,sizeof(T));
                }
            }
            auto* destination=reinterpret_cast<T*>(address+span*sizeof(T));
            if(destination) {
                ::new (static_cast<void*>(destination)) T;
                std::memcpy(destination,&value,sizeof(T));
            }
            allocation.release(begin);
            limit=reinterpret_cast<T*>(address+next*sizeof(T));
            begin=replacement;
            end=reinterpret_cast<T*>(address+(count+1)*sizeof(T));
        } else {
            if(end) {
                ::new (static_cast<void*>(end)) T;
                std::memcpy(end,&value,sizeof(T));
            }
            end=reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(end)+sizeof(T));
        }
    }
    void free_storage() {
        allocation.release(begin); // Null is passed to delete, too.
        reset();
    }
};

template<class Function> inline Function method(void* object,std::size_t slot) {
    return reinterpret_cast<Function>((*static_cast<void***>(object))[slot/4]);
}
inline void retain(void* device) {
    using F=std::uint32_t (HD2_CLIENT_THISCALL *)(void*);
    method<F>(device,4)(device);
}
inline void release(void* device) {
    using F=std::uint32_t (HD2_CLIENT_THISCALL *)(void*);
    method<F>(device,8)(device);
}

struct InputManager {
    void* graphics;
    std::uint8_t joystick_tag;
    std::array<std::uint8_t,3> joystick_padding;
    Vector<JoystickInput> joysticks;
    std::array<std::uint8_t,12> reserved14;
    std::array<std::uint8_t,64> scans,key_counts;
    std::array<std::uint8_t,6> mouse_counts;
    std::array<std::uint8_t,2> reserved_a6;
    std::array<float,3> axis_limits;
    std::uint8_t attachment_flag;
    std::array<std::uint8_t,3> reserved_b5;
    std::uint8_t binding_tag;
    std::array<std::uint8_t,3> binding_padding;
    Vector<InputBinding> bindings;
    std::uint8_t first_tag;
    std::array<std::uint8_t,3> first_padding;
    Vector<std::uint32_t> first;
    std::uint8_t second_tag;
    std::array<std::uint8_t,3> second_padding;
    Vector<std::uint32_t> second;
    std::uint64_t key_gate;
    std::uint32_t mouse_gate;

    //006C6A20: preserve all bytes not explicitly written, even in placement
    // storage. The four allocator bytes come from the high byte of saved THIS.
    HD2_CLIENT_BODY InputManager* HD2_CLIENT_THISCALL initialize() {
        const auto tag=static_cast<std::uint8_t>(reinterpret_cast<std::uintptr_t>(this)>>24);
        joystick_tag=tag;joysticks.reset();
        binding_tag=tag;bindings.reset();
        first_tag=tag;first.reset();second_tag=tag;second.reset();
        axis_limits={{20.0f,20.0f,20.0f}};graphics=nullptr;
        return this;
    }
    void clear_counts() {
        scans.fill(0);key_counts.fill(0);
        std::fill_n(mouse_counts.begin(),4,std::uint8_t{0});
    }
    //006C6FF0: no map publication and no reference operations.
    HD2_CLIENT_BODY void HD2_CLIENT_THISCALL clear_bindings() {
        bindings.clear();first.clear();second.clear();clear_counts();
    }
    //006C6B60: initial count, freshly resolved base for each reverse callback.
    HD2_CLIENT_BODY void HD2_CLIENT_THISCALL detach() {
        bindings.clear();first.clear();second.clear();
        for(auto n=joysticks.size();n;--n) {
            auto* device=joysticks.begin[n-1].device;
            if(device) release(device);
        }
        joysticks.clear();
    }
    //006C6AB0 normal path. Original MSVC SEH cleanup remains separate work;
    // this method does not claim the original exception-registration ABI.
    HD2_CLIENT_BODY void HD2_CLIENT_THISCALL destroy() {
        detach();second.free_storage();first.free_storage();
        bindings.free_storage();joysticks.free_storage();
    }
    //006C6CC0: raw count/array preconditions, reverse validation before mutation,
    // forward retaining and appending. An append failure leaves the new retain
    // outstanding, as in the original. The checked host owner still balances it.
    HD2_CLIENT_BODY bool HD2_CLIENT_THISCALL attach(void* graph,void* const* devices,std::int32_t count) {
        if(!graph) return false;
        for(auto n=count;n;--n) if(!devices[n-1]) return false;
        clear_counts();graphics=graph;
        for(std::int32_t i=0;i<count;++i) {
            JoystickInput record{};record.device=devices[i];
            retain(record.device);joysticks.append(record);
        }
        key_gate=~std::uint64_t{0};
        // Same FILD/FSTP sequence as006C6C90, without an invented x87 CW change.
        std::int32_t x=400,y=400,z=1;
#if (defined(__i386__) || defined(__x86_64__)) && (defined(__GNUC__) || defined(__clang__))
        __asm__ volatile("fildl %3\n\tfstps %0\n\tfildl %4\n\tfstps %1\n\tfildl %5\n\tfstps %2"
            : "=m"(axis_limits[0]),"=m"(axis_limits[1]),"=m"(axis_limits[2])
            : "m"(x),"m"(y),"m"(z) : "st");
#else
        axis_limits={{static_cast<float>(x),static_cast<float>(y),static_cast<float>(z)}};
#endif
        attachment_flag=0;return true;
    }
    std::size_t scan_slot(std::uint8_t scan) const {
        return static_cast<std::size_t>(std::find(scans.begin(),scans.end(),scan)-scans.begin());
    }
    void publish_scans() {
        using F=std::int32_t (HD2_CLIENT_STDCALL *)(void*,const std::uint8_t*,std::uint32_t);
        method<F>(graphics,0xb4)(graphics,scans.data(),64);
    }
    //006C7480: original replacement-pointer/index stack order, AL/RET8.
    HD2_CLIENT_BODY bool HD2_CLIENT_THISCALL install(const InputBinding* replacement,std::int32_t index) {
        if(index<0) return false;
        const auto slot=static_cast<std::uint32_t>(index);
        if(slot>=bindings.size()) {
            while(slot>=bindings.size()) bindings.append(InputBinding{});
            while(slot>=first.size()) first.append(0);
            while(slot>=second.size()) second.append(0);
        }
        if(bindings.begin[slot].device_kind==1) {
            const auto source=scan_slot(static_cast<std::uint8_t>(bindings.begin[slot].code));
            if(source<64 && --key_counts[source]==0) {
                scans[source]=0;publish_scans();
            }
        }
        // Reread after the graphics callback; it may have changed the old kind.
        if(bindings.begin[slot].device_kind==2) {
            const auto source=binding_mouse_slot(bindings.begin[slot].code);
            if(source<5) --mouse_counts[source];
        }
        if(!replacement) { bindings.begin[slot]={};return true; }
        const auto kind=replacement->device_kind;
        if(kind==1) {
            const auto scan=static_cast<std::uint8_t>(replacement->code);
            auto source=scan_slot(scan);
            if(source==64) {
                source=scan_slot(0);if(source==64) return false;
                scans[source]=scan;publish_scans();
            }
            ++key_counts[source];
        } else if(kind==2) {
            const auto source=binding_mouse_slot(replacement->code);
            if(source<5) ++mouse_counts[source];
        } else if(kind==3) {
            if(replacement->device_index>=joysticks.size()) return false;
            const auto flags=replacement->direction==3 && replacement->code<0xff ? 4u : 0u;
            auto* device=joysticks.begin[replacement->device_index].device;
            using F=std::uint8_t (HD2_CLIENT_THISCALL *)(void*,std::uint32_t,std::uint32_t);
            method<F>(device,0x30)(device,replacement->code,flags);
        }
        bindings.begin[slot]=*replacement;
        return true;
    }
    //006C7440: ECX is the caller's manager, not an implicit global lookup.
    HD2_CLIENT_BODY bool HD2_CLIENT_THISCALL set(std::int32_t index,std::uint32_t kind,
            std::uint32_t code,std::uint32_t device,std::uint32_t direction) {
        const InputBinding record{direction,static_cast<std::uint16_t>(kind),
                                  static_cast<std::uint16_t>(code),device};
        return install(&record,index);
    }
};

// Original008AEA48, relocated as a C++ data symbol. Static zero-initialization
// precedes its constructor, as in006D5D50. Native registration below is not a
// recovery claim for the original CRT or the destructor's MSVC SEH machinery.
inline InputManager global_input_manager{};
struct GlobalInputLifetime {
    GlobalInputLifetime() { global_input_manager.initialize(); }
    ~GlobalInputLifetime() { global_input_manager.destroy(); }
};
inline GlobalInputLifetime global_input_lifetime;

struct Setting {
    std::int32_t index;
    std::uint16_t device_kind,code;
    std::uint32_t direction,mode,group,caption;
    std::uint8_t dirty;
    std::array<std::uint8_t,3> padding; // Opaque; record construction never writes it.
};
static_assert(sizeof(Setting)==0x1c && offsetof(Setting,dirty)==0x18);

struct BindingConfiguration {
    std::uint8_t tag;
    std::array<std::uint8_t,3> padding;
    Vector<Setting> settings;
    std::uint32_t mode;
    //006A81C0 returns EAX=this, writes only byte0 of the allocator prefix.
    HD2_CLIENT_BODY BindingConfiguration* HD2_CLIENT_THISCALL initialize() {
        tag=static_cast<std::uint8_t>(reinterpret_cast<std::uintptr_t>(this)>>24);
        settings.reset();mode=2;return this;
    }
    //006A8600 is a thiscall method (receiver unused), not a free cdecl call.
    HD2_CLIENT_BODY void HD2_CLIENT_THISCALL apply_setting(Setting* setting) {
        global_input_manager.set(setting->index,setting->device_kind,setting->code,0,setting->direction);
        setting->dirty=0;
    }
    HD2_CLIENT_BODY void HD2_CLIENT_THISCALL apply_mode(std::uint32_t selected) {
        for(auto n=settings.size();n;--n)
            if(settings.begin[n-1].mode==selected) apply_setting(settings.begin+n-1);
    }
    HD2_CLIENT_BODY void HD2_CLIENT_THISCALL select_mode(std::uint32_t selected) {
        if(selected!=2 && selected!=3) return;
        apply_mode(selected);mode=selected;
    }
    HD2_CLIENT_BODY void HD2_CLIENT_THISCALL apply_dirty() {
        if(mode!=2 && mode!=3) return;
        for(auto n=settings.size();n;--n) {
            auto* record=settings.begin+n-1;
            if(!record->dirty) continue;
            if(record->mode==2 && mode!=2) continue;
            if(record->mode==3 && mode!=3) continue;
            apply_setting(record);
        }
    }
    void append_defaults(const std::array<hd2::input::BindingSetting,92>& defaults) {
        settings.clear();
        // Like the original stack record, construction writes bytes00..18 and
        // relocation copies all1C bytes, including unconstrained stack padding.
        Setting record;
        for(const auto& value:defaults) {
            std::memcpy(&record,&value,0x19);
            settings.append(record);
        }
        apply_dirty();
    }
    HD2_CLIENT_BODY void HD2_CLIENT_THISCALL primary_defaults() { append_defaults(primary_binding_defaults); }
    HD2_CLIENT_BODY void HD2_CLIENT_THISCALL alternate_defaults() { append_defaults(alternate_binding_defaults); }
    HD2_CLIENT_BODY void HD2_CLIENT_THISCALL choose_defaults(std::uint32_t selector) {
        if(selector==1) alternate_defaults();else primary_defaults();
    }
};

#if defined(__i386__) || defined(_M_IX86)
static_assert(sizeof(Vector<Setting>)==12 && sizeof(BindingConfiguration)==0x14);
static_assert(offsetof(BindingConfiguration,settings)==4 && offsetof(BindingConfiguration,mode)==0x10);
static_assert(offsetof(InputManager,joysticks)==8 && offsetof(InputManager,scans)==0x20);
static_assert(offsetof(InputManager,key_counts)==0x60 && offsetof(InputManager,mouse_counts)==0xa0);
static_assert(offsetof(InputManager,axis_limits)==0xa8 && offsetof(InputManager,attachment_flag)==0xb4);
static_assert(offsetof(InputManager,bindings)==0xbc && offsetof(InputManager,first)==0xcc);
static_assert(offsetof(InputManager,second)==0xdc && offsetof(InputManager,key_gate)==0xe8);
static_assert(offsetof(InputManager,mouse_gate)==0xf0);
#endif
static_assert(std::is_trivially_copyable_v<InputManager> && std::is_trivially_copyable_v<BindingConfiguration>);

}
#endif
