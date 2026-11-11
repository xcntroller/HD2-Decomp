#ifndef HD2_GAME_HUMAN_EQUIPMENT_ATTACHMENTS_H
#define HD2_GAME_HUMAN_EQUIPMENT_ATTACHMENTS_H

#include "game/human_equipment_state.h"
#include "game/human_equipment_action.h"
#include "game/human_equipment_offsets.h"
#include "game/item_model_cache.h"

namespace hd2::game {

//0078DF60's loading cases0/1/2/3/10 and action cases5..8. The SAME catalogue/item cache and
//Human230 state are borrowed. Services must supply the actual DD0 opacity,
//identity lookup,6950 flag receiver and007920B0 visibility refresh; none has
//a successful default. Cases4/9 require their separate action producers.
template<class Frames,class Items,class Services>
class HumanEquipmentAttachments {
public:
    using Frame=typename Frames::Frame;
    using Vector=typename Frames::Vector;
    using Matrix=typename Frames::Matrix;
    struct Release {
        Frames* frames=nullptr;
        void operator()(Frame frame) const noexcept {
            if(frame)try{frames->release_item_frame(frame);}
            catch(...){frames->record_actor_cleanup_failure(std::current_exception());}
        }
    };
    using Owner=std::unique_ptr<std::remove_pointer_t<Frame>,Release>;
    HumanEquipmentAttachments(Frames& frames,Items& items,Services& services,
            const frontend::PreMissionItemCatalogue& catalogue,HumanEquipmentState<Frame>& state)
        :frames_(frames),items_(items),services_(services),catalogue_(catalogue),state_(state) {}
    ~HumanEquipmentAttachments(){clear_parts();}
    HumanEquipmentAttachments(const HumanEquipmentAttachments&)=delete;
    HumanEquipmentAttachments& operator=(const HumanEquipmentAttachments&)=delete;
    void attach(unsigned kind) {
        switch(kind) {
        case 0:attach_head();return;
        case 1:attach_bag();return;
        case 2:attach_back_model();return;
        case 3:attach_hand();return;
        case 5:case 6:case 7:case 8:attach_human_equipment_action(frames_,state_,services_,kind);return;
        case 10:attach_hand_on_back();return;
        default:throw std::runtime_error("Human equipment hands/action attachment is not reconstructed");
        }
    }
    void clear_parts() noexcept { //0078DE30, four instance roots ascending, unlink then direct release.
        if(parts_[3]&&state_.model(4)==parts_[3].get())state_.model(4,Frame{});
        for(auto& part:parts_)if(part) {
            try{frames_.link_scene_object(part.get(),Frame{});}
            catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            part.reset();
        }
    }
    const auto& parts() const{return parts_;}
private:
    const frontend::PreMissionItem& definition(std::size_t offset) const {
        const auto* result=catalogue_.item(state_.words().template get<std::int32_t>(offset));
        if(!result)throw std::runtime_error("Human equipment definition is absent from selected catalogue");
        return *result;
    }
    void position(Frame frame,const Vector& value) {frames_.object_position(frame,{value[0],value[1],value[2]});}
    void alpha(Frame frame) {
        if(frames_.visual_type(frame)==0x5f4a424fu)frames_.human_equipment_opacity(frame,services_.equipment_opacity());
    }
    void attach_head() {
        const auto model=state_.model(3);if(!model)return;
        frames_.human_equipment_link(model,state_.markers()[1],2);
        frames_.car_direction(model,Vector{{0,0,1,0}});frames_.object_visible(model,true);
        position(model,human_equipment_offset24);frames_.update_scene_object(model);
        const auto identity=state_.words().template get<std::int32_t>(0x44);
        const auto scalar=identity<0?std::optional<float>{}:services_.equipment_identity_scalar(identity);
        if(!scalar||*scalar>.1f)services_.equipment_face_flag(true);
        alpha(model);services_.refresh_equipment_visibility();
    }
    void attach_bag() {
        auto model=state_.model(4);if(!model)return;
        if(state_.words().template get<std::int32_t>(0x38)!=-1) {
            if(const auto* cached=items_.scuba_parts(definition(0x38))) {
                for(unsigned index=0;index!=4;++index)if(!parts_[index]&&(*cached)[index]) {
                    auto clone=ItemFrameTree<Frames>::clone(frames_,(*cached)[index].get());
                    if(!clone)throw std::runtime_error("Cannot clone required Human scuba part");
                    parts_[index]=Owner(clone.release(),Release{&frames_});
                }
                //The original dereferences all four; reject incomplete assets
                //before attachment rather than reporting a successful scuba set.
                for(const auto& part:parts_)if(!part)throw std::runtime_error("Required Human scuba part is absent");
                const std::array<unsigned,3> marker{{1,3,4}};
                for(unsigned index=0;index!=3;++index) {
                    const auto frame=parts_[index].get();
                    frames_.human_equipment_link(frame,state_.markers()[marker[index]],2);
                    frames_.car_direction(frame,Vector{{0,0,1,0}});
                    if(!index)frames_.object_visible(frame,true);
                    position(frame,index?Vector{}:human_equipment_offset24);
                    if(index)frames_.object_visible(frame,true);
                    frames_.update_scene_object(frame);
                }
                model=parts_[3].get();state_.model(4,model);
                for(const auto& part:parts_)alpha(part.get());
            }
        }
        frames_.human_equipment_link(model,state_.markers()[2],2);
        frames_.car_direction(model,Vector{{0,0,1,0}});
        if(services_.equipment_mode()<1)frames_.object_visible(model,true);
        position(model,human_equipment_offset28);frames_.update_scene_object(model);
        alpha(model);services_.refresh_equipment_visibility();
    }
    static Vector cross_rows(const Matrix& matrix) { //Native x87, second row cross first row.
        return Vector{{static_cast<float>(static_cast<long double>(matrix[5])*matrix[2]-static_cast<long double>(matrix[6])*matrix[1]),
            static_cast<float>(static_cast<long double>(matrix[6])*matrix[0]-static_cast<long double>(matrix[4])*matrix[2]),
            static_cast<float>(static_cast<long double>(matrix[4])*matrix[1]-static_cast<long double>(matrix[5])*matrix[0]),0}};
    }
    static Vector row(const Matrix& matrix,unsigned index) {return {{matrix[index*4],matrix[index*4+1],matrix[index*4+2],0}};}
    void rotate(Frame frame,const Matrix& matrix,const Vector& axis,float angle) {
        const auto rotation=frames_.human_axis_rotation(axis,angle);
        const auto product=frames_.rigid_matrix_product(matrix,rotation);
        frames_.human_equipment_local_matrix(frame,product);
    }
    void attach_back_model() {
        const auto model=state_.model(2);if(!model)return;
        const auto backlink=frames_.actor_find(model,"backlink",0x20);
        const auto mag=frames_.actor_find(model,"mag1",0xffff);if(!backlink)return;
        auto& words=state_.words();words.set(0xbc,0.f);words.set(0xc0,0.f);
        frames_.human_equipment_link(model,state_.markers()[0],2);
        frames_.car_direction(model,Vector{{0,1,0,0}});
        if(services_.equipment_mode()<1)frames_.object_visible(model,true);
        position(model,human_equipment_offset20);frames_.update_scene_object(model);
        const auto parent=frames_.object_parent(backlink);
        if(!parent)throw std::runtime_error("Human equipment backlink has no actual parent");
        const auto parent_matrix=frames_.car_local_matrix(parent);
        const auto source=frames_.car_position(backlink);
        auto offset=frames_.car_transform(Vector{{-source[0],-source[1],-source[2],0}},parent_matrix);
        offset[0]=static_cast<float>(static_cast<long double>(offset[0])+.0275f);
        offset[2]=static_cast<float>(static_cast<long double>(offset[2])+.09f);
        position(model,offset);frames_.update_scene_object(model);
        const bool long_pack=!mag||definition(0x3c).kind()==4;
        words.set(0xc4,long_pack?.4f:.29f);
        words.set(0xc8,long_pack?0x1.16872cp-4f:0x1.374bc6p-5f); //3D8B4396/3D1BA5E3.
        auto saved=frames_.car_position(model);
        if(!long_pack){saved[0]=static_cast<float>(static_cast<long double>(saved[0])-.01f);saved[2]=static_cast<float>(static_cast<long double>(saved[2])-.08f);}
        auto matrix=frames_.car_local_matrix(model);
        rotate(model,matrix,cross_rows(matrix),long_pack?-.25f:-0x1.f6a7a4p+0f); //BFFB53D2.
        position(model,saved);frames_.update_scene_object(model);
        saved=frames_.car_position(model);matrix=frames_.car_local_matrix(model);
        rotate(model,matrix,row(matrix,long_pack?1:0),long_pack?.1f:-.35f);
        position(model,saved);frames_.update_scene_object(model);
        matrix=frames_.car_local_matrix(model);
        rotate(model,matrix,Vector{{1,0,0,0}},mag?.15f:.25f);
        frames_.update_scene_object(model);
        if(!words.template get<std::int32_t>(0xcc)) {words.set(0xbc,0.f);words.set(0xc0,0.f);}
        else {
            const auto angle=words.template get<float>(0xc4);words.set(0xbc,angle);
            const auto rotation=frames_.human_axis_rotation(Vector{{1,0,0,0}},-angle);
            matrix=frames_.car_local_matrix(model);
            frames_.human_equipment_local_matrix(model,frames_.rigid_matrix_product(matrix,rotation));
            const auto shift=words.template get<float>(0xc8);words.set(0xc0,shift);
            auto value=frames_.car_position(model);value[2]=static_cast<float>(-static_cast<long double>(shift)+value[2]);position(model,value);
        }
        alpha(model); //Original case2 returns without007920B0 and without another update.
    }
    static std::int32_t payload_kind(const frontend::PreMissionItemPayload& payload) {
        if(!payload.type)return -1;
        if(payload.type==10&&payload.bytes.empty())return 9;
        if(payload.bytes.size()<8)throw std::runtime_error("Truncated hand equipment payload");
        return frontend::pre_mission_detail::Bytes::integer(frontend::pre_mission_detail::Bytes::word(payload.bytes.data()+4));
    }
    void attach_hand() {
        const auto model=state_.model(0);
        if(!model||!state_.first_volume()){services_.refresh_equipment_visibility();return;}
        auto& words=state_.words();words.set(0xd4,std::int32_t{-10000});
        if(words.template get<std::uint32_t>(0x50)&1u){attach_hand_on_back();return;}
        frames_.object_visible(model,true);frames_.human_equipment_link(model,state_.first_volume(),6);
        position(model,Vector{});frames_.car_direction(model,Vector{{1,0,0,0}});
        frames_.object_visible(model,true);frames_.update_scene_object(model);
        const auto id=words.template get<std::int32_t>(0x34);
        const auto* item=catalogue_.item(id);
        services_.equipment_primary_face(item&&(payload_kind(item->payloads[0])==0||item->kind()==9)?1u:2u);
        services_.set_equipment_weapon(model,item?static_cast<std::int32_t>(item->kind()):-1);
        services_.equipment_secondary_face(2);
        if(id!=-1&&!services_.equipment_visibility_override()) {
            if(!item)throw std::runtime_error("Hand equipment sound has no actual definition");
            //004374D0 defaults, group3, definition native40, flags0, position0,
            //relative frame005A2AD0 and owner Human. The actual SOUND+54
            //consumer must construct/play this request; no success default.
            services_.equipment_sound(item->fields38[2]);
        }
        alpha(model);services_.refresh_equipment_visibility();
    }
    Vector backlink_position(Frame backlink) {
        const auto parent=frames_.object_parent(backlink);
        if(!parent)throw std::runtime_error("Hand equipment backlink has no actual parent");
        const auto matrix=frames_.car_local_matrix(parent);const auto source=frames_.car_position(backlink);
        return frames_.car_transform(Vector{{-source[0],-source[1],-source[2],0}},matrix);
    }
    void attach_hand_on_back() {
        auto& words=state_.words();
        if(words.template get<std::int32_t>(0xd4)!=-10000&&services_.equipment_life_value()>0) {
            const auto flag=words.template get<std::uint32_t>(0x50)&1u;
            struct RestoreFlag {
                HumanEquipmentState<Frame>& state;std::uint32_t flag;
                ~RestoreFlag(){if(flag)state.words().set(0x50,state.words().template get<std::uint32_t>(0x50)|1u);}
            } restore{state_,flag};
            if(flag)words.set(0x50,words.template get<std::uint32_t>(0x50)&0xfffffffeu);
            attach_hand();
        }
        words.set(0xd4,std::int32_t{-10000});
        const auto model=state_.model(0);
        if(!model)throw std::runtime_error("Back attachment requires an actual hand model");
        const auto backlink=frames_.actor_find(model,"backlink",0x20);
        const auto kind=definition(0x34).kind();
        if(kind<3||(kind>7&&kind!=12)){frames_.object_visible(model,false);return;}
        if(!backlink)return;
        const bool other=state_.model(2)!=Frame{};
        frames_.human_equipment_link(model,state_.markers()[0],2);
        frames_.car_direction(model,Vector{{0,1,0,0}});
        if(services_.equipment_mode()<1)frames_.object_visible(model,true);
        auto offset=human_equipment_offset20;if(other)offset[0]=-offset[0];
        position(model,offset);frames_.update_scene_object(model);
        auto value=backlink_position(backlink);
        value[0]=static_cast<float>(static_cast<long double>(value[0])+(other?(state_.model(4)?.01f:.025f):.0275f));
        value[2]=static_cast<float>(static_cast<long double>(value[2])+(other?.06f:.09f));
        position(model,value);frames_.update_scene_object(model);
        const auto mag=frames_.actor_find(model,"mag1",0xffff);
        if(!mag) {
            if(!other) {
                const auto saved=frames_.car_position(model);const auto matrix=frames_.car_local_matrix(model);
                rotate(model,matrix,row(matrix,1),.1f);position(model,saved);frames_.update_scene_object(model);
            }
        }else {
            auto saved=frames_.car_position(model);
            saved[0]=static_cast<float>(static_cast<long double>(saved[0])-.01f);
            saved[2]=static_cast<float>(static_cast<long double>(saved[2])-.08f);
            auto matrix=frames_.car_local_matrix(model);
            rotate(model,matrix,cross_rows(matrix),other?-0x1.6d9130p+0f:-0x1.f6a7a4p+0f); //BFB6C898/BFFB53D2.
            position(model,saved);frames_.update_scene_object(model);
            if(!other) {
                saved=frames_.car_position(model);matrix=frames_.car_local_matrix(model);
                rotate(model,matrix,row(matrix,0),-.35f);position(model,saved);frames_.update_scene_object(model);
            }
        }
        const auto matrix=frames_.car_local_matrix(model);
        rotate(model,matrix,Vector{{1,0,0,0}},mag||other?.15f:.25f);
        frames_.update_scene_object(model);alpha(model);
    }
    Frames& frames_;Items& items_;Services& services_;
    const frontend::PreMissionItemCatalogue& catalogue_;HumanEquipmentState<Frame>& state_;
    std::array<Owner,4> parts_{};
};
} //namespace hd2::game
#endif
