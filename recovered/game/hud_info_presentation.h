#ifndef HD2_GAME_HUD_INFO_PRESENTATION_H
#define HD2_GAME_HUD_INFO_PRESENTATION_H
#include "game/hud_help_bindings.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
namespace hd2::game {
//Typed consumer of SAME info TextState/style handles. Peer keys and descriptor
//loans are consumed under the actual network read lock, never retained in HUD.
template<class Panel,class Queries>class HudInfoPresentation {
    using Peer=const void*;
    Panel& panel_;Queries& q_;
    void label(std::uint32_t id,std::uint32_t offset){panel_.append_localized(id,offset);}
    void value(const std::string& text,std::uint32_t offset){panel_.append_string(text,offset);}
    void blank(){label(0x7d2,0x11c);} //00498E80.
    static std::int32_t signed_word(std::uint32_t bits){std::int32_t result;std::memcpy(&result,&bits,4);return result;}
    static std::int32_t ftol(float f){const long double v=f;if(!(v>=-0x1p63L&&v<0x1p63L))return 0;return signed_word(static_cast<std::uint32_t>(static_cast<std::int64_t>(v)));}
    std::string localized(std::uint32_t id){const auto text=q_.localized(id);return text?*text:std::string{};}
    static std::string number(std::int32_t n){return std::to_string(n);}
    static bool name_less(const std::string& first,const std::string& second){ //00496430→007E88F0, original C-locale stricmp.
        const auto fold=[](unsigned char c){return c>='A'&&c<='Z'?static_cast<unsigned char>(c+32):c;};
        for(std::size_t n=0;;++n){const auto a=fold(n<first.size()?static_cast<unsigned char>(first[n]):0),b=fold(n<second.size()?static_cast<unsigned char>(second[n]):0);
            if(a!=b)return a<b;if(!a)return false;}
    }
    void clock(std::uint32_t id,std::int32_t milliseconds,std::uint32_t offset){
        const auto seconds=milliseconds/1000;std::array<char,80> text{};
        const auto count=std::snprintf(text.data(),text.size(),"%02d:%02d:%02d",seconds/3600,(seconds/60)%60,seconds%60);
        if(count<0||static_cast<std::size_t>(count)>=text.size())throw std::runtime_error("HUD info clock formatting failed");
        value(localized(id)+" "+text.data(),offset);
    }
    void objectives(){ //00497210, actual mission related/native34 descriptors; fresh count after each entry.
        for(std::size_t n=0;n<q_.objective_count();++n){auto* objective=q_.objective(n);if(!objective)throw std::logic_error("HUD info objective borrower is null");
            const auto status=q_.objective_status(*objective);if(status==4)continue;
            if(q_.offline())label(q_.objective_caption(*objective),0x20);
            else {const auto team=q_.local_team();if(team==0)label(q_.objective_team_caption(*objective,0),0x20);
                else if(q_.local_team()==1)label(q_.objective_team_caption(*objective,1),0x20);else continue;}
            std::uint32_t caption=0;
            if(q_.offline()){switch(status){case 0:caption=0xb65;break;case 1:caption=0xb63;break;case 2:caption=0xb64;break;case 3:caption=0xb66;break;default:break;}}
            else switch(status){case 0:caption=0xb65;break;case 1:{const auto team=q_.objective_team(*objective);caption=q_.local_team()==team?0xb63:0xb64;break;}
                case 2:{const auto team=q_.objective_team(*objective);caption=q_.local_team()==team?0xb64:0xb63;break;}default:break;}
            if(caption)label(caption,0x24);
        }
    }
    void help(){ //00496F00;006AC1C0 copies unchanged input on an unknown token, then returns false.
        if(panel_.help_ids().empty())return;blank();blank();label(0xa2d,0x20);blank();
        for(std::size_t n=0;n<panel_.help_ids().size();++n){auto current=localized(panel_.help_ids()[n]);std::shared_ptr<const std::string> output;bool failed=false;
            for(;;){const auto begin=current.find('<'),end=current.find('>');if(begin==std::string::npos||end==std::string::npos||end<=begin+1)break;
                auto next=expand_hud_help_binding(current,[&](std::int32_t action,std::uint32_t mode){return q_.binding_name(action,mode);});
                if(!next){output=std::make_shared<const std::string>(current);failed=true;break;}if(*next==current)throw std::runtime_error("HUD help expansion made no progress");
                output=std::make_shared<const std::string>(std::move(*next));current=*output;
            }
            if(!failed)output=std::make_shared<const std::string>(std::move(current));panel_.append_shared(output,0xf4);
        }
    }
    void mission(){if(!q_.offline())if(auto text=q_.mission_title())value(*text,0x124);} //00499770.
    void footer(){const auto caption=localized(0x930);value(caption+" "+q_.network_address(),0xf8);} //004996B0.
    void limits(){ //004998C0; remaining time is NOT0069AE80's clamped value.
        const auto minutes=q_.time_limit(),frags=q_.frag_limit();
        if(minutes>0)clock(0x1a58,signed_word(static_cast<std::uint32_t>(minutes)*60000u-q_.elapsed_bits()),0x120);
        if(frags>0)value(localized(0x1a59)+" "+number(frags),minutes>0?0x13c:0x138);
    }
    bool inactive(Peer peer,bool result_gate=false){
        if(result_gate&&q_.result_status()==3)return false;
        const auto unavailable=q_.peer_unavailable(peer);auto* human=q_.peer_human(peer); //Both queries occur even if unavailable.
        return unavailable|(human&&q_.health(*human)<1);
    }
    std::int32_t rank(Peer peer){return std::clamp(q_.peer_stat(peer,0x60),0,7);}
    std::vector<Peer> collect(){
        std::vector<Peer> result;for(std::size_t n=0;n<q_.peer_count();++n){const auto peer=q_.peer_identity(n);
            if(!peer)throw std::logic_error("HUD info peer slot has no actual identity");if(!q_.peer_name(peer).empty())result.push_back(peer);}
        return result;
    }
    Peer local(){return q_.local_peer_identity();}
    void number_column(Peer peer,std::uint32_t stat,std::uint32_t offset){value(number(q_.peer_stat(peer,stat)),offset);}
    template<class Body>void peers(Body&& body){
        q_.begin_peers();try{body();}catch(...){q_.end_peers();throw;}q_.end_peers();
    }
    void list(bool mission_mode){ //004977E0/00497380.
        label(0xa33,0x28);label(0xa34,0x38);label(0xa35,0x30);label(0xa36,0x34);if(!mission_mode)label(0xa38,0x2c);
        peers([&]{const auto own=local();auto entries=collect();
            if(mission_mode)std::sort(entries.begin(),entries.end(),[&](Peer a,Peer b){return name_less(q_.peer_name(a),q_.peer_name(b));});
            else std::sort(entries.begin(),entries.end(),[&](Peer a,Peer b){return q_.peer_stat(a,0x4c)>q_.peer_stat(b,0x4c);});
            for(auto peer:entries){const auto dead=inactive(peer);const auto r=rank(peer);std::string name;
                if(mission_mode){const auto caption=localized(0x2792+r);name=caption+" "+q_.peer_name(peer);}
                else {std::string caption;bool prefix=false;if(auto* human=q_.peer_human(peer))if(auto alignment=q_.weapon_alignment(*human)){
                    if(*alignment==0){caption=localized(0x2792+r);prefix=true;}else if(*alignment==1){caption=localized(0x279c+r);prefix=true;}}
                    name=prefix?caption+" "+q_.peer_name(peer):q_.peer_name(peer);}
                const auto name_style=mission_mode?(peer==own?(dead?0x134u:0x130u):(dead?0x12cu:0x128u)):
                    (peer==own?(dead?0x110u:0x10cu):(dead?0xe0u:0x3cu));value(name,name_style);
                if(!mission_mode)number_column(peer,0x4c,dead?0xe4:0x40);
                number_column(peer,0x44,dead?0xe8:0x44);number_column(peer,0x48,dead?0xec:0x48);number_column(peer,0x50,dead?0xf0:0x4c);
            }
        });
    }
    void team_row(Peer peer,bool selected,bool right){ //00498430/00498730.
        if(!peer){if(!right)blank();return;}
        const auto dead=inactive(peer,true);const auto r=rank(peer);constexpr char glyphs[]="oklmpqrnOKLMPQRN";
        value(std::string(1,glyphs[r+(right?8:0)]),right?0x118:0x114);
        const auto normal=right?0x94u:0x7cu,disabled=right?0xbcu:0xa8u;
        const auto selected_normal=right?0x100u:0xfcu,selected_dead=right?0x108u:0x104u;
        value(q_.peer_name(peer),selected?(dead?selected_dead:selected_normal):(dead?disabled:normal));
        number_column(peer,0x50,right?(dead?0xcc:0xa4):(dead?0xb8:0x8c));
        number_column(peer,0x44,right?(dead?0xc4:0x9c):(dead?0xb0:0x84));
        number_column(peer,0x48,right?(dead?0xc8:0xa0):(dead?0xb4:0x88));
        number_column(peer,0x4c,right?(dead?0xc0:0x98):(dead?0xac:0x80));
    }
    void teams(){ //00497E60, axis then allied headings; both lists descending score4C.
        const auto summary=[&](unsigned team){const bool counts=panel_.kind()==3;const auto caption=localized(counts?0xa41:0xa3e);
            const auto amount=counts?q_.team_count(team):ftol(q_.team_score(team));return number(amount)+" "+caption;};
        auto text=summary(1);label(0xa3c,0x78);value(text,0xd0);text=summary(0);label(0xa3d,0x90);value(text,0xd4);blank();
        for(const auto base:{0x50u,0x64u}){label(0xa33,base);label(0xa34,base+0x10);label(0xa35,base+8);label(0xa36,base+0xc);label(0xa38,base+4);}
        peers([&]{const auto own=local();std::array<std::vector<Peer>,2> groups;
            for(std::size_t n=0;n<q_.peer_count();++n){const auto peer=q_.peer_identity(n);if(!peer)throw std::logic_error("HUD info peer slot is null");if(q_.peer_name(peer).empty())continue;
                const auto team=q_.peer_team(peer);if(team==0||team==1)groups[team].push_back(peer);}
            (void)localized(0xa40);label(0xa3f,0xd8);label(0xa3f,0xdc);
            //Native group1 is drawn left; group0 right.
            for(auto& group:groups)std::sort(group.begin(),group.end(),[&](Peer a,Peer b){return q_.peer_stat(a,0x4c)>q_.peer_stat(b,0x4c);});
            const auto count=std::max(groups[0].size(),groups[1].size());
            for(std::size_t n=0;n<count;++n){const auto left=n<groups[1].size()?groups[1][n]:nullptr,right=n<groups[0].size()?groups[0][n]:nullptr;
                team_row(left,own==left,false);if(right)team_row(right,own==right,true);}
        });
    }
public:
    HudInfoPresentation(Panel& panel,Queries& queries):panel_(panel),q_(queries){}
    void refresh(){ //00496E70, clear occurs before dispatch, including unknown kind.
        panel_.clear_text();switch(panel_.kind()){
        case 0:blank();label(0xa29,0x1c);blank();objectives();help();break;
        case 1:blank();label(0xa2a,0x1c);mission();blank();list(false);footer();limits();break;
        case 2:blank();label(0xa2b,0x1c);mission();blank();teams();footer();break;
        case 3:blank();label(0xa2c,0x1c);mission();blank();objectives();blank();blank();teams();footer();break;
        case 4:blank();label(0x4e22,0x1c);mission();blank();objectives();blank();blank();list(true);footer();clock(0x3d22,signed_word(q_.elapsed_bits()),0x120);break;
        default:break;}
    }
};
} //namespace hd2::game
#endif
