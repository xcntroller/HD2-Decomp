#ifndef HD2_HOST_GAME_SPEECH_DEFINITIONS_H
#define HD2_HOST_GAME_SPEECH_DEFINITIONS_H

#include "host/chunk_files.h"
#include "frontend/pre_mission_data.h"
#include "game/speech_definitions.h"

namespace hd2::host {

//005AFE00's actual resolver. Borrow the SAME startup ChunkFiles and client CRT
//random source; no private seed, archive resolver or successful audio callback.
template<class Random>
class GameSpeechDefinitions {
public:
    GameSpeechDefinitions(ChunkFiles& files,Random& random):files_(files),random_(random) {}
    game::SpeechDefinition* acquire(std::int32_t index) {return definitions_.acquire(index);}
    void release(game::SpeechDefinition* value) noexcept {definitions_.release(value);}
    std::optional<std::vector<unsigned char>> read_speech_table() {
        return frontend::pre_mission_detail::read_file(files_,"Tables\\speech.set");
    }
    std::uint32_t speech_random() {return random_();}
private:
    ChunkFiles& files_;Random& random_;
    game::SpeechDefinitionCache<GameSpeechDefinitions> definitions_{*this};
};

} //namespace hd2::host
#endif
