#ifndef HD2_HOST_GAME_SCENE_OBJECTS_H
#define HD2_HOST_GAME_SCENE_OBJECTS_H

#include "host/game_scene.h"
#include "host/chunk_files.h"
#include "host/loaded_resources.h"
#include "game/mission_scene_reader.h"
#include "game/lightmap_payload.h"
#include "scene/model_resource_key.h"
#include <algorithm>
#include <functional>
#include <limits>

namespace hd2::host {

//Actual engine frame/resource endpoints for006CD940. The containing world adds
//actor/table/module owners; this class supplies no actor or module substitutes.
//All borrowed names, mesh views and animated-model references must be discarded
//before the containing scene is reset. No readiness is published by this bridge.
class GameSceneObjects {
public:
    using Frame = EngineFrame*;
    GameSceneObjects(GameScene& scene, MenuDriver& driver, HMODULE module, Graphics& graphics,
                     ChunkFiles& files, LoadedResources& resources,
                     std::function<void(unsigned,float)> progress)
        : scene_(scene), driver_(driver), module_(module), graphics_(graphics), files_(files),
          resources_(resources), progress_(std::move(progress)) {}

    template<class Read> bool read_scene_document(const std::string& path, Read&& read) {
        if (document_) throw std::logic_error("Nested mission document read");
        const auto handle = files_.open(path.c_str(), 0);
        if (handle == 0xffffffffu) return false;
        struct Document {
            ChunkFiles& files;
            std::uint32_t handle;
            std::vector<unsigned char> bytes;
            ~Document() { files.close(handle); }
        } document{files_, handle, {}};
        const auto size = files_.seek(handle, 0, 2);
        if (size == 0xffffffffu || size > std::uint32_t(std::numeric_limits<std::int32_t>::max()))
            throw std::runtime_error("Invalid mission document size");
        if (files_.seek(handle, 0, 0) != 0) throw std::runtime_error("Cannot rewind mission document");
        document.bytes.resize(size);
        if (size && files_.read(handle, document.bytes.data(), size) != size)
            throw std::runtime_error("Cannot read mission document");
        DocumentView view{handle, document.bytes.data(), document.bytes.size()};
        document_ = &view;
        struct Reset { GameSceneObjects& owner; ~Reset() { owner.document_ = nullptr; } } reset{*this};
        read(game::SerializedBytes(document.bytes));
        scene_.rethrow_callback_failure();
        return true;
    }
    void scene_progress(unsigned stage, float fraction) { progress_(stage, fraction); }
    Frame find_scene_frame(const std::string& name, std::uint32_t mask) { return scene_.find_frame(name, mask); }
    Frame resolve_scene_path(const std::string& name) { return scene_.resolve_frame_path(name); }
    std::string frame_name(Frame frame) const { const auto* name = engine_frame_name(frame); return name ? name : ""; }
    std::uint32_t frame_kind(Frame frame) const { return field<std::uint16_t>(frame, 0xf8); }
    std::uint32_t visual_type(Frame frame) const { return call<std::uint32_t>(frame, 4); }
    Frame object_parent(Frame frame) const { return field<Frame>(frame, 0x108); }
    void update_scene_object(Frame frame) { call<void>(frame, 0x24); }
    void object_visible(Frame frame, bool value) { call<void>(frame, 0x30, std::uint32_t(value)); }
    void object_position(Frame frame, const game::MissionVector& value) { //0041D110.
        const auto flags = field<std::uint32_t>(frame, 0xe0);
        if (!(flags & 0x80u)) {
            put(frame, 0xb0, value);
            put(frame, 0xe0, (flags & 0xfffffedfu) | 0x40000000u);
        }
    }
    void object_rotation(Frame frame, const game::MissionQuaternion& value) { //0041D180.
        put(frame, 0xc0, value);
        using Normalize = void (WINAPI *)(void*);
        exported<Normalize>("?Normalize@S_quat@@QAGXXZ", 0x30540)(address(frame, 0xc0));
        put(frame, 0xe0, (field<std::uint32_t>(frame, 0xe0) & 0xfffffecbu) | 0x40000008u);
    }
    void object_scale(Frame frame, const game::MissionVector& value) { //005DE200.
        put(frame, 0xd0, value);
        const auto flags = field<std::uint32_t>(frame, 0xe0);
        if (!(flags & 8u)) {
            using Make = void (WINAPI *)(void*, const void*);
            exported<Make>("?Make@S_quat@@QAGXABUS_matrix@@@Z", 0x2f220)(address(frame, 0xc0), address(frame, 0x80));
        }
        put(frame, 0xe0, (flags & 0xfffffecbu) | 0x40000008u);
    }
    void link_scene_object(Frame frame, Frame parent) {
        if (call<std::int32_t>(frame, 0x38, parent, std::uint32_t{0}) < 0)
            throw std::runtime_error("Cannot link mission frame");
    }
    void auto_parent_object(Frame frame) {
        if (!(field<std::uint32_t>(frame, 0xe0) & 0x20u)) {
            using UpdateWorld = void (__thiscall *)(Frame);
            exported<UpdateWorld>("?UpdateWMatrixProc@I3D_frame@@AAEXXZ", 0x1e390)(frame);
        }
        call<void>(scene_.frame(), 0xec, frame, static_cast<const void*>(address(frame, 0x70)));
    }
    Frame create_scene_object(std::uint32_t kind, std::uint32_t subtype, const std::string& name) {
        auto owner = kind == 1 ? create_visual(subtype) : driver_.create(kind);
        auto* frame = owner.get();
        if (kind == 3) camera_fov(frame, 1.39626348f);
        if (call<std::int32_t>(frame, 0x34, name.c_str()) < 0)
            throw std::runtime_error("Cannot name mission frame");
        call<void>(scene_.frame(), 0x80, frame); //Scene retains; release local ref on return.
        return frame;
    }
    void load_scene_model(Frame frame, const std::string& name) {
        if (resources_.load_model(frame, recovered::model_resource_key(name), 0) != 0)
            throw std::runtime_error("Cannot load required mission model: " + name);
    }
    void remove_model(Frame frame) { call<void>(scene_.frame(), 0x84, frame); }
    Frame replace_scene_visual(Frame old, std::uint32_t type) {
        auto owner = create_visual(type);
        auto* frame = owner.get();
        if (call<std::int32_t>(frame, 0x4c, old) < 0)
            throw std::runtime_error("Cannot copy required mission visual");
        link_scene_object(frame, object_parent(old));
        while (call<std::uint32_t>(old, 0x3c)) {
            auto* child = call<Frame>(old, 0x40, std::uint32_t{0});
            if (!child) throw std::runtime_error("Missing child during visual replacement");
            link_scene_object(child, frame);
        }
        update_scene_object(frame);
        link_scene_object(old, nullptr);
        auto* model = object_parent(frame);
        while (model && frame_kind(model) != 9) model = object_parent(model);
        if (model) {
            call<void>(model, 0x80, old);
            call<void>(model, 0x7c, frame);
        } else {
            call<void>(scene_.frame(), 0x84, old);
            call<void>(scene_.frame(), 0x80, frame);
        }
        return frame;
    }
    bool model_animation(Frame frame) { return call<Frame>(frame, 0x88, std::uint32_t{0}) != nullptr; }
    void register_animated_model(Frame frame) { //0069A2E0, native owned references.
        for (const auto& value : animated_) if (value.get() == frame) return;
        if (call<std::uint32_t>(frame, 0xd0)) animated_.push_back(retain(frame));
    }
    void clear_scene_object_references() { animated_.clear(); }
    void camera_fov(Frame frame, float value) { call<void>(frame, 0x74, value); update_scene_object(frame); }
    void camera_range(Frame frame, float near_clip, float far_clip) { call<void>(frame, 0x7c, near_clip, far_clip); }
    void orthographic_camera() {
        const auto frame = field<Frame>(scene_.frame(), 0x15c);
        if (frame) { call<void>(frame, 0x88, std::uint32_t{1}); update_scene_object(frame); }
    }
    void scene_background(const game::MissionVector& value) { put(scene_.frame(), 0x210, value); }
    void scene_range(const std::array<float, 2>& values) { call<void>(scene_.frame(), 0xe4, values[0], values[1]); }
    void scene_driver_flag(std::uint8_t value) {
        if (std::uint8_t(driver_.render_state(1) != 0) != value)
            if (driver_.set_render_state(1, value) < 0) throw std::runtime_error("Cannot set mission driver flag");
    }
    void scene_graphics_word(std::uint32_t value) {
        using Set = void (WINAPI *)(Frame, std::uint32_t, std::uint32_t);
        checked_engine_method<Set>(reinterpret_cast<Frame>(&graphics_), module_, 0x74)(
            reinterpret_cast<Frame>(&graphics_), 0xc, value);
    }
    void register_scene_frame(Frame frame) { call<void>(scene_.frame(), 0x128, frame); }
    void dummy_bounds(Frame frame, const std::array<float, 6>& values) {
        //Serialized vectors are12 bytes each; engine S_vector has an ignored
        //fourth lane. Never copy the original reader's uninitialized padding.
        const std::array<float, 8> bounds{{values[0],values[1],values[2],0,values[3],values[4],values[5],0}};
        put(frame, 0x120, bounds);
    }
    void clear_light_sectors(Frame frame) {
        while (field<std::uint32_t>(frame, 0x128)) {
            auto* list = field<Frame*>(frame, 0x120);
            if (!list || !list[0]) throw std::runtime_error("Invalid mission light sector list");
            call<void>(list[0], 0x78, frame);
        }
    }
    void light_type(Frame frame, std::uint32_t value) { call<void>(frame, 0x74, value); }
    void light_colours(Frame frame, const std::array<float, 9>& values) {
        for (std::size_t i = 0; i != 3; ++i) call<void>(frame, 0x7c + i * 4, values.data() + i * 3);
    }
    void light_power(Frame frame, const std::array<float, 3>& values) {
        for (std::size_t i = 0; i != 3; ++i) call<void>(frame, 0x88 + i * 4, values[i]);
    }
    void light_cone(Frame frame, const std::array<float, 2>& values) { call<void>(frame, 0xa4, values[0], values[1]); }
    void light_ranges(Frame frame, const std::array<float, 6>& values) {
        call<void>(frame, 0x98, values[0]);
        call<void>(frame, 0x9c, values[1], values[2], values[3]);
    }
    void light_flags(Frame frame, std::uint32_t value) { call<void>(frame, 0x94, value); }
    void add_light_sector(Frame light, Frame sector) { call<void>(sector, 0x74, light); }
    void begin_scene_sound(Frame frame) { call<void>(frame, 0x90, std::uint32_t{0}); object_visible(frame, false); }
    void load_scene_sound(Frame frame, const std::string& name) {
        if (resources_.load_sound(frame, name, 0x10) != 0)
            throw std::runtime_error("Cannot load required mission sound: " + name);
    }
    void sound_scalar(Frame frame, std::uint16_t id, std::uint32_t value) {
        const auto slot = id == 0x4061 ? 0x7cu : id == 0x4062 ? 0x8cu : id == 0x4063 ? 0x88u : 0x98u;
        //All slots take one32-bit scalar; passing bits also preserves floats.
        call<void>(frame, slot, value);
    }
    void sound_cone(Frame frame, const std::array<float, 2>& values) { call<void>(frame, 0x84, values[0], values[1]); }
    void sound_ranges(Frame frame, const std::array<float, 4>& values) { call<void>(frame, 0x80, values[0],values[1],values[2],values[3]); }
    void sound_enable(Frame frame) { call<void>(frame, 0x90, std::uint32_t{1}); }
    void sound_target(Frame sound, Frame target, std::uint32_t kind) { call<void>(target, kind == 5 ? 0x7c : 0x78, sound); }
    void finish_scene_sound(Frame frame, bool loop) { object_visible(frame, loop); }
    void area_geometry(Frame frame, const std::vector<game::MissionVector>& vertices,
                       const std::vector<std::array<std::uint16_t, 3>>& triangles) {
        std::vector<std::array<float, 4>> aligned;
        aligned.reserve(vertices.size());
        for (const auto& v : vertices) aligned.push_back({v[0],v[1],v[2],0}); //006D1B80.
        if (call<std::int32_t>(frame, 0x74, aligned.data(), std::uint32_t(aligned.size()),
                              triangles.data(), std::uint32_t(triangles.size()), std::uint32_t{16}) < 0)
            throw std::runtime_error("Cannot construct mission area geometry");
    }
    void visual_environment(Frame frame, std::uint16_t id, std::uint32_t bits) {
        if (frame_kind(frame) != 1 || visual_type(frame) != 0x534e454cu)
            throw std::runtime_error("Environment payload requires actual lens-flare visual");
        const auto offset = id == 0xb112 ? 0x1e4u : id == 0xb113 ? 0x1e0u : id == 0xb114 ? 0x1ecu :
                            id == 0xb115 ? 0x220u : id == 0xb116 ? 0x224u : 0x240u;
        if (id == 0xb114) {
            float value; std::memcpy(&value, &bits, sizeof(value));
            if (!std::isfinite(value) || value < -2147483648.f || double(value) >= 2147483648.)
                throw std::runtime_error("Invalid lens-flare integer value");
            put(frame, offset, static_cast<std::int32_t>(value));
        } else put(frame, offset, bits);
    }
    void read_scene_lightmap(Frame frame, game::SerializedBytes& payload) {
        if (!document_) throw std::logic_error("Lightmap requires its live archive document");
        const auto* start = payload.take(0);
        const auto size = payload.remaining();
        if (start < document_->begin || std::size_t(start - document_->begin) > document_->size ||
            size > document_->size - std::size_t(start - document_->begin))
            throw std::logic_error("Lightmap lies outside its mission document");
        const MeshMetadata geometry(frame, module_);
        game::validate_lightmap_payload(payload, geometry);
        const auto offset = static_cast<std::uint32_t>(start - document_->begin);
        if (files_.seek(document_->handle, offset, 0) != offset) throw std::runtime_error("Cannot seek mission lightmap");
        if (call<std::int32_t>(frame, 0xbc, document_->handle) < 0)
            throw std::runtime_error("Original engine rejected required mission lightmap");
        if (files_.seek(document_->handle, 0, 1) != offset + size)
            throw std::runtime_error("Engine lightmap read disagrees with bounded payload");
        payload.take(size);
    }
protected:
    template<class T> static T field(const void* frame, std::size_t offset) {
        if (!frame) throw std::runtime_error("Missing mission engine object");
        T value; std::memcpy(&value, static_cast<const unsigned char*>(frame) + offset, sizeof(value)); return value;
    }
    static unsigned char* address(void* frame, std::size_t offset) { return static_cast<unsigned char*>(frame) + offset; }
    template<class T> static void put(void* frame, std::size_t offset, const T& value) {
        std::memcpy(address(frame, offset), &value, sizeof(value));
    }
    template<class Result, class... Args> Result call(Frame frame, std::size_t slot, Args... args) const {
        using Function = Result (WINAPI *)(Frame, Args...);
        return checked_engine_method<Function>(frame, module_, slot)(frame, args...);
    }
    template<class Function> Function exported(const char* name, std::uintptr_t rva) const {
        const auto entry = GetProcAddress(module_, name);
        if (reinterpret_cast<std::uintptr_t>(entry) != reinterpret_cast<std::uintptr_t>(module_) + rva)
            throw std::runtime_error("Unsupported mission engine export");
        Function function; std::memcpy(&function, &entry, sizeof(function)); return function;
    }
    MenuDriver::Owner retain(Frame frame) const {
        const auto count = field<std::uint16_t>(frame, 0xe4);
        if (count == 0xffffu) throw std::runtime_error("Mission frame reference count overflow");
        put(frame, 0xe4, static_cast<std::uint16_t>(count + 1));
        return {frame, scene_method<MenuDriver::Release>(frame, module_, 0, 0x1c370)};
    }
    GameScene& scene_;
    MenuDriver& driver_;
    HMODULE module_;
    Graphics& graphics_;
    ChunkFiles& files_;
    LoadedResources& resources_;
    MenuDriver::Owner create_visual(std::uint32_t subtype) const {
        auto* frame = call<Frame>(driver_.frame(), 0x50, subtype);
        if (!frame) throw std::runtime_error("Original visual factory returned null");
        return {frame, scene_method<MenuDriver::Release>(frame, module_, 0, 0x1c370)};
    }
private:
    class MeshMetadata {
    public:
        MeshMetadata(Frame frame, HMODULE module) : mesh_(field<Frame>(frame, 0x1e0)), module_(module) {
            if (!mesh_) throw std::runtime_error("Required lightmap visual has no mesh");
        }
        std::uint32_t lod_count() const { return field<std::uint32_t>(mesh_, 0x44); }
        std::uint32_t vertices(std::uint32_t index) const { return field<std::uint32_t>(lod(index), 0x54); }
        std::uint32_t faces(std::uint32_t index) const {
            auto* value = lod(index);
            using Count = std::uint32_t (WINAPI *)(Frame);
            return checked_engine_method<Count>(value, module_, 0x10)(value);
        }
        std::uint32_t groups(std::uint32_t index) const { return field<std::uint32_t>(lod(index), 0x58); }
        std::uint32_t group_faces(std::uint32_t index, std::uint32_t group) const {
            auto* current = field<const void*>(lod(index), 0x5c);
            for (std::uint32_t i = 0; i != group; ++i) current = field<const void*>(current, 0xc);
            return field<std::uint32_t>(current, 8);
        }
    private:
        Frame lod(std::uint32_t index) const {
            if (index >= lod_count()) throw std::runtime_error("Invalid lightmap LOD index");
            return field<Frame>(mesh_, 0x48 + std::size_t(index) * 4);
        }
        Frame mesh_;
        HMODULE module_;
    };
    struct DocumentView { std::uint32_t handle; const unsigned char* begin; std::size_t size; };
    DocumentView* document_ = nullptr;
    std::function<void(unsigned,float)> progress_;
    std::vector<MenuDriver::Owner> animated_;
};

} // namespace hd2::host
#endif
