#ifndef HD2_RECOVERED_SCENE_MODEL_LOAD_POLICY_H
#define HD2_RECOVERED_SCENE_MODEL_LOAD_POLICY_H

#include <cstdint>

namespace hd2::recovered {

enum class ModelLoadAction {
    accept,
    try_next_candidate,
    stop_failure,
};

enum class ModelLoadDiagnostic {
    none,
    bad_file_version,
    bad_file_time,
};

struct ModelLoadDecision {
    ModelLoadAction action;
    ModelLoadDiagnostic diagnostic;
};

// Classifies result bits from one candidate; it does not perform loading or retries.
ModelLoadDecision classify_model_load_result(std::uint32_t result_bits) noexcept;

} // namespace hd2::recovered

#endif
