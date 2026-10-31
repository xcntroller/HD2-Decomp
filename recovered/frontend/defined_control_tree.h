#ifndef HD2_FRONTEND_DEFINED_CONTROL_TREE_H
#define HD2_FRONTEND_DEFINED_CONTROL_TREE_H

#include "frontend/container_input.h"
#include "frontend/control_factory.h"
#include "frontend/defined_container_events.h"
#include "frontend/menu_definition.h"
#include "frontend/screen_focus.h"
#include "frontend/screen_input.h"
#include "frontend/frontend_update.h"

#include <limits>
#include <memory>
#include <utility>

namespace hd2::frontend {

// Native ownership for the recovered factory/container subset. Backend and
// immutable definitions, frames and queue outlive the tree. Successful children
// are compacted in original insertion order; definition IDs are never renumbered.
// Leaf constructors, engine ABI and availability policy remain backend contracts.
template<class Backend>
class DefinedControlTree {
public:
    using Frame = typename Backend::Frame;
    using Leaf = typename Backend::Leaf;
    using Owner = std::unique_ptr<DefinedControlTree>;
    struct Result { bool handled; Owner control; };

    static Result construct(Backend& backend, const MenuControlDefinition& definition,
                            Frame search_root, Frame layout_root, ControlEventDestination& queue) {
        Factory factory{backend, definition, search_root, layout_root, queue, {}, {}};
        const bool handled = construct_defined_control(factory);
        return {handled, std::move(factory.result)};
    }
    DefinedControlTree(const DefinedControlTree&) = delete;
    DefinedControlTree& operator=(const DefinedControlTree&) = delete;

    std::uint32_t identifier() const { return definition_.identifier; }
    bool eligible() const { return group_ ? group_->eligible() : leaf_->eligible(); }
    bool disabled() const { return group_ ? group_->disabled() : leaf_->disabled(); }
    std::size_t child_count() const { return children_.size(); }
    DefinedControlTree& child_at(std::size_t index) { return *children_.at(index); }
    Frame frame() const { return frame_; }
    Frame layout_root() const { return layout_root_; }

    bool receive_event(ControlEvent& event, ControlEventDestination& upstream) {
        if (!group_) return leaf_->receive_event(event, upstream);
        EmittingControlDestination<DefinedControlTree> destination{*this, &upstream};
        return dispatch_control_event(event, destination);
    }
    bool dispatch_control_event(ControlEvent& event, ControlEventDestination& destination) {
        return group_ ? group_->dispatch_control_event(event, destination) : leaf_->receive_event(event, destination);
    }
    bool key(std::uint32_t word) { return group_ ? container_key(*this, word) : leaf_->key(word); }
    void update(std::uint32_t milliseconds) {
        if (group_) update_focused_control(*this, milliseconds); // 00656af0.
        else leaf_->update(milliseconds);
    }
    int enter_focus(std::uint32_t target) {
        return group_ ? container_focus_enter(*this, target) : leaf_->apply(true);
    }
    int leave_focus(std::uint32_t target) {
        return group_ ? screen_focus_leave(*this, target) : leaf_->apply(false);
    }
    int pointer_focus(std::uint32_t target) {
        return group_ ? container_pointer_focus(*this, target) : leaf_->pointer_move();
    }
    int activate(std::uint32_t target) {
        return group_ ? container_activate(*this, target) : leaf_->activate(queue_);
    }
    int deactivate(std::uint32_t target) {
        return group_ ? container_deactivate(*this, target) : leaf_->deactivate(queue_);
    }
    int click(std::uint32_t target) { return group_ ? screen_click(*this, target) : leaf_->click(queue_); }
    int wheel(std::uint32_t target, std::int32_t delta) {
        return group_ ? screen_wheel(*this, target, delta) : leaf_->wheel_event(queue_);
    }
    int pointer(ScreenPointerCallback callback, std::uint32_t target) {
        if (!group_) {
            if (callback == ScreenPointerCallback::focus) return pointer_focus(target);
            if (callback == ScreenPointerCallback::click) return click(target);
            return leaf_->pointer_event(callback, queue_);
        }
        switch (callback) {
        case ScreenPointerCallback::focus: return pointer_focus(target);
        case ScreenPointerCallback::press: return screen_press(*this, target);
        case ScreenPointerCallback::release: return screen_release(*this, target);
        case ScreenPointerCallback::drag: return screen_drag(*this, target);
        case ScreenPointerCallback::drag_end: return screen_drag_end(*this, target);
        case ScreenPointerCallback::drag_begin: return screen_drag_begin(*this, target);
        case ScreenPointerCallback::click: return click(target);
        case ScreenPointerCallback::double_click: return screen_double_click(*this, target);
        }
        throw std::logic_error("Unknown container pointer method");
    }

