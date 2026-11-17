#ifndef HD2_GAME_EFFECTS_DECORATIONS_LOADING_H
#define HD2_GAME_EFFECTS_DECORATIONS_LOADING_H

#include "game/serialized_bytes.h"
#include "game/actor_activation_math.h"
#include "frontend/pre_mission_data.h"
#include <memory>
#include <optional>
#include <vector>
#include <limits>
#include <cmath>
#include <algorithm>

namespace hd2::game {

//Actual GEffects+20 decoration definitions/grid and reached0047BCC0 runtime.
//Models are acquired from the SAME World through required typed services.
//Positive wind/contact producers retain explicit boundaries below.
class EffectsDecorationsLoading {
public:
    using Vector=std::array<float,4>;
    class RuntimeModel {public:
        virtual ~RuntimeModel()=default;virtual void visible(bool)=0;virtual void update()=0;
        virtual void opacity(float)=0;virtual void base_opacity(float)=0;
        virtual void position(const Vector&)=0;virtual void direction(const Vector&,float)=0;virtual void scale(float)=0;
        virtual Vector world_position()=0;virtual Vector world_forward()=0;virtual Vector request_direction()=0;
        virtual float base_opacity()const=0;virtual void visual_amount(float)=0;virtual void request_opacity(float)=0;
    };
    struct Ground {Vector position{},normal{};std::uint32_t material=0;};
    class Runtime {public:
        virtual ~Runtime()=default;
        virtual void seed(std::uint32_t)=0;virtual long double random(float)=0;
        virtual Vector centre()=0;virtual std::optional<Ground> ground(const Vector&)=0;
        virtual std::unique_ptr<RuntimeModel> create_model()=0;
        virtual std::int32_t load_model(RuntimeModel&,const std::string&)=0;
        virtual void clone_model(RuntimeModel&,RuntimeModel&)=0;virtual void link_model(RuntimeModel&)=0;
        virtual void prepare_material(RuntimeModel&)=0;
        //Remaining positive contact/deformation producers stay required. This
        //receiver may return only at independently verified native gates.
        virtual void update_contacts(EffectsDecorationsLoading&,std::uint32_t)=0;
    };
    struct Definition {std::string name;std::vector<std::unique_ptr<RuntimeModel>> models;~Definition(){while(!models.empty())models.pop_back();}};
    struct Entry {
        float weight=1,field4=0,field14=.2f,field18=1;
        std::string name;Definition* definition=nullptr;
        bool flag10=false;std::uint8_t enabled11=1,enabled12=1,flag13=0; //Serialized bytes retain all bits.
        std::uint32_t field40=0;std::unique_ptr<RuntimeModel> model;
        //Native24/30 resource arrays have no source producer in this cold path.
    };
    struct Group {
        std::vector<std::unique_ptr<Entry>> entries;float field10=.949999988079071f,field14=.5f;
        ~Group(){while(!entries.empty())entries.pop_back();}
    };
    struct Request;
    struct Cell {std::uint32_t field0=0;std::unique_ptr<RuntimeModel> model;std::int32_t identity=-1;Request* request=nullptr;bool flag10=false;}; //0047E1C0.
    struct Request { //0047FDCC prefix; two opacity fields remain unwritten on bounce-only requests.
        RuntimeModel* model=nullptr;float target=0,current=0,speed=0;
        std::optional<std::array<float,2>> opacity;bool bounce=false;std::uint32_t field1c=8000;Cell* cell=nullptr;
        ~Request(){if(cell){cell->request=nullptr;if(bounce)cell->flag10=false;}} //0047F150/0047EA60, borrower stores BEFORE free.
    };
    struct Mask {
        std::uint32_t width=0,height=0;float step=0;std::array<float,3> origin{};float field80=0;
        std::vector<unsigned char> bytes;
    };
    const auto& groups()const{return groups_;}const auto& definitions()const{return definitions_;}
    const auto& cells()const{return cells_;}const auto& mask()const{return mask_;}
    const auto& active_entries()const{return active_entries_;}
    const auto& requests()const{return requests_;}
    float material_sight_factor(std::uint32_t material)const{ //004867E0→0047B4C0, absent frozen inventory.
        if(material>=groups_.size()||!groups_[material])return 1.f;
        return groups_[material]->field14;
    }
    std::int32_t width()const{return width_;}std::int32_t height()const{return height_;}
    float density()const{return density_;}bool enabled()const{return enabled_;}bool quality()const{return quality_;}
    bool initial_first_person()const{return first_person_;}
    std::array<float,2> grid_centre()const{return {{static_cast<float>(static_cast<long double>(cell_x_+offset_x_)*density_),static_cast<float>(static_cast<long double>(cell_z_+offset_z_)*density_)}};}
    void read(SerializedBytes bytes){ //0047C530 and0047CC90; existing groups only replaced at supplied indices.
        if(enabled_||!cells_.empty())throw std::logic_error("Retire decorations before another loading read");
        auto root=next_mission_chunk(bytes);if(!root||root->id<=1)return;
        width_=signed_word(root->bytes.word());height_=width_;density_=root->bytes.real();
        while(root->bytes.remaining()){
            const auto index=signed_word(root->bytes.word());
            if(index<0||index>0x100000)throw std::runtime_error("Decoration group index exceeds native safe bounds");
            if(groups_.size()<=std::size_t(index))groups_.resize(std::size_t(index)+1);
            auto group=std::make_unique<Group>();
            //Original replaces/deletes the prior group BEFORE reading the new one.
            groups_[std::size_t(index)]=std::move(group);
            auto record=next_mission_chunk(root->bytes);if(!record)break; //0047CC90 returnsfalse if next chunk absent.
            auto& current=*groups_[std::size_t(index)];auto& data=record->bytes;
            const auto count=signed_word(data.word());
            if(root->id>2)current.field10=data.real();
            if(root->id>8)current.field14=data.real();
            for(std::int32_t n=0;n<count;++n){
                auto name=next_mission_chunk(data);if(!name)throw std::runtime_error("Decoration entry has no name chunk");
                auto entry=std::make_unique<Entry>();entry->name=name->bytes.section(std::min<std::size_t>(999,name->bytes.remaining())).text();
                current.entries.push_back(std::move(entry));auto& value=*current.entries.back();
                value.weight=data.real();value.enabled11=data.byte();
                if(root->id>3)value.field4=data.real();
                if(root->id>4)value.enabled12=data.byte();
                if(root->id>5){value.field40=data.word();value.field14=data.real();}
                if(root->id>6)value.field18=data.real();
                if(root->id>7)value.flag13=data.byte();
            }
        }
    }
    void read_mask(SerializedBytes bytes){ //0047C930, first chunk ID ignored.
        release_mask();auto root=next_mission_chunk(bytes);if(!root)throw std::runtime_error("Decoration mask has no header");
        auto& data=root->bytes;mask_.width=data.word();mask_.height=data.word();mask_.step=data.real();
        for(auto& axis:mask_.origin)axis=data.real();
        mask_.field80=data.real();
        const auto count=std::uint64_t(mask_.width)*mask_.height;
        if(count>128u*1024u*1024u||count>data.remaining())throw std::runtime_error("Invalid decoration mask dimensions or short byte payload");
        const auto* first=data.take(std::size_t(count));mask_.bytes.assign(first,first+std::size_t(count));
    }
    template<class Files>bool load(Files& files,const std::string& resource){
        const auto path="missions\\"+resource+"\\decorations.bin";
        auto bytes=frontend::pre_mission_detail::read_file(files,path.c_str());if(!bytes)return false;
        read(SerializedBytes(*bytes));release_mask();
        const auto mask_path="missions\\"+resource+"\\decorator_sl.bin";
        auto mask=frontend::pre_mission_detail::read_file(files,mask_path.c_str());if(mask)read_mask(SerializedBytes(*mask));
        return true;
    }
    void initialize(bool quality,bool first_person){ //0047B0B0,0047E0F0, actual profile4 and game58 OR59.
        if(enabled_||!cells_.empty())throw std::logic_error("Retire the prior decoration grid before initialization");
        for(auto group=groups_.size();group;)if(auto* current=groups_[--group].get())
            for(auto n=current->entries.size();n;){
                auto& entry=*current->entries[--n];auto name=entry.name;
                if(name.size()>=256)throw std::runtime_error("Decoration resource name exceeds native setup buffer");
                for(auto& c:name)if(c>='A'&&c<='Z')c=char(c-'A'+'a');
                Definition* definition=nullptr;
                for(auto i=definitions_.size();i;)if(definitions_[--i]->name==name){definition=definitions_[i].get();break;}
                if(!definition){auto next=std::make_unique<Definition>();next->name=std::move(name);definition=next.get();definitions_.push_back(std::move(next));}
                entry.definition=definition;if(entry.field40)active_entries_.push_back(&entry);
            }
        if(width_>0){
            const auto count=std::uint64_t(std::uint32_t(width_))*std::uint32_t(height_);
            if(height_<=0||count>0x400000)throw std::runtime_error("Decoration cell grid exceeds native safe bounds");
            cells_.resize(std::size_t(count));cell_x_=cell_z_=100000;field40_=false;field44_=field48_=0;field3c_=.05f;first_person_=first_person;
        }
        enabled_=true;quality_=quality;
        if(!quality_)set_enabled(false); //0047B010, cold cells contain no engine model.
    }
    void set_enabled(bool enabled){
        if(enabled==enabled_||(!quality_&&enabled))return;
        //Native ascending y/x loop visits currently instantiated cell models.
        if(width_>0&&height_>0)for(std::int32_t y=0;y<height_;++y)for(std::int32_t x=0;x<width_;++x){
            auto& cell=cells_.at(std::size_t(y)*std::size_t(width_)+std::size_t(x));
            if(cell.model){cell.model->visible(enabled);cell.model->update();}
        }
        enabled_=enabled;
    }
    template<class Update>void update_loading(bool quality,Update&& update){ //0047BCC0, profile transition precedes all spatial work.
        if(quality_!=quality){quality_=quality;set_enabled(quality);}
        if(!enabled_||groups_.empty())return; //Native98/9C are GROUPS, not definitions.
        update();
    }
    void update_runtime(std::uint32_t elapsed,Runtime& runtime){
        //0047BCC0 skips genuinely null per-entry dynamic mesh borrowers.
        for(auto n=active_entries_.size();n;)if(active_entries_[--n]->model)
            throw std::runtime_error("Decoration wind requires actual retained mesh/vertex producers");
        const auto point=runtime.centre();elapsed_=elapsed;
        update_grid(point[0],point[2],runtime); //0047EB80; large move returns after complete rebuild.
        for(auto n=requests_.size();n;){--n;if(!advance_request(*requests_[n],elapsed))erase_request(n);} //0047F150, reverse then swap/pop AFTER retirement.
    }
    template<class FirstPerson>void update_player_contact(const Vector& point,const Vector& velocity,std::uint32_t posture,
            FirstPerson&& first_person,std::uint32_t elapsed,Runtime& runtime){ //0047F4E0, actual position/velocity captured BEFORE posture and view queries.
        if(posture!=field48_&&(posture==2||field48_==2)){field40_=true;field44_=1000;}
        field48_=posture;
        if(first_person()!=first_person_&&posture==2)field40_=true;
        first_person_=first_person(); //Native0069A4B0 is queried again for publication.
        const float square=static_cast<float>((static_cast<long double>(velocity[0])*velocity[0]+static_cast<long double>(velocity[2])*velocity[2])+static_cast<long double>(velocity[1])*velocity[1]);
        const bool visual=posture==2||field40_;
        if(!visual&&square<9.99999993922529e-09f)return;
        if(field40_){
            field44_-=elapsed;
            if(signed_word(field44_)<0){field44_=0;field40_=false;}
            float value=static_cast<float>(static_cast<long double>(signed_word(field44_))*.0010000000474974513f);
            if(field48_!=2)value=1.f-value;
            field3c_=value+.05f;
            if(!first_person_)field3c_=1.f;
            if(field3c_>1)field3c_=1;
        }
        float x=0,z=0;
        if(square>9.99999993922529e-09f||!visual){
            const auto inverse=1.L/std::sqrt(static_cast<long double>(square));
            x=static_cast<float>(velocity[0]*inverse);z=static_cast<float>(velocity[2]*inverse);
        }
        const float inner=posture==0?.25f:posture==2?.75f:.3f,outer=posture==0?.4f:posture==2?1.f:.5f;
        bend(point,x,z,inner*inner,outer*outer,3.f,.9f,visual,runtime);
    }
    void bend(const Vector& point,float direction_x,float direction_z,float inner_squared,float outer_squared,
            float radius,float opacity_factor,bool visual,Runtime& runtime){ //0047FAE0, X ascending THEN Z ascending, actual circular allocation indices.
        if(!(density_>0))throw std::logic_error("Decoration contact requires initialized cell spacing");
        const auto count=truncated(static_cast<long double>(radius)/density_);
        const auto centre_x=width_/2-cell_x_+coordinate(point[0],density_),centre_z=height_/2-cell_z_+coordinate(point[2],density_);
        for(auto x=centre_x-count;x<centre_x+count;++x)for(auto z=centre_z-count;z<centre_z+count;++z){
            auto cx=x,cz=z;if(cx<0)cx+=width_;if(cz<0)cz+=height_;if(cx>=width_)cx-=width_;if(cz>=height_)cz-=height_;
            if(cx<0||cz<0||cx>=width_||cz>=height_)continue;
            auto& cell=cells_.at(std::size_t(cz)*std::size_t(width_)+std::size_t(cx));
            if(cell.identity<0||cell.field0>=groups_.size()||!groups_[cell.field0])continue;
            const auto& group=*groups_[cell.field0];if(std::size_t(cell.identity)>=group.entries.size()||!group.entries[std::size_t(cell.identity)]->enabled11)continue;
            const bool bounce=cell.request&&cell.request->bounce;
            if(!cell.model)continue;
            if(visual){
                float amount=static_cast<float>(static_cast<long double>(std::abs(z-centre_z)+std::abs(x-centre_x))/(count*2)+field3c_);
                if(amount>1)amount=1;
                cell.model->visual_amount(amount); //Native first-visual A8 happens EVEN when an already flattened cell skips bending.
            }
            if(cell.flag10&&!bounce)continue;
            const auto position=cell.model->world_position();
            const long double dx=static_cast<long double>(point[0])-position[0],dy=static_cast<long double>(point[1])-position[1],dz=static_cast<long double>(point[2])-position[2];
            const auto distance=(dz*dz+dy*dy)+dx*dx;const float stored_distance=static_cast<float>(distance);
            if(!(distance<outer_squared))continue;
            Request* request=nullptr;
            if(bounce){request=cell.request;request->speed=1;}
            else {
                auto next=std::make_unique<Request>();next->model=cell.model.get();next->cell=&cell;next->speed=1;
                request=next.get();cell.request=request;requests_.push_back(std::move(next)); //Publication before native insertion; RAII retracts a throwing prefix.
            }
            request->target=1.2566370964050293f; //Native3FA0D97C, signed by007885B0.
            const auto first=actor_activation_direction(cell.model->world_forward());
            const auto second=actor_activation_direction(cell.model->world_forward()); //Native repeats world getter before X normalization.
            auto angle=std::atan2(static_cast<long double>(direction_z),static_cast<long double>(direction_x))-
                std::atan2(static_cast<long double>(first[2]),static_cast<long double>(second[0]));
            if(angle< -3.1415927410125732f)angle+=6.2831854820251465f;
            if(angle>3.1415927410125732f)angle-=6.2831854820251465f;
            if(angle>0)request->target=-request->target;
            cell.flag10=true;
            if(inner_squared<=stored_distance){request->bounce=true;request->target*=.1f;
                request->target=static_cast<float>((runtime.random(.3f)+1)*request->target);
            }else{request->bounce=false;const auto opacity=cell.model->base_opacity();request->opacity=std::array<float,2>{{opacity,static_cast<float>(static_cast<long double>(opacity)*opacity_factor)}};}
        }
    }
    float mask_opacity(float x,float z)const { //0047D110, bilinear bytes, native factor1/256.
        if(mask_.bytes.empty())return 0;
        const long double px=static_cast<long double>(x)-mask_.origin[0],pz=static_cast<long double>(z)-mask_.origin[1];
        if(!(mask_.step>0))throw std::runtime_error("Decoration mask has invalid sampling step");
        const long double inverse=1.L/mask_.step;
        const auto ix=truncated(px*inverse),iz=truncated(pz*inverse);
        if(ix<0||iz<0||std::uint64_t(ix)+1>=mask_.width||std::uint64_t(iz)+1>=mask_.height)
            throw std::runtime_error("Decoration mask sample exceeds actual byte allocation");
        const float tx=static_cast<float>((px-static_cast<long double>(ix)*mask_.step)*inverse);
        const float tz=static_cast<float>((pz-static_cast<long double>(iz)*mask_.step)*inverse);
        auto blend=[](float a,float b,float t){return static_cast<float>(static_cast<long double>(b)*t+(1.L-t)*a);};
        const auto row=std::size_t(iz)*mask_.width+std::size_t(ix);
        const float next=blend(mask_.bytes[row+mask_.width],mask_.bytes[row+mask_.width+1],tx);
        const float current=blend(mask_.bytes[row],mask_.bytes[row+1],tx);
        return static_cast<float>(static_cast<long double>(blend(current,next,tz))*.00390625f);
    }
    void clear(){ //0047B090→0047EA60 then0047C020; actual requests retire before their model/cell borrowers.
        while(!requests_.empty())requests_.pop_back(); //0047EA60: reverse requests BEFORE any cell/model release.
        for(auto& cell:cells_)cell.model.reset(); //Native ascending Y/X releases, then cell-array RET destructors.
        cells_.clear();
        for(auto n=active_entries_.size();n;)active_entries_[--n]->model.reset();
        active_entries_.clear();
        while(!groups_.empty())groups_.pop_back();
        while(!definitions_.empty())definitions_.pop_back();
        release_mask();enabled_=false; //0047D0C0 leaves metadata scalars unchanged.
    }
    ~EffectsDecorationsLoading(){clear();}
    EffectsDecorationsLoading()=default;EffectsDecorationsLoading(const EffectsDecorationsLoading&)=delete;
    EffectsDecorationsLoading& operator=(const EffectsDecorationsLoading&)=delete;
private:
    void erase_request(std::size_t index){ //Destructor clears typed cell loan/conditional bounce flag before swapping last pointer.
        requests_.at(index).reset();if(index+1!=requests_.size())requests_[index]=std::move(requests_.back());requests_.pop_back();
    }
    bool advance_request(Request& request,std::uint32_t elapsed){ //0047F200, angle first, local orientation/update, THEN one-shot opacity.
        if(!request.model||!request.cell||request.cell->model.get()!=request.model||request.cell->request!=&request)
            throw std::logic_error("Decoration request requires its SAME live cell/model borrowers");
        const float old=request.current;const bool increasing=old<request.target;
        const auto step=static_cast<long double>(signed_word(elapsed))*request.speed*(request.bounce?.3f:3.f)*.0010000000474974513f;
        const auto next=static_cast<long double>(old)+(increasing?step:-step);request.current=static_cast<float>(next);
        bool alive=increasing?next<request.target:next>=request.target;
        if(!alive){request.current=request.target;
            if(request.bounce){
                if(std::abs(request.target)<=.017453293005625408)request.current=0;
                else{request.target*= -.9f;request.speed*=.9f;alive=true;}
            }
        }
        const auto direction=request.model->request_direction();request.model->direction(direction,request.current);request.model->update();
        if(!request.bounce){
            if(!request.opacity)throw std::logic_error("One-shot decoration request has no produced opacity fields");
            auto& opacity=*request.opacity;
            if(opacity[1]<opacity[0]){
                const auto alpha=static_cast<long double>(opacity[0])-static_cast<float>(signed_word(elapsed))*.0003000000142492354f;
                opacity[0]=static_cast<float>(alpha);if(alpha<opacity[1])opacity[0]=opacity[1];
                request.model->request_opacity(opacity[0]);return true;
            }
        }
        return alive;
    }
    static std::int32_t truncated(long double value){
        if(!std::isfinite(value)||value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())
            throw std::runtime_error("Decoration coordinate exceeds signed native range");
        return static_cast<std::int32_t>(value);
    }
    static std::int32_t coordinate(float value,float step){ //Native truncation then decrement for EVERY negative coordinate, including exact multiples.
        return truncated(static_cast<long double>(value)*(1.L/step))-(value<0?1:0);
    }
    std::int32_t fraction(float value,std::int32_t coordinate,std::int32_t size)const{
        return truncated((static_cast<long double>(value)-static_cast<long double>(coordinate)*density_)*(1.L/density_)*size);
    }
    std::unique_ptr<RuntimeModel> acquire(std::uint32_t material,std::uint32_t& selected,bool& rotate,float& height,bool& align,Runtime& runtime){ //0047B500.
        if(material>=groups_.size()||!groups_[material])return {};
        auto& group=*groups_[material];const float chance=static_cast<float>(runtime.random(100));
        std::size_t n=0;for(;n<group.entries.size();++n)if(chance<group.entries[n]->weight)break;
        if(n==group.entries.size())return {};
        auto& entry=*group.entries[n];height=entry.field4;rotate=entry.enabled12!=0;align=entry.flag13!=0;
        if(!entry.definition)throw std::logic_error("Decoration selection lost initialized definition");
        auto& pool=entry.definition->models;
        selected=static_cast<std::uint32_t>(n);
        if(!pool.empty()){
            if(pool.size()>1){auto model=std::move(pool.back());pool.pop_back();return model;}
            auto model=runtime.create_model();if(!model)throw std::runtime_error("Decoration clone allocation returned null");
            runtime.clone_model(*model,*pool.front());runtime.link_model(*model);return model;
        }
        if(entry.flag10)return {}; //Original failed-load latch, no repeated lookup/successful substitute.
        auto model=runtime.create_model();if(!model)throw std::runtime_error("Decoration model allocation returned null");
        if(runtime.load_model(*model,entry.name)!=0){entry.flag10=true;return {};}
        if(entry.field40&&!entry.model)throw std::runtime_error("Decoration dynamic mesh capture requires actual0047BAA0 owner");
        runtime.link_model(*model);runtime.prepare_material(*model);
        auto retained=runtime.create_model();if(!retained)throw std::runtime_error("Decoration template allocation returned null");
        //0047B7E8 inserts the actual template BEFORE its clone call.
        pool.push_back(std::move(retained));runtime.clone_model(*pool.back(),*model);return model;
    }
    void release_cell(Cell& cell){ //0047E290, hide before returning SAME model to its definition pool.
        if(!cell.model)return;
        cell.model->visible(false);
        auto& group=groups_.at(cell.field0);if(!group||cell.identity<0)throw std::logic_error("Decoration cell lost its material/selection");
        auto* definition=group->entries.at(std::size_t(cell.identity))->definition;
        if(!definition)throw std::logic_error("Decoration cell lost definition pool");
        definition->models.push_back(std::move(cell.model));
    }
    void refresh_cell(std::int32_t row,std::int32_t column,Runtime& runtime){ //0047E2D0, arguments are Y then X.
        const auto x=(cell_x_-width_/2)+offset_x_+column,z=(cell_z_-height_/2)+offset_z_+row;
        const long double argument=(x&&z)?static_cast<long double>(x)*std::sin(static_cast<long double>(z)):
            static_cast<long double>(x-row)*std::sin(static_cast<long double>(z-column));
        const auto seed=static_cast<std::uint32_t>(truncated(std::cos(argument+density_)*10000.L+123456.L));
        runtime.seed(seed);
        const auto pz=static_cast<float>(((runtime.random(1)-.5f)*.75f+z)*density_);
        const auto px=static_cast<float>(((runtime.random(1)-.5f)*.75f+x)*density_);
        auto cx=column+offset_x_,cz=row+offset_z_;if(cx>=width_)cx-=width_;if(cz>=height_)cz-=height_;
        if(cx<0||cz<0||cx>=width_||cz>=height_)throw std::logic_error("Decoration grid coordinate escaped actual allocation");
        auto& cell=cells_.at(std::size_t(cz)*std::size_t(width_)+std::size_t(cx));
        const auto ground=runtime.ground({{px,100,pz,0}}); //Query precedes old request/model retirement.
        if(cell.request){
            bool found=false;for(auto n=requests_.size();n;)if(requests_[--n].get()==cell.request){erase_request(n);found=true;break;}
            if(!found)throw std::logic_error("Decoration cell lost its actual request owner");
        }
        cell.flag10=false;
        if(!ground){release_cell(cell);return;}
        const float slope=static_cast<float>((static_cast<long double>(ground->normal[2])+ground->normal[0])*0.f+ground->normal[1]);
        const auto material=ground->material;
        const float minimum=material<groups_.size()&&groups_[material]?groups_[material]->field10:1000.f;
        if(slope<=minimum){release_cell(cell);return;}
        release_cell(cell);
        bool rotate=true,align=false;float fixed_height=0;std::uint32_t selected=static_cast<std::uint32_t>(cell.identity);
        cell.field0=material;cell.model=acquire(material,selected,rotate,fixed_height,align,runtime);cell.identity=static_cast<std::int32_t>(selected);
        if(!cell.model)return;
        cell.model->visible(true); //0047E240 AFTER actual model acquisition.
        const float opacity=static_cast<float>(static_cast<long double>(mask_opacity(ground->position[0],ground->position[2]))*.5f);
        cell.model->opacity(opacity);cell.model->base_opacity(opacity);
        auto position=ground->position;if(fixed_height!=0)position[1]=fixed_height;cell.model->position(position);
        if(align){const float roll=rotate?static_cast<float>(runtime.random(6.2831855f)):0;cell.model->direction(ground->normal,roll);}
        else if(rotate){Vector direction{{static_cast<float>(runtime.random(2)-1),0,static_cast<float>(runtime.random(2)-1),0}};cell.model->direction(direction,0);}
        cell.model->scale(static_cast<float>((runtime.random(2)-1)*.2f+.8f));cell.model->update();
    }
    void refresh_column(std::int32_t column,Runtime& runtime){for(std::int32_t row=0;row<height_;++row)refresh_cell(row,column,runtime);} //0047EA00.
    void refresh_row(std::int32_t row,Runtime& runtime){for(std::int32_t column=0;column<width_;++column)refresh_cell(row,column,runtime);} //0047EA30.
    void shift_x(bool reverse){ //0047E960.
        offset_x_+=reverse?-1:1;
        if(offset_x_>=width_){offset_x_=0;cell_x_+=width_;}
        if(offset_x_<0){offset_x_=width_-1;cell_x_-=width_;}
    }
    void shift_z(bool reverse){ //0047E9B0.
        offset_z_+=reverse?-1:1;
        if(offset_z_>=height_){offset_z_=0;cell_z_+=height_;}
        if(offset_z_<0){offset_z_=height_-1;cell_z_-=height_;}
    }
    void update_grid(float x,float z,Runtime& runtime){ //0047EB80, native Y/X strip order and rolling indices.
        if(width_<1)return;
        if(!(density_>0))throw std::runtime_error("Decoration grid requires actual positive cell spacing");
        const auto nx=coordinate(x,density_),nz=coordinate(z,density_);
        const auto dx=nx-cell_x_-offset_x_,dz=nz-cell_z_-offset_z_;
        const auto ax=std::abs(dx),az=std::abs(dz);
        if(ax>=width_/2||az>=height_/2){
            sub_x_=fraction(x,nx,height_);sub_z_=fraction(z,nz,width_);cell_x_=nx;cell_z_=nz;
            offset_x_=offset_z_=0;for(std::int32_t col=0;col<width_;++col)refresh_column(col,runtime);return;
        }
        if(cell_x_+offset_x_==nx){
            const auto next=fraction(x,nx,height_);
            if(sub_x_<next)for(auto i=sub_x_+1;i<=next;++i)refresh_cell(i,width_-1,runtime);
            else if(next<sub_x_)for(auto i=next+1;i<=sub_x_;++i)refresh_cell(i,0,runtime);
            sub_x_=next;
        }
        if(cell_z_+offset_z_==nz){
            const auto next=fraction(z,nz,width_);
            if(sub_z_<next)for(auto i=sub_z_+1;i<=next;++i)refresh_cell(height_-1,i,runtime);
            else if(next<sub_z_)for(auto i=next+1;i<=sub_z_;++i)refresh_cell(0,i,runtime);
            sub_z_=next;
        }
        if(cell_x_+offset_x_!=nx){
            if(dx<0)for(std::int32_t i=0;i<=sub_x_;++i)refresh_cell(i,0,runtime);
            else if(dx>0)for(auto i=sub_x_+1;i<height_;++i)refresh_cell(i,width_-1,runtime);
            for(auto remaining=ax;remaining;){--remaining;shift_x(dx<0);
                if(dx<0){if(remaining)refresh_column(0,runtime);if(sub_z_<width_-1)refresh_cell(0,sub_z_+1,runtime);}
                else {if(remaining)refresh_column(width_-1,runtime);else {refresh_cell(height_-1,sub_z_,runtime);if(sub_z_>0)refresh_cell(height_-1,sub_z_-1,runtime);}}
            }
            const auto next=fraction(x,nx,height_);
            if(dx>0)for(std::int32_t i=0;i<=next;++i)refresh_cell(i,width_-1,runtime);
            else if(dx<0)for(auto i=next+1;i<height_;++i)refresh_cell(i,0,runtime);
            sub_x_=next;
        }
        if(cell_z_+offset_z_!=nz){
            if(dz<0)for(std::int32_t i=0;i<=sub_z_;++i)refresh_cell(0,i,runtime);
            else if(dz>0)for(auto i=sub_z_+1;i<width_;++i)refresh_cell(height_-1,i,runtime);
            for(auto remaining=az;remaining;){--remaining;shift_z(dz<0);
                if(dz<0){if(remaining)refresh_row(0,runtime);if(sub_x_<height_-1)refresh_cell(sub_x_+1,0,runtime);}
                else {if(remaining)refresh_row(height_-1,runtime);else {refresh_cell(sub_x_,width_-1,runtime);if(sub_x_>0)refresh_cell(sub_x_-1,width_-1,runtime);}}
            }
            const auto next=fraction(z,nz,width_);
            if(dz>0)for(std::int32_t i=0;i<=next;++i)refresh_cell(height_-1,i,runtime);
            else if(dz<0)for(auto i=next+1;i<width_;++i)refresh_cell(0,i,runtime);
            sub_z_=next;
        }
        runtime.update_contacts(*this,elapsed_);
    }
    void release_mask()noexcept{decltype(mask_.bytes){}.swap(mask_.bytes);} //0047D0C0 frees the actual byte buffer.
    static std::int32_t signed_word(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
    std::vector<std::unique_ptr<Group>> groups_;std::vector<std::unique_ptr<Definition>> definitions_;std::vector<Entry*> active_entries_;
    std::vector<Cell> cells_;std::vector<std::unique_ptr<Request>> requests_;Mask mask_;
    std::int32_t width_=60,height_=60,cell_x_=-1,cell_z_=-1;float density_=.5f,field3c_=.05f;
    std::int32_t offset_x_=0,offset_z_=0,sub_x_=0,sub_z_=0;std::uint32_t elapsed_=0;
    std::uint32_t field44_=0,field48_=0;bool enabled_=false,quality_=false,first_person_=false,field40_=false;
};
} //namespace hd2::game
#endif
