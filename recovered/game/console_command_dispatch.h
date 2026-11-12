#ifndef HD2_GAME_CONSOLE_COMMAND_DISPATCH_H
#define HD2_GAME_CONSOLE_COMMAND_DISPATCH_H
#include "game/console_variables.h"
#include "game/mission_script_output_commands.h"
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <limits>
#include <string>
#include <type_traits>
namespace hd2::game {
struct ConsoleItemName {std::int32_t definition;std::string_view name;};
//Actual008442B0 pairs, sentinelFFFFFFFF/NULL at00844460; not the menu catalogue.
inline constexpr std::array<ConsoleItemName,54> console_item_names{{
    {0,"PARABELLUM"},{1,"ENFIELD"},{2,"COLT"},{3,"TAISHO"},{5,"TOKAREV"},{4,"K98"},
    {6,"LEE"},{7,"DELISLE"},{8,"MOSIN"},{10,"GARAND"},{11,"CARBINE"},{12,"ARISAKA"},
    {14,"SPRINGFIELD"},{13,"K98OPT"},{15,"SPRINGFIELDOPT"},{16,"ARISAKAOPT"},{17,"LEEOPT"},
    {18,"MP40"},{19,"ZB26"},{20,"THOMPSON"},{21,"STENGUN"},{22,"STENGUNSIL"},
    {23,"SHOTGUN"},{24,"SPAGIN"},{25,"PPS"},{26,"MP44"},{28,"BAR"},{29,"BRENGUN"},
    {31,"DEGTYAREV"},{38,"PANZERFAUST"},{39,"BAZOOKA"},{53,"MEDIKITBIG"},{54,"MEDIKIT"},
    {99,"BINOCULARS"},{229,"CAMERA"},{56,"KNIFEGER"},{57,"KNIFESAS"},{59,"WIRECUTTER"},
    {60,"GRENADE36"},{62,"GRENADE69"},{61,"GRENADEGER"},{63,"GRENADEJAP"},{66,"CLUMMYBOMB"},
    {67,"EXPLOSIVESBAG"},{68,"DYNAMITE"},{70,"MINE"},{71,"TANKMINE"},{251,"MAGNETICMINE"},
    {95,"BACKPACKRUS"},{97,"BACKPACKGER"},{101,"BACKPACKGB1"},{222,"BACKPACKGB2"},
    {228,"BACKPACKGB3"},{231,"AQUALUNG"}
}};
inline bool console_item_name_equal(std::string_view first,std::string_view second){ //007E88F0, actual ASCII aliases.
    if(first.size()!=second.size())return false;
    const auto upper=[](unsigned char c){return c>='a'&&c<='z'?c-'a'+'A':c;};
    for(std::size_t n=0;n<first.size();++n)if(upper(first[n])!=upper(second[n]))return false;
    return true;
}
inline std::size_t console_text_length(const char* text,std::size_t capacity){
    if(!text||!capacity)throw std::invalid_argument("Console requires its actual bounded input buffer");
    const auto* end=std::find(text,text+capacity,'\0');if(end==text+capacity)throw std::invalid_argument("Unterminated console input");
    return static_cast<std::size_t>(end-text);
}
//0045F020, the actual Players roster; required005A6060 stays a Human operation.
template<class Queries>void heal_console_roster(Queries& queries){
    if((queries.mode()&7u)!=7u||queries.difficulty()==4)return;
    for(auto n=queries.roster_count();n;)if(auto* human=queries.roster_human(--n))
        if(queries.health(*human)>0)queries.set_health_percent(*human,100);
}
//Bind the real committed Human damage service where it is available. An older
//Human checkpoint must supply its explicit required producer, never set health
//directly as a substitute for native D4(kind6,-1,zero,zero,NULL,0).
template<class Services,class Human,class=void>struct HasConsoleHumanDamage:std::false_type{};
template<class Services,class Human>struct HasConsoleHumanDamage<Services,Human,std::void_t<decltype(std::declval<Services&>().apply_human_damage(
    std::declval<Human&>(),6u,-1.f,std::declval<const std::array<float,4>&>(),std::declval<const std::array<float,4>&>(),nullptr,0u))>>:std::true_type{};
template<class Services,class Human,class Required>void console_human_kill(Services& services,Human& human,Required&& required){
    if constexpr(HasConsoleHumanDamage<Services,Human>::value){const std::array<float,4> zero{};services.apply_human_damage(human,6,-1.f,zero,zero,nullptr,0);}
    else required(human);
}
//00720130/00720340 use one process descriptor/value owner. Queries are actual
//in-call World/Players/Items/input and required gameplay/transport receivers.
template<class Queries>class ConsoleCommandDispatch {
public:
    ConsoleCommandDispatch(ConsoleVariables& variables,Queries& queries):variables_(variables),queries_(queries){}
    std::uint32_t console(char* input,std::size_t capacity){ //00720130, ECX=input.
        const auto length=console_text_length(input,capacity);
        if(capacity<2)throw std::invalid_argument("Console prefix query requires two addressable bytes");
        if(input[0]=='.')return command(1,input+1,capacity-1);
        //Original compares SECOND byte with comma and still passes input+1.
        //Preserve even the empty edit buffer's retained second byte.
        if(input[1]==',')return command(2,input+1,capacity-1);
        std::size_t token=0;while(token<length&&input[token]!=' ')++token;
        if(token>=1000)throw std::length_error("Console token exceeds original local buffer");
        std::size_t tail=token;while(tail<length&&input[tail]==' ')++tail;
        if(const auto index=variables_.command(std::string_view(input,token)))return command(*index,input+tail,capacity-tail);
        std::string line(input,token);line+=' ';line.append(input+tail,length-tail);
        if(line.size()<500)queries_.system_command(line); //Actual process98C0FC owner is required; no new one here.
        return 0;
    }
    std::uint32_t query_variable(std::size_t index){ //00720940, actual table has no string variables.
        const auto& descriptor=ConsoleVariables::descriptors.at(index);if(!descriptor.variable)return 7;
        if(descriptor.type!=1)throw std::logic_error("Fixed console table unexpectedly contains a noninteger variable");
        retail_mission_script_output(std::string(descriptor.name)+" = "+std::to_string(variables_.integer(index))); //Genuine0069BFE0 RET.
        return 0;
    }
    std::uint32_t command(std::size_t index,char* input,std::size_t capacity){ //00720340, ECX=INDEX/EDX=arguments.
        auto length=console_text_length(input,capacity);while(length&&*input==' '){++input;--capacity;--length;}
        while(length&&input[length-1]==' ')input[--length]='\0';
        const auto& descriptor=ConsoleVariables::descriptors.at(index);
        if(descriptor.variable&&!length)return query_variable(index);
        std::int32_t integer=0;
        if(descriptor.word4==0){if(length)return 3;}
        else if(descriptor.word4!=1)return 3;
        else if(descriptor.type==1){ //Native sscanf("%i") accepts base prefixes and trailing bytes.
            char* end=nullptr;errno=0;const auto value=std::strtoll(input,&end,0);
            if(end==input||errno==ERANGE||value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())return 4;
            integer=static_cast<std::int32_t>(value);
        }else if(descriptor.type!=2)return 3;
        else if(length&&input[0]=='"'&&input[length-1]=='"'){
            if(length<2)throw std::invalid_argument("Single quote would underwrite the native console argument");
            ++input;--capacity;length-=2;input[length]='\0';
        }
        if(descriptor.variable){if(descriptor.type!=1)throw std::logic_error("Fixed console table has no string-variable owner");return variables_.set_integer_variable(index,integer);}
        const auto mode=queries_.mode();
        switch(index){
        case 0:queries_.sensitivity(integer);return 0;
        case 1:case 2:queries_.chat(std::string_view(input,std::min<std::size_t>(length,99)),index==2);return 0; //0072EA90 terminates byte99.
        case 3:{auto* human=queries_.current_human();
            if((queries_.mode()&7u)==7u){if(human)queries_.kill(*human);}
            else if(human){const auto id=queries_.identity(*human);if(id!=-1&&id==queries_.local_identity())queries_.network_kill(id);}
            return 0;}
        case 4:if((mode&7u)==7u&&queries_.difficulty()!=4)variables_.happy_ammunition(integer>0);return 0;
        case 7:if((mode&7u)==7u&&queries_.difficulty()!=4)queries_.heal_roster();return 0;
        case 8:
            if((mode&7u)!=7u||queries_.difficulty()==4)return 0;
            for(const auto& entry:console_item_names)if(console_item_name_equal(entry.name,std::string_view(input,length))){
                const auto id=entry.definition;if(id<0||id>=500||!queries_.has_definition(id))return 0;
                queries_.give_item(id,queries_.current_human());
                if(queries_.ammunition(id)>=0)for(unsigned n=0;n<6;++n){auto* human=queries_.current_human();const auto ammo=queries_.ammunition(id);queries_.give_item(ammo,human);}
                else if(queries_.kind(id)==9||queries_.kind(id)==11)for(unsigned n=0;n<5;++n)queries_.give_item(id,queries_.current_human());
                return 0;
            }
            return 0;
        case 9:return 0; //Original anorexy case genuinely has no action after parsing.
        default:return 2;
        }
    }
private:
    ConsoleVariables& variables_;Queries& queries_;
};
} //namespace hd2::game
#endif
