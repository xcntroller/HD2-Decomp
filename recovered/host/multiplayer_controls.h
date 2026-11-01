#ifndef HD2_HOST_MULTIPLAYER_CONTROLS_H
#define HD2_HOST_MULTIPLAYER_CONTROLS_H

#include "host/pre_mission_controls.h"
#include "host/profiles_menu.h"
#include "frontend/multiplayer_server.h"

namespace hd2::host {

// Map rows carry the selected identity without the movie-list selection tint.
class MultiplayerMapRow final : public MenuControl {
  public:
    using MenuControl::MenuControl;
    int enter_focus(unsigned target) override {
        const int result=MenuControl::enter_focus(target); hovered_=true; paint(); return result;
    }
    int leave_focus(unsigned target) override {
        const int result=MenuControl::leave_focus(target); hovered_=false; paint(); return result;
    }
    int pointer_focus(unsigned target) override {
        const int result=MenuControl::pointer_focus(target); hovered_=true; paint(); return result;
    }
    int activate(frontend::ControlEventDestination& destination) override {
        const int result=MenuControl::activate(destination); paint(); return result;
    }
  private:
    void paint() { display_row_selection(false); MenuControl::apply(hovered_); }
    bool hovered_=false;
};

// The browser's authored barrows contains two normal/active pairs. It is the
// original kind0A direction button, not a kind10 selector with child arrows.
class MultiplayerSortArrow final : public MenuControl {
  public:
    MultiplayerSortArrow(const MenuScene& scene,HMODULE module,
                         const frontend::MenuControlDefinition& definition,EngineFrame* frame,
                         MenuTextObject& text,MenuWidgetContext& context)
        : MenuControl(scene,module,false,definition,frame,text,true,&context),scene_(scene),module_(module),
          frame_(frame),id_(definition.identifier) {}
    bool initialize_visuals() override {
        if (!MenuControl::initialize_visuals()) return false;
        const char* names[]{"normalup*","activedup*","normaldown*","activeddown*"};
        for (unsigned i=0;i<4;++i) {
            arrows_[i]=scene_.control_frame(names[i],module_,false,frame_);
            std::memcpy(reinterpret_cast<char*>(arrows_[i])+0xec,&id_,4);
        }
        //00654C60 copies normaldown's local position to all other planes.
        std::array<float,3> position{};
        std::memcpy(position.data(),reinterpret_cast<const char*>(arrows_[2])+0xb0,12);
        for (auto* arrow:arrows_) scene_.set_frame_local_position(arrow,position);
        paint(); return true;
    }
    bool receive_event(frontend::ControlEvent& event,frontend::ControlEventDestination& destination) override {
        const bool result=MenuControl::receive_event(event,destination); paint(); return result;
    }
    int enter_focus(unsigned target) override { hovered_=true; const int result=MenuControl::enter_focus(target); paint(); return result; }
    int leave_focus(unsigned target) override { hovered_=false; const int result=MenuControl::leave_focus(target); paint(); return result; }
    int apply(bool hover) override { hovered_=hover; const int result=MenuControl::apply(hover); paint(); return result; }
    int activate(frontend::ControlEventDestination& destination) override {
        const int result=MenuControl::activate(destination); paint(); return result;
    }
    int click(frontend::ControlEventDestination& destination) override {
        const int result=MenuControl::click(destination); paint(); return result;
    }
    void update(unsigned ms) override { MenuControl::update(ms); paint(); }
    int pointer_target(frontend::ScreenPointerCallback callback,unsigned target,
                       frontend::ControlEventDestination& destination) override {
        const int result=MenuControl::pointer_target(callback,target,destination); paint(); return result;
    }
    void direction(bool descending) { descending_=descending; paint(); }
  private:
    void paint() {
        if (!arrows_[0]) return;
        const unsigned selected=(descending_?2u:0u)+(hovered_ && !disabled()?1u:0u);
        for (unsigned i=0;i<4;++i) scene_.set_screen_visible(module_,arrows_[i],visible() && i==selected);
    }
    const MenuScene& scene_;
    HMODULE module_;
    EngineFrame* frame_;
    unsigned id_;
    std::array<EngineFrame*,4> arrows_{};
    bool descending_=false,hovered_=false;
};

//006610D0's text* is a text surface with seven checkbox-art children.
//Face the surface away while retaining the artwork's authored world transform.
class MultiplayerLabeledToggle final : public MenuControl {
  public:
    MultiplayerLabeledToggle(const MenuScene& scene,HMODULE module,
        const frontend::MenuControlDefinition& definition,EngineFrame* plane,EngineFrame* outer,
        MenuTextObject& text,MenuWidgetContext& context,CinematicsCloneBackend& clone)
        : MenuControl(scene,module,false,definition,plane,text,true,&context),module_(module),
          plane_(plane),outer_(outer),text_(text),clone_(clone) {}
    bool initialize_visuals() override {
        using Count=unsigned(WINAPI*)(EngineFrame*);
        using Child=EngineFrame*(WINAPI*)(EngineFrame*,unsigned);
        const auto count=scene_method<Count>(plane_,module_,0x3c,0x1db80)(plane_);
        if (count!=7) throw std::runtime_error("Labeled toggle needs seven authored checkbox planes");
        const auto child=scene_method<Child>(plane_,module_,0x40,0x1dba0);
        std::array<EngineFrame*,7> children{};
        for (unsigned i=0;i<7;++i) children[i]=child(plane_,i);
        for (auto* frame:children) clone_.link(frame,outer_,1);
        //00629EE0 sends02000073(0) to every labeled rule checkbox.
        text_.initialize_static_frame(); text_.object().inset=0;
        for (auto* frame:children) clone_.link(frame,plane_,1);
        return MenuControl::initialize_visuals();
    }
    int enter_focus(unsigned target) override {
        const int result=MenuControl::enter_focus(target);
        text_.highlighted(!disabled());
        return result;
    }
    int leave_focus(unsigned target) override {
        text_.highlighted(false);
        return MenuControl::leave_focus(target);
    }
  private:
    HMODULE module_;
    EngineFrame *plane_,*outer_;
    MenuTextObject& text_;
    CinematicsCloneBackend& clone_;
};

//00665A50: server row icons belong to the corresponding normal/active
// planes. Text uses the five original30..34 style records on one row plane.
class MultiplayerServerRow final : public MenuControl {
  public:
    MultiplayerServerRow(const MenuScene& scene,HMODULE module,
                         const frontend::MenuControlDefinition& definition,EngineFrame* frame,
                         MenuTextObject& text,MenuWidgetContext& context,CinematicsCloneBackend& clone)
        : MenuControl(scene,module,false,definition,frame,text,true,&context),scene_(scene),module_(module),
          definition_(definition),frame_(frame),text_(text),clone_(clone) {}
    bool initialize_visuals() override {
        if (!MenuControl::initialize_visuals()) return false;
        const char* names[]{"xpassword","xdedicated","xvoice","xpassword01","xdedicated01","xvoice01"};
        auto* normal=scene_.control_frame("normal*",module_,false,frame_);
        auto* active=scene_.control_frame("actived*",module_,false,frame_);
        for (unsigned i=0;i<6;++i) {
            icons_[i]=scene_.control_frame(names[i],module_,false,frame_);
            scene_.set_screen_visible(module_,icons_[i],false);
            std::memcpy(reinterpret_cast<char*>(icons_[i])+0xec,&definition_.identifier,4);
            clone_.link(icons_[i],i<3?normal:active,1);
        }
        text_.object().inset=0;
        return true;
    }
    void value(const frontend::MultiplayerServer& server) {
        auto& object=text_.object(); object.clear();
        const auto raw=[&](std::string text,unsigned style) {
            object.append(std::make_shared<const std::string>(std::move(text)),style);
        };
        raw(server.manual && server.style==4 && server.ping>9998?" ??? ":
            std::string(frontend::multiplayer_text(server.host)),30);
        raw(std::string(frontend::multiplayer_text(server.map)),31);
        if (server.style<=3) object.append(text_.localize(server.style==3?20001:2891+server.style),32);
        else object.append(text_.localize(2894),32);
        raw(std::to_string(server.ping),33);
        raw(std::to_string(server.players)+" / "+std::to_string(server.capacity),34);
        object.disabled=server.ping>9998; //Unanswered records retain the original subdued text.
        const bool flags[]{server.password!=0,server.dedicated!=0,server.flag88!=0};
        for (unsigned i=0;i<6;++i) scene_.set_screen_visible(module_,icons_[i],flags[i%3]);
        text_.invalidate_bounds();
    }
  private:
    const MenuScene& scene_;
    HMODULE module_;
    const frontend::MenuControlDefinition& definition_;
    EngineFrame* frame_;
    MenuTextObject& text_;
    CinematicsCloneBackend& clone_;
    std::array<EngineFrame*,6> icons_{};
};

//006663F0: exactly11 retained row products, backed by the owned result list.
// Scrolling changes the records shown by those products rather than creating
// hundreds of overlapping authored planes or invalidating callback pointers.
class MultiplayerServerList final : public OptionsGroup {
  public:
    using RowLookup=std::function<MultiplayerServerRow*(unsigned)>;
    using Selected=std::function<void(const frontend::MultiplayerServer&)>;
    MultiplayerServerList(const MenuScene& scene,HMODULE module,
                          const frontend::MenuControlDefinition& definition,EngineFrame* frame,
                          MenuTextObject& text,MenuWidgetContext& context,
                          std::function<MenuTextObject*(unsigned)> lookup,Extra extra,
                          RowLookup rows,Selected selected)
        : OptionsGroup(scene,module,definition,frame,text,context,std::move(lookup),std::move(extra)),
          scene_(scene),module_(module),rows_(std::move(rows)),selected_(std::move(selected)) {}
    bool initialize_visuals() override {
        if (!OptionsGroup::initialize_visuals()) return false;
        if (tree_->child_count()!=11) throw std::logic_error("Server browser requires11 authored rows");
        std::memcpy(anchor_.data(),reinterpret_cast<const char*>(tree_->child_at(0).frame())+0xb0,12);
        return true;
    }
    void initialize_events(const std::vector<frontend::ControlEvent>& events) override {
        for (auto event:events) receive_event(event,context_.queue);
        layout();
    }
    bool receive_event(frontend::ControlEvent& event,frontend::ControlEventDestination& destination) override {
        if (event.target & 0xff000u) return tree_->receive_event(event,destination);
        if (!(event.code & 0x040ff000u)) switch (event.code & 0x03000fffu) {
        case 0x0100000b: slider_=event.argument; return true;
        case 0x02000018: records({}); return true;
        case 0x0200002b: {
            const auto next=std::min<std::size_t>(event.argument,maximum());
            if (next!=offset_) { offset_=next; layout(); }
            return true;
        }
        case 0x0200002c: layout(); return true;
        default: break;
        }
        return tree_->receive_event(event,destination);
    }
    void records(std::vector<frontend::MultiplayerServer> values) {
        std::optional<frontend::MultiplayerServer> previous;
        if (selected_index_ && *selected_index_<values_.size()) previous=values_[*selected_index_];
        values_=std::move(values);
        offset_=std::min(offset_,maximum());
        selected_index_.reset();
        if (previous) for (std::size_t i=0;i<values_.size();++i)
            if (values_[i].ipv4==previous->ipv4 && values_[i].port==previous->port) { selected_index_=i; break; }
        layout();
    }
    void choose(unsigned identifier) {
        const auto slot=(identifier>>12)&255u;
        if (!slot || slot>11 || offset_+slot-1>=values_.size()) return;
        selected_index_=offset_+slot-1;
        for (const auto& row:definition_.children)
            if (row.identifier!=identifier) rows_(row.identifier)->deselect_row();
        selected_(values_[*selected_index_]);
    }
    bool key(unsigned word) override {
        if (tree_->key(word)) return true;
        if (values_.empty()) return false;
        auto index=static_cast<std::int64_t>(selected_index_.value_or(offset_));
        switch (word&255u) {
        case 199: index=0; break;
        case 207: index=values_.size()-1; break;
        case 201: index-=10; break;
        case 209: index+=10; break;
        case 200: --index; break;
        case 208: ++index; break;
        default: return false;
        }
        index=std::clamp<std::int64_t>(index,0,values_.size()-1);
        if (static_cast<std::size_t>(index)<offset_) offset_=index;
        else if (static_cast<std::size_t>(index)>=offset_+11) offset_=index-10;
        layout();
        const auto target=definition_.children.at(index-offset_).identifier;
        tree_->leave_focus(target); tree_->enter_focus(target); tree_->activate(target);
        return true;
    }
    int wheel_target(std::int32_t delta,frontend::ControlEventDestination&) override {
        const auto steps=frontend::slider_wheel_steps(delta);
        for (unsigned i=0;i<steps.count;++i)
            context_.queue.dispatch_immediate({slider_,steps.code,0,0,0,{}});
        return 1;
    }
  private:
    std::size_t maximum() const { return values_.size()>11?values_.size()-11:0; }
    void layout() {
        if (!tree_) return;
        for (unsigned i=0;i<11;++i) {
            auto& child=tree_->child_at(i);
            auto position=anchor_;
            position[1]-=i*frontend::widget_float_word(0x3d072b02);
            scene_.set_frame_local_position(child.frame(),position);
            scene_.update_screen_frame(module_,child.frame());
            const bool visible=offset_+i<values_.size();
            frontend::ControlEvent event{0,visible?0x01000003u:0x01000004u,0,0,0,{}};
            child.receive_event(event,context_.queue);
            auto* row=rows_(definition_.children[i].identifier);
            if (visible) row->value(values_[offset_+i]);
            row->display_row_selection(visible && selected_index_ && *selected_index_==offset_+i);
        }
        if (slider_) {
            context_.queue.enqueue({slider_,0x01000020,static_cast<unsigned>(maximum()+1),0,0,{}});
            context_.queue.enqueue({slider_,0x0200002b,static_cast<unsigned>(offset_),0,0,{}});
        }
    }
    const MenuScene& scene_;
    HMODULE module_;
    RowLookup rows_;
    Selected selected_;
    std::vector<frontend::MultiplayerServer> values_;
    std::optional<std::size_t> selected_index_;
    std::size_t offset_=0;
    unsigned slider_=0;
    std::array<float,3> anchor_{};
};

//0064ED00/0064EFE0: an editable value on the authored button plane and
// an independent nonfocusable text* caption. The parent's caption messages
// address the caption, while numeric/range/character messages address the edit.
// Native products own their internals; no raw original vtable is executed.
class MultiplayerNumeric final : public MenuControl {
  public:
    MultiplayerNumeric(const MenuScene& scene, HMODULE module,
                       const frontend::MenuControlDefinition& definition, EngineFrame* frame,
                       MenuTextObject& text, MenuWidgetContext& context,
                       std::function<MenuTextObject*(unsigned)> lookup)
        : MenuControl(scene,module,false,definition,frame,text,true,&context,false),
          scene_(scene), module_(module), definition_(definition), frame_(frame), context_(context),
          lookup_(std::move(lookup)) {}
    bool initialize_visuals() override {
        if (definition_.children.size() != 2) throw std::logic_error("Numeric field lacks internal definitions");
        const auto& value = definition_.children[0];
        const auto& caption = definition_.children[1];
        auto* label = scene_.control_frame("text*", module_, false, frame_);
        edit_ = std::make_unique<ProfileEdit>(scene_,module_,value,frame_,*lookup_(value.identifier),context_);
        caption_ = std::make_unique<MenuControl>(scene_,module_,false,caption,label,*lookup_(caption.identifier),true,&context_);
        if (!edit_->initialize_visuals() || !caption_->initialize_visuals()) return false;
        edit_->additional_hover([this] {
            const auto& cursor=context_.input.pointer.cursor();
            return lookup_(definition_.children[1].identifier)->rectangle(false).contains({cursor.x(),cursor.y()});
        });
        lookup_(value.identifier)->visibility_query([this] { return visible(); });
        lookup_(caption.identifier)->visibility_query([this] { return visible(); });
        return true;
    }
    void initialize_events(const std::vector<frontend::ControlEvent>& events) override {
        edit_->initialize_events(definition_.children[0].initialization);
        caption_->initialize_events(definition_.children[1].initialization);
        for (auto event : events) receive_event(event,context_.queue);
    }
    bool receive_event(frontend::ControlEvent& event,frontend::ControlEventDestination& upstream) override {
        if (!(event.code & 0x040ff000u)) {
            const auto code = event.code & 0x03000fffu;
            switch (code) {
            case 0x01000001: case 0x01000002:
                //The composite's value and caption both follow its eligibility.
                edit_->receive_event(event,upstream);
                caption_->receive_event(event,upstream);
                break;
            case 0x01000014: case 0x01000015:
                return caption_->receive_event(event,upstream);
            case 0x01000007: case 0x0100006e: case 0x0100006f:
            case 0x01000078: case 0x01000079: case 0x0100009b:
            case 0x010000a0: case 0x0200002f:
                return edit_->receive_event(event,upstream);
            case 0x0200002c:
                caption_->receive_event(event,upstream);
                return edit_->receive_event(event,upstream);
            default: break;
            }
        }
        frontend::EmittingControlDestination<MultiplayerNumeric> destination{*this,&upstream};
        frontend::ControlStateRuntime<MultiplayerNumeric> state{*this,properties_,bindings_,destination};
        return state.dispatch(event);
    }
    bool dispatch_control_event(frontend::ControlEvent& event,frontend::ControlEventDestination& destination) {
        return receive_event(event,destination);
    }
    bool eligible() const override { return properties_.eligible() && edit_->eligible(); }
    bool visible() const override { return properties_.visible != 0; }
    bool disabled() const override { return properties_.disabled != 0; }
    bool key(unsigned word) override { return edit_->key(word); }
    void update(unsigned ms) override { edit_->update(ms); }
    bool preserves_part_targets() const override { return true; }
    void bind_pick_target(unsigned target) override { edit_->bind_pick_target(target); }
    int enter_focus(unsigned target) override { pointer_target_=target; return edit_->enter_focus(target); }
    int leave_focus(unsigned target) override { return edit_->leave_focus(target); }
    int pointer_focus(unsigned target) override { pointer_target_=target; return edit_->pointer_focus(target); }
    int pointer_target(frontend::ScreenPointerCallback callback,unsigned target,
                       frontend::ControlEventDestination& destination) override {
        pointer_target_=target;
        if (target==definition_.children[1].identifier) return 7;
        return edit_->pointer_target(callback,target,destination);
    }
    int activate(frontend::ControlEventDestination& destination) override { return edit_->activate(destination); }
    int deactivate(frontend::ControlEventDestination& destination) override { return edit_->deactivate(destination); }
    int click(frontend::ControlEventDestination& destination) override {
        //0065B110 only toggles an editor for its own pointer target.
        //Caption hover belongs to the composite; caption click is inert.
        if (pointer_target_==definition_.children[1].identifier) return 7;
        return edit_->click(destination);
    }
    bool has_text() const { return false; }
    void set_text_disabled(std::uint8_t) {}
    void set_frame_visible(std::uint8_t value) { scene_.set_screen_visible(module_,frame_,value!=0); }
    void update_frame() {
        scene_.update_screen_frame(module_,frame_);
        lookup_(definition_.children[0].identifier)->invalidate_bounds();
        lookup_(definition_.children[1].identifier)->invalidate_bounds();
    }
    std::size_t child_count() const { return 2; }
    void dispatch_child(std::size_t index,frontend::ControlEvent& event) {
        (index ? caption_.get() : edit_.get())->receive_event(event,context_.queue);
    }
    std::string value() const { return edit_->value(); }
    void value(frontend::SharedText text) {
        lookup_(definition_.children[0].identifier)->object().set(std::move(text),13,0);
    }

