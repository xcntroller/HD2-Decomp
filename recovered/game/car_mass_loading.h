#ifndef HD2_GAME_CAR_MASS_LOADING_H
#define HD2_GAME_CAR_MASS_LOADING_H

#include "game/car_auxiliary_geometry.h"
#include "game/car_physics.h"

namespace hd2::game {

//004F3950's actual seat/ballast load owner.004E2E30 zeroes each entire native
//record before filling its position/direction; the weights therefore start0.
//Geometry positions are borrowed from the retained SAME model geometry.
template<class Frames>class CarMassLoading {
public:
    using Auxiliary=CarAuxiliaryGeometry<Frames>;using Vector=typename Frames::Vector;
    CarMassLoading(CarPhysics& physics,const Auxiliary& geometry)
        :physics_(physics),geometry_(geometry),seats_(geometry.seats().size(),0),ballast_(geometry.ballast().size(),0){}
    void initialize(bool& changed){ //004EA780 clears seats then004F3950(0,0).
        std::fill(seats_.begin(),seats_.end(),0);seat(0,0,changed);
    }
    void seat(std::size_t index,float mass,bool& changed){ //004F3950.
        if(index>=seats_.size())return;
        if(!std::isfinite(mass))throw std::runtime_error("Non-finite Car seat mass");
        seats_[index]=mass;Vector average=geometry_.seats().front().position;float total=seats_.front();
        for(std::size_t n=1;n<seats_.size();++n)merge(average,total,geometry_.seats()[n].position,seats_[n]);
        for(auto n=ballast_.size();n;) {--n;merge(average,total,geometry_.ballast()[n].position,ballast_[n]);}
        const auto& fields=physics_.scalars();std::array<float,3> offset{},centre{},inertia{};
        const float ratio=static_cast<float>(static_cast<long double>(total)/(static_cast<long double>(total)+fields.get(0x91c)));
        for(unsigned axis=0;axis!=3;++axis){
            offset[axis]=average[axis]-fields.get(0x1f0+axis*4);
            offset[axis]*=ratio; //004F3CB0, FLOAT scale and three separate stores.
            centre[axis]=fields.get(0x1f0+axis*4)+offset[axis];
        }
        //0040E0B0 spills the extended XYZ sum to FLOAT before its tiny/unit
        //branches and sqrt; the returned sqrt remains extended in004F3950.
        const float square=static_cast<float>((static_cast<long double>(offset[0])*offset[0]+
            static_cast<long double>(offset[1])*offset[1])+static_cast<long double>(offset[2])*offset[2]);
        if(!std::isfinite(square))throw std::runtime_error("Non-finite Car load centre");
        std::int32_t bits;std::memcpy(&bits,&square,4);
        const long double length=bits<0x322bcc77?0.L:std::abs(square-1.f)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
        const auto moment=length*length*total;
        for(unsigned axis=0;axis!=3;++axis)inertia[axis]=static_cast<float>(1.L/(1.L/fields.get(0x230+axis*4)+moment));
        changed=true;physics_.load_mass_result(centre,inertia);
        //Original does not update mass918 or gravity204 here.
    }
    const auto& seat_masses()const{return seats_;}
    const auto& ballast_masses()const{return ballast_;}
private:
    static void merge(Vector& average,float& total,const Vector& point,float mass){
        std::array<float,3> displacement{};
        for(unsigned axis=0;axis!=3;++axis)displacement[axis]=point[axis]-average[axis];
        if(mass>=.0010000000474974513f||total>=.0010000000474974513f){
            const auto ratio=static_cast<long double>(mass)/(static_cast<long double>(total)+mass);
            for(auto& component:displacement)component=static_cast<float>(component*ratio);
        }else displacement={};
        for(unsigned axis=0;axis!=3;++axis)average[axis]+=displacement[axis];
        total+=mass;
    }
    CarPhysics& physics_;const Auxiliary& geometry_;std::vector<float> seats_,ballast_;
};

} //namespace hd2::game
#endif