    // Context for the original shared focus/pointer helpers. Resolve children
    // afresh at each callback; unique ownership keeps their addresses stable.
    int focused_control() const { return focused_; }
    void update_control(int index, std::uint32_t milliseconds) { child_at(index).update(milliseconds); }
    void set_focused_control(int index) { focused_ = index; }
    bool valid_control(int index) const {
        return index >= 0 && static_cast<std::size_t>(index) < children_.size();
    }
    int control_for_target(std::uint32_t target) const {
        struct Identifiers {
            const std::vector<Owner>& children;
            std::size_t size() const { return children.size(); }
            std::uint32_t operator[](std::size_t i) const { return children[i]->identifier(); }
        } identifiers{children_};
        return find_screen_control(identifier(), identifiers, target);
    }
    bool control_eligible(int index) const { return children_.at(static_cast<std::size_t>(index))->eligible(); }
    bool key_control(int index, std::uint32_t word) { return child_at(index).key(word); }
    int enter_control(int index, std::uint32_t target) { return child_at(index).enter_focus(target); }
    int leave_control(int index, std::uint32_t target) { return child_at(index).leave_focus(target); }
    int move_control(int index, std::uint32_t target) { return child_at(index).pointer_focus(target); }
    int activate_control(int index, std::uint32_t target) { return child_at(index).activate(target); }
    int deactivate_control(int index, std::uint32_t target) { return child_at(index).deactivate(target); }
    int click_control(int index, std::uint32_t target) { return child_at(index).click(target); }
    int wheel_control(int index, std::uint32_t target, std::int32_t delta) { return child_at(index).wheel(target, delta); }
    int pointer_control(int index, ScreenPointerCallback callback, std::uint32_t target) {
        return child_at(index).pointer(callback, target);
    }

    // Frame and child construction callbacks for 006618a0 / kind-five +2c.
    void bind_frame(Frame frame) { frame_ = frame; }
    void set_frame_identifier(Frame frame, std::uint32_t id) { backend_.set_frame_identifier(frame, id); }
    void set_definition_kind(MenuControlKind kind) {
        if (kind != MenuControlKind::container) throw std::logic_error("Container definition kind changed");
    }
    bool create_child(Frame search_root, Frame layout_root, const MenuControlDefinition& definition) {
        auto result = construct(backend_, definition, search_root, layout_root, queue_);
        if (result.control) {
            if (children_.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
                throw std::length_error("Too many container controls");
            auto* child = result.control.get();
            children_.push_back(std::move(result.control));
            try {
                group_->append_child(definition.identifier,
                    [child](ControlEvent& event, ControlEventDestination& destination) {
                        return child->receive_event(event, destination);
                    });
            } catch (...) { children_.pop_back(); throw; }
        }
        return result.handled;
    }
    void set_frame_visible(std::uint8_t visible) { backend_.set_frame_visible(frame_, visible); }
    void update_frame() { backend_.update_frame(frame_); }

private:
    DefinedControlTree(Backend& backend, const MenuControlDefinition& definition,
                       Frame frame, Frame layout_root, ControlEventDestination& queue)
        : backend_(backend), definition_(definition), frame_(frame), layout_root_(layout_root), queue_(queue) {
        if (definition.kind == MenuControlKind::container)
            group_ = std::make_unique<DefinedContainerEvents<DefinedControlTree>>(identifier(), *this, queue_);
        else {
            leaf_ = backend_.allocate_leaf(definition, frame_, layout_root_);
            if (!leaf_) throw std::bad_alloc();
        }
    }
    bool initialize_visuals() {
        return group_ ? initialize_container_visuals(*this, frame_, layout_root_, &definition_)
                      : leaf_->initialize_visuals();
    }
    void initialize_definition_data() { if (leaf_) leaf_->initialize_definition_data(); }
    void initialize_events() {
        if (group_) group_->initialize(definition_.initialization);
        else leaf_->initialize_events(definition_.initialization);
    }
    struct Factory {
        Backend& backend;
        const MenuControlDefinition& definition;
        Frame search_root;
        Frame layout_root;
        ControlEventDestination& queue;
        Owner candidate;
        Owner result;
        Frame frame{};
        bool find_frame(std::uint32_t flags) { frame = backend.find_frame(search_root, definition.frame, flags); return !!frame; }
        void report_missing_frame() { backend.report_missing_frame(definition); }
        void report_unknown_kind(std::uint32_t kind) { backend.report_unknown_kind(kind); }
        std::uint32_t control_kind() const { return static_cast<std::uint32_t>(definition.kind); }
        bool allocate_control(std::uint32_t) {
            candidate.reset(new DefinedControlTree(backend, definition, frame, layout_root, queue));
            return true;
        }
        bool initialize_visuals() { return candidate->initialize_visuals(); }
        void initialize_definition_data() { candidate->initialize_definition_data(); }
        void initialize_definition_events() { candidate->initialize_events(); }
        void append_control() { result = std::move(candidate); }
        void destroy_control() { candidate.reset(); }
    };
    Backend& backend_;
    const MenuControlDefinition& definition_;
    Frame frame_;
    Frame layout_root_;
    ControlEventDestination& queue_;
    std::unique_ptr<Leaf> leaf_;
    // Event endpoints borrow children; destroy event storage before the owners.
    std::vector<Owner> children_;
    std::unique_ptr<DefinedContainerEvents<DefinedControlTree>> group_;
    int focused_ = -1;
};

}

#endif
