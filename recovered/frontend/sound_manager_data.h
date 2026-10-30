#ifndef HD2_FRONTEND_SOUND_MANAGER_DATA_H
#define HD2_FRONTEND_SOUND_MANAGER_DATA_H

#include "frontend/module_registration.h"
#include "frontend/sound_runtime.h"

namespace hd2::frontend {

// 00433590/004336D0 and004335D0: original STATIC singleton00899728 and module6,
// name Sound. This address lies entirely in PE zero-filled storage, unlike an
// arbitrary caller's uninitialized object. Apply zero provenance, then explicit
// constructor writes (primary volume/current/target/rate1, request-1, four groups3).
// Catalog00899838 and clock00899830 are separate owners and are NOT reset here.
// The original destructor frees container storage and unregisters; it does not
// stop voices/movies or clear attached sector data. Call the actual shutdown
// operation before destroying a live owner. No automatic engine reset is added.
class SoundManagerData : public ClientModuleRegistration {
public:
    explicit SoundManagerData(ClientModuleRegistry& registry)
        : ClientModuleRegistration(registry, 6, "Sound") {
        runtime.listener_sector = nullptr;
        runtime.driver_delay = 0;
        secondary.target = 0;
        secondary.scale = 0;
        secondary.rate = 0;
        selection.pending = 0;
        selection.transition = SoundMovieRequest{0, 0, 0};
    }
    SoundRuntimeState runtime;
    SettingsSoundState settings{0, 1, nullptr, 1};
    SoundMovieFade fade;
    SecondarySoundMovieState secondary;
    SoundMovieSelection selection;
};

}
#endif
