#ifndef HD2_GAME_HUMAN_FACE_MESH_POOL_H
#define HD2_GAME_HUMAN_FACE_MESH_POOL_H

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace hd2::game {

//00796D80/00797020: seven native groups, each0x128 bytes.
//Only recovered fields are represented. Model frames/vertices are borrowed;
//nodes and edges belong to the pooled record, never to the engine mesh.
struct HumanFaceMeshData {
    using Position=std::array<float,3>;
    using Triangle=std::array<std::uint16_t,3>;
    struct Edge;
    struct Node {
        std::uint16_t vertex=0;
        std::uint8_t field04=0;
        std::vector<Edge*> edges;
    };
    struct Edge {
        Node* first=nullptr;Node* second=nullptr;
        std::uint16_t triangle=0,other_triangle=0xffff;
    };
    struct Group {
        std::int32_t index=-1;
        std::uint32_t triangle_count=0,vertex_count=0;
        std::vector<std::unique_ptr<Node>> nodes;
        std::vector<std::unique_ptr<Edge>> edges;
        std::array<std::uint32_t,63> fields2c{};

        Node* find(std::uint16_t vertex) const { //00797A50 reverse lookup.
            for(auto n=nodes.size();n;)if(nodes[--n]->vertex==vertex)return nodes[n].get();
            return nullptr;
        }
        void connect(std::uint16_t first,std::uint16_t second,std::int32_t triangle) { //00797AA0.
            auto* a=find(first);auto* b=find(second);Edge* existing=nullptr;
            const auto search=[&](Node* node,std::uint16_t other) {
                for(auto n=node->edges.size();n;) {
                    auto* edge=node->edges[--n];
                    if(edge->first->vertex==other||edge->second->vertex==other)existing=edge;
                }
                if(existing&&signed_word(existing->triangle)!=triangle&&signed_word(existing->other_triangle)<0)
                    existing->other_triangle=static_cast<std::uint16_t>(triangle);
            };
            const auto create=[&](std::uint16_t vertex) {
                auto node=std::make_unique<Node>();node->vertex=vertex;
                node->field04=static_cast<std::uint8_t>(first); //Native uses FIRST argument for both nodes.
                auto* result=node.get();nodes.push_back(std::move(node));return result;
            };
            if(a)search(a,second);else a=create(first);
            if(b)search(b,first);else b=create(second);
            if(existing) {
                if(signed_word(existing->other_triangle)<0)existing->other_triangle=static_cast<std::uint16_t>(triangle);
                return;
            }
            auto edge=std::make_unique<Edge>();edge->first=a;edge->second=b;
            edge->triangle=static_cast<std::uint16_t>(triangle);
            //Reserve before publication so an allocation failure cannot leave dangling borrowers.
            a->edges.reserve(a->edges.size()+2);b->edges.reserve(b->edges.size()+2);
            edges.reserve(edges.size()+1);
            a->edges.push_back(edge.get());b->edges.push_back(edge.get());edges.push_back(std::move(edge));
        }
        void build(unsigned group,const std::vector<Position>& vertices,const std::vector<Triangle>& triangles) {
            if(!nodes.empty()||!edges.empty())throw std::logic_error("Face mesh group is already built");
            if(vertices.size()>0x7fffffffu||triangles.size()>0x7fffffffu)
                throw std::length_error("Face mesh exceeds original signed counts");
            for(std::size_t n=0;n<triangles.size();++n) {
                const auto& t=triangles[n];
                for(auto vertex:t)if(vertex>=vertices.size())throw std::runtime_error("Face triangle vertex is outside LOD0");
                connect(t[0],t[1],static_cast<std::int32_t>(n));
                connect(t[1],t[2],static_cast<std::int32_t>(n));
                connect(t[2],t[0],static_cast<std::int32_t>(n));
            }
            //00797020 joins coincident vertices in reverse pair order, including
            //unordered x87 comparisons. It does not merge or renumber nodes.
            for(auto n=nodes.size();n;) {
                const auto first=nodes[--n]->vertex;
                for(auto m=n;m;) {
                    const auto second=nodes[--m]->vertex;
                    const auto& a=vertices[first];const auto& b=vertices[second];
                    const long double x=static_cast<long double>(b[0])-a[0],
                        y=static_cast<long double>(b[1])-a[1],z=static_cast<long double>(b[2])-a[2];
                    if(!((z*z+x*x)+y*y>static_cast<long double>(1.e-8f)))connect(first,second,0);
                }
            }
            index=static_cast<std::int32_t>(group);
            triangle_count=static_cast<std::uint32_t>(triangles.size());vertex_count=static_cast<std::uint32_t>(vertices.size());
            initialize_extrema(vertices);
        }
        void clear() noexcept { //00796E90: nodes reverse, then edges reverse.
            while(!nodes.empty())nodes.pop_back();
            while(!edges.empty())edges.pop_back();
        }
        ~Group(){clear();}
    private:
        static std::int32_t signed_word(std::uint16_t value){return value&0x8000u?static_cast<int>(value)-65536:value;}
        void initialize_extrema(const std::vector<Position>& vertices) { //00797330.
            if(nodes.empty())throw std::runtime_error("Original face group requires at least one triangle node");
            fields2c.fill(0);
            auto minimum=vertices.at(nodes.front()->vertex),maximum=minimum;
            std::array<std::uint32_t,3> low{},high{};
            for(std::size_t n=0;n<nodes.size();++n) {
                const auto& p=vertices.at(nodes[n]->vertex);
                for(unsigned axis=0;axis!=3;++axis) {
                    if(p[axis]<minimum[axis]){minimum[axis]=p[axis];low[axis]=static_cast<std::uint32_t>(n);}
                    if(maximum[axis]<p[axis]){maximum[axis]=p[axis];high[axis]=static_cast<std::uint32_t>(n);}
                }
            }
            fields2c[(0x30-0x2c)/4]=low[0];fields2c[(0x34-0x2c)/4]=high[0];
            fields2c[(0x4c-0x2c)/4]=low[2];fields2c[(0x3c-0x2c)/4]=high[2];
            fields2c[(0x6c-0x2c)/4]=low[1];fields2c[(0xac-0x2c)/4]=high[1];
        }
    };
    explicit HumanFaceMeshData(std::string name):resource(std::move(name)){}
    ~HumanFaceMeshData(){for(auto& group:groups)group.clear();}
    std::array<Group,7> groups;
    std::string resource;
    std::uint32_t references=0;
};

//Shared native0098ADF0 cache. Its containing world outlives every face owner.
//007968A0/00796B90; no independent duplicate model cache or engine ownership.
template<class Frames>class HumanFaceMeshPool {
public:
    using Frame=typename Frames::Frame;
    explicit HumanFaceMeshPool(Frames& frames):frames_(frames){}
    ~HumanFaceMeshPool(){clear();}
    HumanFaceMeshPool(const HumanFaceMeshPool&)=delete;
    HumanFaceMeshPool& operator=(const HumanFaceMeshPool&)=delete;
    HumanFaceMeshData* acquire(Frame model) {
        if(!model)return nullptr;
        const auto resource=frames_.human_face_model_resource(model);
        for(auto n=records_.size();n;)if(records_[--n]->resource==resource) {
            ++records_[n]->references;return records_[n].get();
        }
        const auto visual=frames_.actor_find(model,"a",0xffff);
        if(!visual||!(frames_.actor_visual_flags(visual)&0x800u))return nullptr;
        const auto lod=frames_.human_face_lod(visual);if(!lod)return nullptr;
        auto record=std::make_unique<HumanFaceMeshData>(resource);
        frames_.with_human_face_vertices(lod,[&](const auto& vertices) {
            for(unsigned n=0;n!=7;++n)record->groups[n].build(n,vertices,frames_.human_face_triangles(lod,n));
        });
        ++record->references;auto* result=record.get();records_.push_back(std::move(record));return result;
    }
    void release(HumanFaceMeshData* record) noexcept {
        for(auto n=records_.size();n;)if(records_[--n].get()==record) {
            if(--record->references==0) {
                records_[n].reset();
                if(n+1!=records_.size())records_[n]=std::move(records_.back());
                records_.pop_back();
            }
            return;
        }
    }
    std::size_t size() const{return records_.size();}
    void clear() noexcept {while(!records_.empty())records_.pop_back();} //007967A0; containing world retires faces first.
private:
    Frames& frames_;std::vector<std::unique_ptr<HumanFaceMeshData>> records_;
};
} //namespace hd2::game
#endif
