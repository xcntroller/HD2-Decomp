#ifndef HD2_GAME_HUMAN_POSE_CHAINS_H
#define HD2_GAME_HUMAN_POSE_CHAINS_H

#include "game/human_bone_direction.h"
#include <algorithm>

namespace hd2::game {

//The temporary290..29C graph uses the SAME23 bones as the original2AC..2B8
//skeleton neighbours. It owns working matrices and borrowed indices only.
template<class Frames>class HumanPoseChains {
public:
    using Frame=typename Frames::Frame;
    using Matrix=HumanMatrix;
    using Vector=HumanVector;
    struct Objective {int bone=-1;Vector target{};float weight=0;};
    struct Chain {std::array<Objective,4> objectives{};int root=-1;std::int32_t count=0;};
    struct Node {
        Matrix local140{},world180{},output200{},basis240{};
        Frame source280{};
        float length284=0;
        std::uint32_t mask288=0;
        int constraint_bone=-1;
        unsigned constraint_slot=0;
        std::array<int,3> children{{-1,-1,-1}};
        int parent=-1;
    };
    HumanPoseChains(Frames& frames,const HumanSkeleton<Frame>& skeleton):frames_(frames),skeleton_(skeleton){}
    const auto& nodes()const{return nodes_;}
    void clear(){nodes_={};}
    void reset_graph(){ //007A5B10, ECX points to the containing geometry.
        for(auto& node:nodes_){node.mask288=0;node.children.fill(-1);node.parent=-1;}
    }
    void build(const Chain& chain){ //007A5A30.
        if(chain.count<1)return;
        if(chain.count>4)throw std::out_of_range("Human pose chain exceeds its four retained objectives");
        auto& root=nodes_.at(static_cast<std::size_t>(chain.root));
        const auto frame=bone(chain.root).frame;
        root.local140=frames_.actor_world_matrix(frame);
        root.world180=frames_.actor_world_matrix(frame);
        root.length284=0;
        std::uint32_t mask=1;
        for(std::int32_t objective=0;objective<chain.count;++objective){
            root.mask288|=mask;
            build_path(chain.root,chain.objectives[static_cast<std::size_t>(objective)].bone,mask);
            mask*=2;
        }
    }
    void refresh(const Chain& chain){ //007A6590.
        auto& root=nodes_.at(static_cast<std::size_t>(chain.root));
        const auto frame=bone(chain.root).frame;
        root.local140=frames_.actor_world_matrix(frame);
        root.world180=frames_.actor_world_matrix(frame);
        for(auto child:root.children)if(child>=0)refresh_child(child,chain.root,root.local140);
    }
    void propagate(int index){ //007A73E0.
        auto& node=nodes_.at(static_cast<std::size_t>(index));
        if(node.parent>=0)node.world180=frames_.rigid_matrix_product(node.local140,nodes_[static_cast<std::size_t>(node.parent)].world180);
        for(auto child:node.children)if(child>=0)propagate(child);
    }
    void solve(const Chain& chain,std::int32_t iterations,bool move_anchor){ //007A67E0.
        if(chain.count>4)throw std::out_of_range("Human pose solver exceeds its four retained objectives");
        std::int32_t completed=0;
        for(;completed<iterations;++completed){
            float maximum=0;
            for(std::int32_t index=0;index<chain.count;++index){
                const auto& objective=chain.objectives[static_cast<std::size_t>(index)];
                const auto& node=nodes_.at(static_cast<std::size_t>(objective.bone));
                for(unsigned axis=0;axis!=3;++axis){
                    const float offset=static_cast<float>(static_cast<long double>(node.length284)*node.world180[8+axis]);
                    const float desired=objective.target[axis]-node.world180[12+axis];
                    const float error=desired-offset;
                    if(maximum<std::abs(error))maximum=std::abs(error);
                }
            }
            if(maximum<.01)break; //Original DOUBLE00811968.
            for(auto child:nodes_.at(static_cast<std::size_t>(chain.root)).children)
                if(child>=0)solve_node(chain,child);
        }
        if(completed!=iterations||!move_anchor)return;
        Vector shift{};
        for(std::int32_t index=0;index<chain.count;++index){
            const auto& objective=chain.objectives[static_cast<std::size_t>(index)];
            const auto& node=nodes_.at(static_cast<std::size_t>(objective.bone));
            for(unsigned axis=0;axis!=3;++axis){
                const auto offset=static_cast<long double>(node.length284)*node.world180[8+axis];
                const auto desired=static_cast<long double>(objective.target[axis])-node.world180[12+axis];
                const float error=static_cast<float>((axis==2?desired:static_cast<float>(desired))-offset);
                shift[axis]+=error;
            }
        }
        const auto scale=1.L/chain.count;
        for(unsigned axis=0;axis!=3;++axis)shift[axis]=static_cast<float>(shift[axis]*scale);
        const auto anchor=bone(22).frame;
        const auto world=frames_.actor_world_matrix(anchor);
        for(unsigned axis=0;axis!=3;++axis)shift[axis]+=world[12+axis];
        const auto parent=frames_.object_parent(anchor);
        if(parent){
            const auto origin=frames_.actor_world_matrix(parent);
            for(unsigned axis=0;axis!=3;++axis)shift[axis]-=origin[12+axis];
            Vector local=shift;
            for(unsigned row=0;row!=3;++row){
                const auto matrix=frames_.actor_world_matrix(parent);
                local[row]=static_cast<float>((static_cast<long double>(shift[2])*matrix[row*4+2]+
                    static_cast<long double>(shift[1])*matrix[row*4+1])+static_cast<long double>(shift[0])*matrix[row*4]);
            }
            shift=local;
        }
        frames_.object_position(anchor,{{shift[0],shift[1],shift[2]}});
        frames_.update_scene_object(anchor);
    }
    void publish(const Chain& chain){ //007A5FC0.
        auto& root=nodes_.at(static_cast<std::size_t>(chain.root));
        root.output200=frames_.actor_world_matrix(bone(chain.root).frame);
        for(auto child:root.children)if(child>=0)derive_output(child,chain.root);
        Vector displacement{},result{};
        publish_local(chain.root,-1,displacement,result);
        const auto anchor=bone(22).frame;
        const auto world=frames_.actor_world_matrix(anchor);
        frames_.object_position(anchor,{{result[0]+world[12],result[1]+world[13],result[2]+world[14]}});
        frames_.update_scene_object(anchor);
    }
private:
    using Axis=std::array<long double,3>;
    static Axis normalized_projection(const Vector& direction,long double unrounded_z,const Matrix& matrix){
        Axis result;
        result[0]=(unrounded_z*matrix[2]+static_cast<long double>(direction[1])*matrix[1])+
            static_cast<long double>(direction[0])*matrix[0];
        result[1]=(static_cast<long double>(direction[2])*matrix[6]+static_cast<long double>(direction[1])*matrix[5])+
            static_cast<long double>(direction[0])*matrix[4];
        const auto z=(static_cast<long double>(direction[2])*matrix[10]+static_cast<long double>(direction[1])*matrix[9])+
            static_cast<long double>(direction[0])*matrix[8];
        result[2]=static_cast<float>(z);
        const auto square=(z*result[2]+result[1]*result[1])+result[0]*result[0];
        constexpr float epsilon=9.99999993922529e-09f;
        if(std::abs(square-1)<epsilon)return result;
        const float stored=static_cast<float>(square);
        if(!(stored<epsilon)){
            const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));
            result[0]*=inverse;result[1]*=inverse;result[2]=static_cast<float>(result[2]*inverse);
        }else{
            const auto axis=result[0]!=0?0u:result[2]!=0?2u:1u;
            result[axis]=result[axis]<0?-1.L:1.L;
        }
        return result;
    }
    static std::optional<Vector> correction_quaternion(const Vector& cross,const HumanBoneConstraint& constraint,bool first){
        Vector axis{};
        long double weighted_z=0;
        for(unsigned component=0;component!=3;++component){
            const auto value=(constraint.flags&(1u<<component))?cross[component]:0.f;
            const auto weighted=static_cast<long double>(value)*constraint.weight;
            axis[component]=static_cast<float>(weighted);
            if(component==2)weighted_z=weighted;
        }
        const float square=static_cast<float>((weighted_z*axis[2]+static_cast<long double>(axis[1])*axis[1])+
            static_cast<long double>(axis[0])*axis[0]);
        std::int32_t bits;std::memcpy(&bits,&square,4);
        if(bits<0x322bcc77)return std::nullopt;
        constexpr float epsilon=9.99999993922529e-09f;
        float length=1;
        if(!(std::abs(static_cast<long double>(square)-1)<epsilon)){
            const auto root=std::sqrt(static_cast<long double>(square));
            length=static_cast<float>(root);
            if(!(root>epsilon))return std::nullopt;
            if(length>1)length=1;
        }
        const auto inverse=1.L/length;
        for(unsigned component=0;component!=3;++component)axis[component]=static_cast<float>(axis[component]*inverse);
        const float angle=static_cast<float>(std::asin(static_cast<long double>(length))*.5);
        const auto sine=std::sin(static_cast<long double>(angle));
        Vector result{};
        result[0]=static_cast<float>(sine*axis[0]);
        const auto stored_sine=first?static_cast<float>(sine):sine;
        result[1]=static_cast<float>(stored_sine*axis[1]);result[2]=static_cast<float>(stored_sine*axis[2]);
        result[3]=static_cast<float>(std::cos(static_cast<long double>(angle)));
        return result;
    }
    void solve_node(const Chain& chain,int index){ //007A6C70, children before this joint.
        auto& node=nodes_.at(static_cast<std::size_t>(index));
        for(auto child:node.children)if(child>=0)solve_node(chain,child);
        std::optional<Vector> correction;
        std::uint32_t mask=1;
        for(std::int32_t target=0;target<chain.count;++target,mask*=2){
            if(!(node.mask288&mask))continue;
            const auto& constraint=bone(node.constraint_bone).constraints.at(node.constraint_slot);
            if(!constraint.flags)continue;
            const auto& objective=chain.objectives[static_cast<std::size_t>(target)];
            Vector desired=objective.target;
            for(unsigned axis=0;axis!=3;++axis)desired[axis]-=node.world180[12+axis];
            const auto desired_z=static_cast<long double>(objective.target[2])-node.world180[14];
            const auto direction=normalized_projection(desired,desired_z,node.world180);
            const auto& endpoint=nodes_.at(static_cast<std::size_t>(objective.bone));
            Vector current=objective.target;
            const auto x=static_cast<long double>(endpoint.length284)*endpoint.world180[8]+endpoint.world180[12];
            const float y=static_cast<float>(static_cast<long double>(endpoint.length284)*endpoint.world180[9]);
            const float z=static_cast<float>(static_cast<long double>(endpoint.length284)*endpoint.world180[10]);
            const float endpoint_z=z+endpoint.world180[14];
            current[0]=static_cast<float>(x-node.world180[12]);
            current[1]=static_cast<float>((static_cast<long double>(y)+endpoint.world180[13])-node.world180[13]);
            const auto current_z=static_cast<long double>(endpoint_z)-node.world180[14];
            current[2]=static_cast<float>(current_z);
            const auto orientation=normalized_projection(current,current_z,node.world180);
            const Vector cross{{static_cast<float>(orientation[2]*direction[1]-orientation[1]*direction[2]),
                static_cast<float>(orientation[0]*direction[2]-orientation[2]*direction[0]),
                static_cast<float>(orientation[1]*direction[0]-orientation[0]*direction[1]),0}};
            const auto next=correction_quaternion(cross,constraint,!correction);
            if(!next)continue;
            if(correction)correction=frames_.physics_slerp(*correction,*next,objective.weight*.5f,false);
            else correction=next;
        }
        if(!correction)return;
        const auto rotation=frames_.human_matrix_rotation(node.local140);
        frames_.human_set_matrix_rotation(node.local140,multiply_human_bone_rotations(*correction,rotation));
        propagate(index);
    }
    const auto& bone(int index)const{return skeleton_.bones().at(static_cast<std::size_t>(index));}
    bool path_contains(int target,int index,int previous)const{ //007A5B40, original skeleton traversal.
        if(index==target)return true;
        const auto& current=bone(index);
        for(auto child:current.children)if(child>=0&&child!=previous&&path_contains(target,child,index))return true;
        return current.parent>=0&&current.parent!=previous&&path_contains(target,current.parent,index);
    }
    static Matrix reversed_basis(const Matrix& matrix){ //007A21F0.
        Matrix result{};
        for(unsigned axis=0;axis!=3;++axis){result[axis]=-matrix[axis];result[4+axis]=matrix[4+axis];result[8+axis]=-matrix[8+axis];}
        result[15]=1;return result;
    }
    void build_path(int index,int target,std::uint32_t mask){ //007A5C00.
        auto previous_world=nodes_.at(static_cast<std::size_t>(index)).world180;
        int previous=-1;
        while(index!=target){
            const auto& source=bone(index);
            int next=-1;unsigned edge=0;
            for(;edge!=3;++edge){
                const auto child=source.children[edge];
                if(child>=0&&child!=previous&&path_contains(target,child,index)){next=child;break;}
            }
            auto& current=nodes_[static_cast<std::size_t>(index)];
            if(next>=0){
                auto& node=nodes_[static_cast<std::size_t>(next)];
                node.basis240=source.bases[edge];node.source280=source.frame;node.length284=source.lengths[edge];
            }else{
                next=source.parent;
                if(next<0||next==previous||!path_contains(target,next,index))return;
                const auto& parent=bone(next);
                const auto found=std::find(parent.children.begin(),parent.children.end(),index);
                if(found==parent.children.end())return;
                const auto slot=static_cast<std::size_t>(found-parent.children.begin());
                auto& node=nodes_[static_cast<std::size_t>(next)];
                node.basis240=reversed_basis(parent.bases[slot]);node.source280=parent.frame;node.length284=parent.lengths[slot];
                edge=3;
            }
            auto& node=nodes_[static_cast<std::size_t>(next)];
            node.constraint_bone=index;node.constraint_slot=edge;
            if(!node.mask288){
                node.world180=frames_.rigid_matrix_product(node.basis240,frames_.actor_world_matrix(node.source280));
                auto inverse=human_transpose(previous_world);inverse[3]=inverse[7]=inverse[11]=0;
                node.local140=frames_.rigid_matrix_product(node.world180,inverse);
                node.local140[12]=node.local140[13]=0;node.local140[14]=current.length284;
                node.parent=index;
                const auto empty=std::find(current.children.begin(),current.children.end(),-1);
                if(empty==current.children.end())return;
                *empty=next;
            }
            node.mask288|=mask;previous_world=node.world180;previous=index;index=next;
        }
    }
    void refresh_child(int index,int parent,const Matrix& parent_world){ //007A6670.
        auto& node=nodes_.at(static_cast<std::size_t>(index));
        node.world180=frames_.rigid_matrix_product(node.basis240,frames_.actor_world_matrix(node.source280));
        auto inverse=human_transpose(parent_world);inverse[3]=inverse[7]=inverse[11]=0;
        node.local140=frames_.rigid_matrix_product(node.world180,inverse);
        node.local140[12]=node.local140[13]=0;node.local140[14]=nodes_[static_cast<std::size_t>(parent)].length284;
        if(node.source280!=bone(index).frame){
            const auto actual=frames_.actor_world_matrix(bone(index).frame);
            for(unsigned axis=0;axis!=4;++axis)node.world180[12+axis]=actual[12+axis];
        }
        for(auto child:node.children)if(child>=0)refresh_child(child,index,node.world180);
        for(unsigned axis=0;axis!=4;++axis)node.world180[12+axis]=parent_world[12+axis];
    }
    void derive_output(int index,int parent){ //007A60F0.
        auto& node=nodes_.at(static_cast<std::size_t>(index));
        const auto output=frames_.rigid_matrix_product(human_transpose(node.basis240),node.world180);
        if(node.source280==bone(index).frame){
            node.output200=output;
            for(unsigned axis=0;axis!=3;++axis)
                node.output200[12+axis]=static_cast<float>(static_cast<long double>(node.length284)*node.world180[8+axis]+node.world180[12+axis]);
        }else nodes_[static_cast<std::size_t>(parent)].output200=output;
        for(auto child:node.children)if(child>=0)derive_output(child,index);
    }
    void publish_local(int index,int previous,Vector displacement,Vector& result){ //007A6230.
        auto& node=nodes_.at(static_cast<std::size_t>(index));
        const auto& current=bone(index);
        if(node.mask288){
            Matrix local=node.output200;
            const bool boundary=current.parent<0||!nodes_[static_cast<std::size_t>(current.parent)].mask288;
            Matrix inverse;
            if(boundary){
                const auto parent=frames_.object_parent(current.frame);
                if(!parent)throw std::logic_error("Human pose publication requires its actual engine frame parent");
                inverse=human_transpose(frames_.actor_world_matrix(parent));
            }else{
                const auto& parent=nodes_[static_cast<std::size_t>(current.parent)].output200;
                inverse=human_transpose(parent);
                for(unsigned axis=0;axis!=3;++axis)local[12+axis]-=parent[12+axis];
            }
            inverse[3]=inverse[7]=inverse[11]=0;
            local=frames_.rigid_matrix_product(local,inverse);
            if(boundary)(void)frames_.car_local_matrix(current.frame); //Actual004C8140 before preserving B0.
            orthonormalize_human_bone_matrix(local);
            if(boundary){
                const auto actual=frames_.actor_world_matrix(current.frame);
                for(unsigned axis=0;axis!=3;++axis)displacement[axis]=node.output200[12+axis]-actual[12+axis];
            }
            if(boundary)frames_.human_store_bone_rotation(current.frame,local);
            else if(node.children[0]>=0)frames_.human_equipment_local_matrix(current.frame,local);
        }
        for(auto child:current.children)if(child>=0&&child!=previous)publish_local(child,index,displacement,result);
        if(current.parent>=0){if(current.parent!=previous)publish_local(current.parent,index,displacement,result);return;}
        result=displacement;
    }
    Frames& frames_;
    const HumanSkeleton<Frame>& skeleton_;
    std::array<Node,23> nodes_{};
};

} //namespace hd2::game
#endif
