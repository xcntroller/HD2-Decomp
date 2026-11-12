#ifndef HD2_FRONTEND_NETWORK_MAP_PACKETS_H
#define HD2_FRONTEND_NETWORK_MAP_PACKETS_H
#include "frontend/game_configuration.h"
#include "game/session_network_parameters.h"
#include <cstring>
#include <limits>
namespace hd2::frontend {
inline std::uint32_t network_map_signed16(std::uint32_t value){
    const auto low=static_cast<std::uint16_t>(value);std::int16_t signed_value;std::memcpy(&signed_value,&low,2);
    return static_cast<std::uint32_t>(static_cast<std::int32_t>(signed_value));
}
inline std::uint32_t required_network_map_word(const GameConfiguration::Rotation& map,unsigned index){
    const auto& value=map.words0c.at(index);
    if(!value)throw std::runtime_error("Selected map scalar lacks its actual metadata producer");
    return *value;
}
template<class Packet>void write_network_rules(Packet& packet,const GameConfiguration& configuration,
        const game::SessionNetworkParameters& parameters){ //006AD6E0; native416 bits.
    packet.write(configuration.field174,8);packet.write(static_cast<std::uint32_t>(configuration.style224),8);
    for(auto value:configuration.rules228)packet.write(static_cast<std::uint32_t>(value),16);
    packet.write(static_cast<std::uint32_t>(parameters.word_d8),16);
    packet.write(static_cast<std::uint32_t>(parameters.word_d4),16);
    packet.write(static_cast<std::uint32_t>(parameters.milliseconds_dc/1000),16);
}
inline std::int32_t network_rule_signed16(std::uint32_t value){
    std::int32_t result;value=network_map_signed16(value);std::memcpy(&result,&value,4);return result;
}
template<class Packet>void read_network_rules(Packet& packet,GameConfiguration& configuration,
        game::SessionNetworkParameters& parameters){ //006AD780, destructive incremental updates.
    configuration.field174=packet.read(8);configuration.style224=static_cast<std::int32_t>(packet.read(8));
    for(auto& value:configuration.rules228)value=network_rule_signed16(packet.read(16));
    parameters.word_d8=network_rule_signed16(packet.read(16));
    parameters.word_d4=network_rule_signed16(packet.read(16));
    parameters.milliseconds_dc=network_rule_signed16(packet.read(16))*1000;
}
template<class Packet>void write_network_objective_row(Packet& packet,const std::array<std::uint32_t,6>& row){ //006ADB70.
    packet.write(row[1],16);packet.write(row[0],16);packet.write(row[2],16);
    packet.write(row[3],8);packet.write(row[4],2);packet.write(row[5],16);
}
template<class Packet>void read_network_objective_row(Packet& packet,std::array<std::uint32_t,6>& row){ //006ADBD0, incremental signed reads.
    row[1]=network_map_signed16(packet.read(16));row[0]=network_map_signed16(packet.read(16));row[2]=network_map_signed16(packet.read(16));
    row[3]=packet.read(8);row[4]=packet.read(2);row[5]=network_map_signed16(packet.read(16));
}
template<class Packet>void write_network_map_table_row(Packet& packet,const std::array<std::uint32_t,3>& row){ //006ADC30.
    packet.write(row[0],16);packet.write(row[1],8);packet.write(row[2],8);
}
template<class Packet>void read_network_map_table_row(Packet& packet,std::array<std::uint32_t,3>& row){ //006ADC70.
    row[0]=network_map_signed16(packet.read(16));row[1]=packet.read(8);row[2]=packet.read(8);
}
template<class Packet>void write_network_map(Packet& packet,const GameConfiguration::Rotation& map){ //006AD820.
    packet.write_string(map.text1?*map.text1:std::string{});packet.write_string(map.text5?*map.text5:std::string{});
    for(unsigned n=0;n<5;++n)packet.write(required_network_map_word(map,n),8);
    packet.write(required_network_map_word(map,5),16);packet.write_string(map.metadata44?*map.metadata44:std::string{});
    //Native count8 writers still walk the whole vector. Reject oversized
    //collections instead of emitting wrapped counts with trailing rows.
    if(map.table28.size()>255)throw std::length_error("Selected map objective count exceeds native count8");
    packet.write(static_cast<std::uint32_t>(map.table28.size()),8);
    for(const auto& row:map.table28)write_network_objective_row(packet,row);
    if(map.table38.size()>255)throw std::length_error("Selected map table count exceeds native count8");
    packet.write(static_cast<std::uint32_t>(map.table38.size()),8);
    for(const auto& row:map.table38)write_network_map_table_row(packet,row);
}
template<class Packet>void read_network_map(Packet& packet,GameConfiguration::Rotation& map){ //006AD990, deliberately destructive/partial.
    map.text1=std::make_shared<const std::string>(packet.read_string(0x1000));
    map.text5=std::make_shared<const std::string>(packet.read_string(0x1000));
    for(unsigned n=0;n<5;++n)map.words0c[n]=packet.read(8);
    map.words0c[5]=network_map_signed16(packet.read(16));
    map.metadata44=std::make_shared<const std::string>(packet.read_string(0x1000));
    auto count=packet.read(8);map.table28.clear();
    while(count--){std::array<std::uint32_t,6> row;row.fill(0xffffffffu);read_network_objective_row(packet,row);map.table28.push_back(row);}
    count=packet.read(8);map.table38.clear();
    while(count--){std::array<std::uint32_t,3> row;read_network_map_table_row(packet,row);map.table38.push_back(row);}
}
inline const GameConfiguration::Rotation& selected_network_map(const GameConfiguration& configuration){ //006AD580.
    const auto index=configuration.rotation290;
    if(index>=configuration.rotations284.size()||!configuration.rotations284[index])throw std::out_of_range("Selected map has no actual rotation record");
    return *configuration.rotations284[index];
}
template<class Packet>void write_selected_network_map(Packet& packet,const GameConfiguration& configuration){write_network_map(packet,selected_network_map(configuration));} //006AD590.
template<class Packet>void replace_selected_network_map(Packet& packet,GameConfiguration& configuration){ //006AD5B0.
    for(auto& map:configuration.rotations284)map.reset(); //Native ascending destruction, preserving vector allocation.
    configuration.rotations284.clear();
    auto map=std::make_unique<GameConfiguration::Rotation>();read_network_map(packet,*map);
    configuration.rotations284.push_back(std::move(map));configuration.rotation290=0; //Publish only AFTER full new-record read.
}
} //namespace hd2::frontend
#endif
