#include "scene/model_load_policy.h"

namespace hd2::recovered {

ModelLoadDecision classify_model_load_result(std::uint32_t result_bits) noexcept {
    if (result_bits <= 0x7fffffffU) {
        return {ModelLoadAction::accept, ModelLoadDiagnostic::none};
    }
    switch (result_bits) {
    case 0x80000002U:
    case 0x8000000aU:
        return {ModelLoadAction::stop_failure, ModelLoadDiagnostic::none};
    case 0x80000013U:
        return {ModelLoadAction::try_next_candidate, ModelLoadDiagnostic::bad_file_version};
    case 0x80000014U:
        return {ModelLoadAction::try_next_candidate, ModelLoadDiagnostic::bad_file_time};
    default:
        return {ModelLoadAction::try_next_candidate, ModelLoadDiagnostic::none};
    }
}

} // namespace hd2::recovered
