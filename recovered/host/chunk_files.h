#ifndef HD2_HOST_CHUNK_FILES_H
#define HD2_HOST_CHUNK_FILES_H

#include "io/chunk_file.h"
#include "io/resource_observations.h"
#include <windows.h>

namespace hd2::host {

// Borrow the SAME original rw_data module used by archive registration and
// engine imports. Exported file/search/codec implementations remain delegated.
// Resolving the writer API does not open or write files; only explicit callers
// can invoke those operations. No game-side file operation is agent-executed.
class ChunkFiles {
public:
    explicit ChunkFiles(HMODULE module)
        : open_(resolve<Open>(module, "_dtaOpen@8", 0x12c1)),
          open_write_(resolve<Open>(module, "_dtaOpenWrite@8", 0x145b)),
          seek_(resolve<Seek>(module, "_dtaSeek@12", 0x1528)),
          read_(resolve<Read>(module, "_dtaRead@12", 0x152d)),
          write_(resolve<Write>(module, "_dtaWrite@12", 0x101e)),
          close_(resolve<Close>(module, "_dtaClose@4", 0x14f1)) {
        static_assert(sizeof(void*) == 4, "Original chunk file ABI requires i386");
    }
    std::uint32_t open(const char* name, std::uint32_t flags) { const auto result=open_(name, flags); observations_.opened(name,flags,result); return result; }
    std::uint32_t open_write(const char* name, std::uint32_t flags) { return open_write_(name, flags); }
    std::uint32_t seek(std::uint32_t handle, std::uint32_t offset, std::uint32_t origin) {
        const auto result=seek_(handle, offset, origin); observations_.sought(handle,offset,origin,result); return result;
    }
    std::uint32_t read(std::uint32_t handle, void* output, std::uint32_t count) { const auto result=read_(handle, output, count); observations_.read(handle,output,count,result); return result; }
    std::uint32_t write(std::uint32_t handle, const void* input, std::uint32_t count) { return write_(handle, input, count); }
    void close(std::uint32_t handle) { close_(handle); observations_.closed(handle); }
    auto resource_observations()const noexcept { return observations_.store(); }
    using ObservationScope=io::ResourceObservationScope<ChunkFiles>;
    //Scope must retire before this process file owner. Stores themselves contain
    //no file/World borrowers and may survive it. Replacement is never reverted.
    auto observe_scope(std::shared_ptr<io::ResourceObservations> store){return ObservationScope(*this,std::move(store));}
    void observe_resources(std::shared_ptr<io::ResourceObservations> store)noexcept { observations_.bind(std::move(store)); }
private:
    io::ResourceOperationObserver observations_;
    template<class Function>
    static Function resolve(HMODULE module, const char* name, std::uintptr_t rva) {
        const auto entry = module ? GetProcAddress(module, name) : nullptr;
        if (!entry || reinterpret_cast<std::uintptr_t>(entry) != reinterpret_cast<std::uintptr_t>(module) + rva)
            throw std::runtime_error("Unsupported original chunk-file export");
        Function result = nullptr;
        static_assert(sizeof(result) == sizeof(entry));
        std::memcpy(&result, &entry, sizeof(result));
        return result;
    }
    using Open = std::uint32_t (WINAPI *)(const char*, std::uint32_t);
    using Seek = std::uint32_t (WINAPI *)(std::uint32_t, std::uint32_t, std::uint32_t);
    using Read = std::uint32_t (WINAPI *)(std::uint32_t, void*, std::uint32_t);
    using Write = std::uint32_t (WINAPI *)(std::uint32_t, const void*, std::uint32_t);
    using Close = void (WINAPI *)(std::uint32_t);
    Open open_, open_write_;
    Seek seek_;
    Read read_;
    Write write_;
    Close close_;
};

}
#endif
