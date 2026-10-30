#ifndef HD2_FRONTEND_DEFINED_CONTROLS_H
#define HD2_FRONTEND_DEFINED_CONTROLS_H

#include "frontend/defined_screen.h"
#include "frontend/control_factory.h"

#include <cstdio>
#include <memory>
#include <utility>

namespace hd2::frontend {

// One stable control collection builds both original screen definitions.
// Availability roles are host policy, separate from original control state.
template<class Control>
class DefinedControls {
public:
    template<class Create>
    DefinedControls(const MenuScreenDefinition& definition,
                    std::initializer_list<MenuControlRole> available, Create create)
        : definition_(definition), controls_(definition.controls.size()),
          available_(definition.controls.size(), false), hovered_(definition.controls.size(), false) {
        if (definition.controls.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            throw std::length_error("Too many screen controls");
        }
        std::size_t count = 0;
        for (std::size_t i = 0; i < definition.controls.size(); ++i) {
            const auto& entry = definition.controls[i];
            for (const auto role : available) {
                if (role != MenuControlRole::none && role == entry.role) available_[i] = true;
            }
            Factory<Create> factory{*this, create, entry, i, {}};
            construct_defined_control(factory);
            if (controls_[i]) ++count;
        }
        std::printf("Definition controls ready: screen=%s children=%u constructed=%u\n",
                    definition.label, static_cast<unsigned>(controls_.size()), static_cast<unsigned>(count));
    }

    void bind_screen(DefinedScreen<Control>& screen) {
        if (&screen.definition() != &definition_) throw std::logic_error("Screen definition mismatch");
        for (std::size_t i = 0; i < controls_.size(); ++i) {
            if (available_[i] && controls_[i]) screen.bind_control(static_cast<int>(i), *controls_[i]);
        }
    }
    void bind_pick_targets() const {
        for (std::size_t i = 0; i < controls_.size(); ++i) {
            if (available_[i] && controls_[i]) controls_[i]->bind_pick_target(definition_.controls[i].identifier);
        }
    }
    int index_for_role(MenuControlRole role) const {
        for (std::size_t i = 0; i < definition_.controls.size(); ++i) {
            if (role != MenuControlRole::none && definition_.controls[i].role == role) {
                return static_cast<int>(i);
            }
        }
        throw std::logic_error("Missing screen role");
    }
    std::uint32_t target_for_role(MenuControlRole role) const {
        return definition_.controls[static_cast<std::size_t>(index_for_role(role))].identifier;
    }
    Control& control_at(int index) {
        if (index < 0 || static_cast<std::size_t>(index) >= controls_.size() ||
                !available_[static_cast<std::size_t>(index)] || !controls_[static_cast<std::size_t>(index)]) {
            throw std::logic_error("Screen control is unavailable");
        }
        return *controls_[static_cast<std::size_t>(index)];
    }
    // Message delivery follows the original child vector, including controls
    // that are currently unavailable for user input.
    Control& message_control_at(std::size_t index) {
        if (index >= controls_.size() || !controls_[index]) {
            throw std::logic_error("Missing screen message control");
        }
        return *controls_[index];
    }
    Control& control_for_role(MenuControlRole role) {
        return control_at(index_for_role(role));
    }
    void set_hover(MenuControlRole role, bool hover) {
        const auto index = static_cast<std::size_t>(index_for_role(role));
        auto& control = control_for_role(role);
        if (hovered_[index] == hover) return;
        control.apply(hover);
        hovered_[index] = hover;
    }

private:
    template<class Create>
    struct Factory {
        DefinedControls& owner;
        Create& create;
        const MenuControlDefinition& definition;
        std::size_t index;
        std::unique_ptr<Control> candidate;
        bool find_frame(std::uint32_t flags) { return create.find_frame(definition, flags); }
        void report_missing_frame() { create.report_missing_frame(definition); }
        void report_unknown_kind(std::uint32_t kind) { create.report_unknown_kind(kind); }
        std::uint32_t control_kind() const { return static_cast<std::uint32_t>(definition.kind); }
        bool allocate_control(std::uint32_t) {
            candidate = create(definition);
            return candidate != nullptr;
        }
        bool initialize_visuals() {
            return candidate->initialize_visuals();
        }
        void initialize_definition_data() { candidate->initialize_definition_data(); }
        void initialize_definition_events() {
            // Availability governs input binding, not initialization. Hidden OK
            // runs its original hide event through the same shared handler.
            candidate->initialize_events(definition.initialization);
        }
        void append_control() { owner.controls_[index] = std::move(candidate); }
        void destroy_control() { candidate.reset(); }
    };
    const MenuScreenDefinition& definition_;
    std::vector<std::unique_ptr<Control>> controls_;
    std::vector<bool> available_;
    std::vector<bool> hovered_;
};


}

#endif
