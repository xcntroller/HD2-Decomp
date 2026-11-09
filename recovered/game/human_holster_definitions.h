#ifndef HD2_GAME_HUMAN_HOLSTER_DEFINITIONS_H
#define HD2_GAME_HUMAN_HOLSTER_DEFINITIONS_H

#include <array>
#include <cstdint>

namespace hd2::game {
//0078CE20; fingerprinted data/addresses in human-holster-data.json.
inline constexpr std::array<const char*,10> human_holster_sets{{
    "def_holster_set",
    "en_vyb01",
    "en_vyb02",
    "en_vyb03",
    "ge_vyb01",
    "it_vyb01",
    "ja_vyb",
    "ru_vyb",
    "us_vyb",
    "us_vyb2",
}};
struct HumanHolsterDefinition {const char* name;std::int32_t set,flag;};
inline constexpr std::array<HumanHolsterDefinition,80> human_holster_definitions{{
    {"CI01",-1,6}, //0084F700
    {"CI02",-1,6}, //0084F70C
    {"CI03",-1,6}, //0084F718
    {"CI04",-1,6}, //0084F724
    {"CI05",-1,6}, //0084F730
    {"CI06",-1,6}, //0084F73C
    {"CI07",-1,6}, //0084F748
    {"CI08",-1,6}, //0084F754
    {"CI09",-1,6}, //0084F760
    {"CI10",-1,6}, //0084F76C
    {"CI11",-1,6}, //0084F778
    {"CI12",-1,6}, //0084F784
    {"CI13",-1,6}, //0084F790
    {"CI14",-1,6}, //0084F79C
    {"CI15",-1,6}, //0084F7A8
    {"CI16",-1,6}, //0084F7B4
    {"NA01",-1,6}, //0084F7C0
    {"EN01",1,0}, //0084F7CC
    {"EN02",1,0}, //0084F7D8
    {"EN03",-1,0}, //0084F7E4
    {"EN04",2,0}, //0084F7F0
    {"EN05",-1,0}, //0084F7FC
    {"EN06",3,0}, //0084F808
    {"EN07",2,0}, //0084F814
    {"EN08",1,0}, //0084F820
    {"EN09",1,0}, //0084F82C
    {"EN10",-1,0}, //0084F838
    {"EN11",-1,0}, //0084F844
    {"EN12",1,0}, //0084F850
    {"EN13",-1,0}, //0084F85C
    {"EN14",-1,0}, //0084F868
    {"NPC_EN08",1,0}, //0084F874
    {"GE01A",4,1}, //0084F880
    {"GE01B",4,1}, //0084F88C
    {"GE02A",4,1}, //0084F898
    {"GE02B",4,1}, //0084F8A4
    {"GE02C",4,1}, //0084F8B0
    {"GE02D",4,1}, //0084F8BC
    {"GE03",4,1}, //0084F8C8
    {"GE03B",4,1}, //0084F8D4
    {"GE04",4,1}, //0084F8E0
    {"GE04B",4,1}, //0084F8EC
    {"GE05",4,1}, //0084F8F8
    {"GE05B",4,1}, //0084F904
    {"GE06",4,1}, //0084F910
    {"GE07",4,1}, //0084F91C
    {"GE08",-1,1}, //0084F928
    {"GE09",4,1}, //0084F934
    {"GE09B",4,1}, //0084F940
    {"GE10",4,1}, //0084F94C
    {"GE10B",4,1}, //0084F958
    {"GE11",4,1}, //0084F964
    {"GE13",4,1}, //0084F970
    {"GE13B",4,1}, //0084F97C
    {"GE14",4,1}, //0084F988
    {"GE15",4,1}, //0084F994
    {"GE21",-1,1}, //0084F9A0
    {"GE21b",-1,1}, //0084F9AC
    {"GE27",4,1}, //0084F9B8
    {"GE27B",-1,1}, //0084F9C4
    {"GE28",-1,1}, //0084F9D0
    {"GE28B",-1,1}, //0084F9DC
    {"GE31",4,1}, //0084F9E8
    {"GE31B",4,1}, //0084F9F4
    {"GE32",-1,1}, //0084FA00
    {"NPC_GE05b",4,1}, //0084FA0C
    {"NPC_GE10b",4,1}, //0084FA18
    {"IT01",5,4}, //0084FA24
    {"IT02",5,4}, //0084FA30
    {"IT03",-1,4}, //0084FA3C
    {"IT04",5,4}, //0084FA48
    {"IT05",5,4}, //0084FA54
    {"JA01",6,3}, //0084FA60
    {"JA02",6,3}, //0084FA6C
    {"RU01",7,2}, //0084FA78
    {"RU02",7,2}, //0084FA84
    {"US01",8,5}, //0084FA90
    {"US02",8,5}, //0084FA9C
    {"US03",9,5}, //0084FAA8
    {"US04",8,5}, //0084FAB4
}};
struct HumanHolsterMarker {std::uint32_t group;const char* name;};
inline constexpr std::array<HumanHolsterMarker,24> human_holster_markers{{
    {0,"1"}, //0084FAC8
    {1,"2"}, //0084FAD0
    {2,"41"}, //0084FAD8
    {2,"42"}, //0084FAE0
    {2,"43"}, //0084FAE8
    {3,"51"}, //0084FAF0
    {3,"52"}, //0084FAF8
    {4,"6"}, //0084FB00
    {5,"7"}, //0084FB08
    {6,"81"}, //0084FB10
    {6,"82"}, //0084FB18
    {6,"83"}, //0084FB20
    {7,"91"}, //0084FB28
    {7,"92"}, //0084FB30
    {7,"93"}, //0084FB38
    {8,"31"}, //0084FB40
    {8,"32"}, //0084FB48
    {8,"33"}, //0084FB50
    {9,"aa"}, //0084FB58
    {10,"bb"}, //0084FB60
    {11,"cc"}, //0084FB68
    {12,"dd"}, //0084FB70
    {13,"ee1"}, //0084FB78
    {13,"ee2"}, //0084FB80
}};
} //namespace hd2::game
#endif
