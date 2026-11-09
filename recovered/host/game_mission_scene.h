#ifndef HD2_HOST_GAME_MISSION_SCENE_H
#define HD2_HOST_GAME_MISSION_SCENE_H

#include "host/game_scene_actors.h"
#include "game/effects_environment.h"
#include "game/lightmap_aliases.h"
#include "game/mission_table_reader.h"

namespace hd2::host {

//Concrete006CC760 parser endpoints. The containing world supplies the SAME
//table/effects/alias owners used by its later consumers. This class does not
//supply the missing live actor/module receivers or publish world readiness.
template<class TablesBackend>
class GameMissionScene : public GameSceneActors {
public:
    GameMissionScene(GameScene& scene,MenuDriver& driver,HMODULE module,Graphics& graphics,
                     ChunkFiles& files,LoadedResources& resources,std::function<void(unsigned,float)> progress,
                     const frontend::LocalizedTextCache& text,Runtime& runtime,game::CarCallbackServices<GameActorServices>& cars,
                     game::MissionTables<TablesBackend>& tables,TablesBackend& table_storage,
                     game::EffectsEnvironment& environment,game::LightmapAliases& aliases)
        : GameSceneActors(scene,driver,module,graphics,files,resources,std::move(progress),text,runtime,cars),
          tables_(tables),table_storage_(table_storage),environment_(environment),aliases_(aliases) {}
    void read_scene_tables(game::SerializedBytes& bytes) {
        for(auto& set:tables_.sets())game::read_mission_table(bytes,set,table_storage_,*this);
    }
    Frame table_frame(const std::string& name) {
        auto* frame=resolve_scene_path(name);
        return frame?frame:find_scene_frame(name,0xcffff);
    }
    void* table_actor(const std::string& name) {
        auto* frame=table_frame(name);
        if(!frame)return nullptr;
        const auto* data=find_actor_frame_data(frame);
        return data?reinterpret_cast<void*>(data->words[0]):nullptr;
    }
    void scene_texture_parameters(game::SerializedBytes& bytes) {environment_.read_scene_configuration(bytes);}
    void read_scene_lightmap_aliases(game::SerializedBytes& bytes) {aliases_.read(bytes);}
    void read_lightmap_references(game::SerializedBytes&,const GameScene::Names&) {
        //AE00 is absent from both required runtime-selected input sets. Its
        //populated resolver must be recovered before accepting such a scene.
        throw std::runtime_error("Required AE00 light-map references are not reconstructed");
    }
    void read_mission_scene_document(game::SerializedBytes bytes,GameScene::Names& names,
                                     game::MissionSceneFields& fields,unsigned stage) {
        game::MissionSceneReader<GameMissionScene> reader(*this,names,fields);
        reader.read(bytes,stage);
    }
private:
    game::MissionTables<TablesBackend>& tables_;
    TablesBackend& table_storage_;
    game::EffectsEnvironment& environment_;
    game::LightmapAliases& aliases_;
};

} //namespace hd2::host
#endif