  private:
    const MenuScene& scene_;
    HMODULE module_;
    const frontend::MenuControlDefinition& definition_;
    EngineFrame* frame_;
    MenuWidgetContext& context_;
    std::function<MenuTextObject*(unsigned)> lookup_;
    std::unique_ptr<ProfileEdit> edit_;
    std::unique_ptr<MenuControl> caption_;
    unsigned pointer_target_=0;
    frontend::ControlProperties properties_;
    frontend::ControlEventBindings bindings_;
};

inline frontend::MenuControlDefinition multiplayer_numeric_definition(
        unsigned id,const char* frame,unsigned label,int minimum,int maximum,unsigned capacity=30) {
    using namespace frontend;
    MenuControlDefinition result{id,frame,MenuControlKind::numeric,nullptr,nullptr,MenuControlRole::none,
        {caption_definition(13,label),{0,0x010000a0,static_cast<unsigned>(minimum),static_cast<unsigned>(maximum),0,{}},
         {0,0x0100006f,1,0,0,{}},{0,0x0100006e,capacity,0,0,{}}}};
    result.children.push_back({id|1u,frame,MenuControlKind::edit,"normal*","actived*",MenuControlRole::none,
        {caption_definition(13,0x7d2)}});
    result.children.push_back(text_definition(id|2u,"text*",{{0,0x01000006,0,0,0,{}}}));
    return result;
}

} // namespace hd2::host
#endif
