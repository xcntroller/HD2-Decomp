#ifndef HD2_HOST_MULTIPLAYER_CONNECTION_MENU_H
#define HD2_HOST_MULTIPLAYER_CONNECTION_MENU_H

#include "host/pre_mission_menu.h"
#include "frontend/multiplayer_connection.h"

namespace hd2::host {

//0068A460/0068A990/0068AD20: distinct type8 scene/queue/registry, regular
// console1 and password2. No ordinary Main modal or borrowed row is reused.
class MultiplayerConnectionMenu {
  public:
    MultiplayerConnectionMenu(StartupSession& startup,MenuDriver& driver,HMODULE module,
        Graphics* graphics,HWND window,BorrowedGraphicsInput& keyboard,
        frontend::MultiplayerLaunchRequest request)
        : request_(std::move(request)), menu_(startup,driver,module,graphics,window,keyboard,
            [this](PreMissionMenu& menu) {return content(menu);}) {request_.validate();}
    void initialize(std::function<void(unsigned)> status) {menu_.initialize_connection(std::move(status));}
    void shutdown() {menu_.shutdown(); owner_.reset();}
    void input(const unsigned* first,const unsigned* second) {menu_.input(first,second);}
    void update(unsigned milliseconds) {menu_.update(milliseconds);}
    void render() {menu_.render();}
  private:
    struct Owner {
        using Event=frontend::ControlEvent;
        using K=frontend::MenuControlKind;
        PreMissionMenu& menu;
        frontend::MultiplayerConnectionFlow flow;
        MenuDriver::Owner password_model;
        ProfileEdit* edit=nullptr;
        MenuTextObject* console=nullptr;
        explicit Owner(PreMissionMenu& value,frontend::MultiplayerLaunchRequest request)
            : menu(value),flow(std::move(request)),
              password_model(menu.scene().load_screen_model(menu.module(),"password")) {}
        static frontend::SharedText text(std::string value) {
            return std::make_shared<const std::string>(std::move(value));
        }
        static bool action(void* owner,void*,void* sender,unsigned tag,unsigned) {
            auto& self=*static_cast<Owner*>(owner);
            const auto id=static_cast<ButtonVisualPair*>(sender)->identifier();
            if (id==0x04400000) self.menu.status(4); //Original Exit Game06001004.
            else if (id==0x08300000 || (id==0x08200000 && tag==0x04016000)) self.submit_password();
            else if (id==0x08400000) self.cancel_password(true);
            else if (id==0x08200000 && tag==0x04017000) self.cancel_password(false);
            return true;
        }
        void submit_password() {
            if (!edit || edit->value().empty()) return;
            const auto password=edit->value();
            edit->deactivate(menu.queue());
            if (flow.password(password)) menu.queue().enqueue({0,0x02000003,0,0,0,{}});
        }
        void cancel_password(bool release) {
            //Escape's edit callback must retain capture until its own release
            //consumes the key, otherwise the console receives a second Escape.
            if (release && edit) edit->deactivate(menu.queue());
            menu.queue().enqueue({0,0x02000003,0,0,0,{}});
        }
        frontend::MenuControlDefinition button(unsigned id,const char* frame,unsigned caption) {
            return frontend::button_definition(id,frame,"normal*","actived*",frontend::MenuControlRole::multiplayer,
                {frontend::caption_definition(13,caption),
                 frontend::observer_control_event(0,0x04001000,this,&action)});
        }
        frontend::MenuScreenDefinition console_definition() {
            auto* frame=menu.scene().main_model_frame();
            menu.scene().position_menu_model(menu.module(),frame,.5f,-.375f,-.5f);
            for (const char* name:{"byes*","bno*"})
                menu.scene().set_screen_visible(menu.module(),menu.scene().control_frame(name,menu.module(),false,frame),false);
            auto exit=button(0x04400000,"bok*",0x7d8);
            Event caption{0,0x01000014,13,0,0,{}};
            caption.text=text("Exit Game"); exit.initialization.front()=std::move(caption);
            return {"MP Connect console",0x04000000,"confirm",{
                frontend::text_definition(0x04100000,"text*"),std::move(exit)}};
        }
        frontend::MenuScreenDefinition password_definition() {
            menu.scene().position_menu_model(menu.module(),password_model.get(),.5f,-.375f,-.5f);
            frontend::MenuControlDefinition input{0x08200000,"binput",K::edit,nullptr,nullptr,
                frontend::MenuControlRole::none,{frontend::caption_definition(14,0x7d2),
                frontend::observer_control_event(0,0x04016000,this,&action),
                frontend::observer_control_event(0,0x04017000,this,&action)}};
            for (const char* name:{"binput01","binput02"})
                menu.scene().set_screen_visible(menu.module(),menu.scene().control_frame(name,menu.module(),false,password_model.get()),false);
            return {"PASSWORD",0x08000000,"password",{
                frontend::text_definition(0x08100000,"text*",{frontend::caption_definition(12,0x7d3)}),
                std::move(input),button(0x08300000,"byes*",0x7d1),button(0x08400000,"bno*",2000)}};
        }
        bool receive(Event& event) {
            const auto code=event.code&0x03000fffu;
            if (code==0x02000069) {flow.connected();return true;}
            if (code!=0x0200000a) return false;
            const auto* current=menu.queue().screens().current();
            if (current && current->identifier==0x08000000) cancel_password(true);
            else menu.status(4);
            return true;
        }
        void tick(unsigned ms) {
            const auto* current=menu.queue().screens().current();
            flow.tick(ms,current && current->identifier==0x04000000,*this);
        }
        void clear_console() {console->column_rows.emplace();console->column_line_height=21;}
        void line(std::string value) {
            console->column_rows->push_back({{std::move(value),2,.035f}});
        }
        void label(unsigned id) {line(frontend::lookup_localized_text(menu.startup().text(),id));}
        using Submission=frontend::MultiplayerConnectionFlow::Submission;
        //Only enumeration is currently reconstructed. Do not call the original
        //network/game code, manufacture transport success or emit its errors.
        Submission join(const frontend::MultiplayerLaunchRequest&) {return Submission::unavailable;}
        Submission host(const frontend::MultiplayerLaunchRequest&) {return Submission::unavailable;}
        unsigned result() const {return 0;}
        void unavailable() {line("Session connection is unavailable in this build.");}
        void game_entry(const frontend::MultiplayerLaunchRequest&) {
            line("Multiplayer game loading is unavailable in this build.");
        }
        void password() {
            menu.text(0x08200000)->object().set(text(""),14,0);
            menu.queue().enqueue({0,0x02000001,2,0,0,{}});
            frontend::request_control_focus(menu.queue(),0x08200000);
        }
    };
    PreMissionMenuContent content(PreMissionMenu& menu) {
        owner_=std::make_shared<Owner>(menu,request_);
        const auto owner=owner_;
        PreMissionMenuContent out{owner,owner->console_definition(),{}, {}, {}, {}, {}};
        out.additional.push_back({owner->password_definition(),owner->password_model.get(),false});
        out.control=[owner,&menu](const frontend::MenuControlDefinition& d,EngineFrame* frame,MenuTextObject& text)
            ->std::unique_ptr<MenuControl> {
            if (d.identifier==0x04100000) owner->console=&text;
            if (d.kind!=frontend::MenuControlKind::edit) return {};
            auto edit=std::make_unique<ProfileEdit>(menu.scene(),menu.module(),d,frame,text,menu.widgets());
            owner->edit=edit.get();return edit;
        };
        out.receive=[owner](frontend::ControlEvent& event) {return owner->receive(event);};
        out.tick=[owner](unsigned ms) {owner->tick(ms);};
        return out;
    }
    frontend::MultiplayerLaunchRequest request_;
    PreMissionMenu menu_;
    std::shared_ptr<Owner> owner_;
};

} // namespace hd2::host
#endif
