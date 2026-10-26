#include "scene/model_load_policy.h"

#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    using hd2::recovered::classify_model_load_result;
    using Action = hd2::recovered::ModelLoadAction;
    using Diagnostic = hd2::recovered::ModelLoadDiagnostic;
    struct Case {
        std::uint32_t bits;
        Action action;
        Diagnostic diagnostic;
    };
    const std::vector<Case> cases{
        {0, Action::accept, Diagnostic::none},
        {1, Action::accept, Diagnostic::none},
        {0x13, Action::accept, Diagnostic::none},
        {0x14, Action::accept, Diagnostic::none},
        {0x7fffffffU, Action::accept, Diagnostic::none},
        {0x80000000U, Action::try_next_candidate, Diagnostic::none},
        {0x80000001U, Action::try_next_candidate, Diagnostic::none},
        {0x80000002U, Action::stop_failure, Diagnostic::none},
        {0x80000003U, Action::try_next_candidate, Diagnostic::none},
        {0x80000009U, Action::try_next_candidate, Diagnostic::none},
        {0x8000000aU, Action::stop_failure, Diagnostic::none},
        {0x8000000bU, Action::try_next_candidate, Diagnostic::none},
        {0x80000012U, Action::try_next_candidate, Diagnostic::none},
        {0x80000013U, Action::try_next_candidate, Diagnostic::bad_file_version},
        {0x80000014U, Action::try_next_candidate, Diagnostic::bad_file_time},
        {0x80000015U, Action::try_next_candidate, Diagnostic::none},
        {0xffffffffU, Action::try_next_candidate, Diagnostic::none},
    };
    std::size_t failures = 0;
    for (const auto& test : cases) {
        const auto actual = classify_model_load_result(test.bits);
        if (actual.action != test.action || actual.diagnostic != test.diagnostic) {
            std::cerr << "FAIL result classification for " << test.bits << '\n';
            ++failures;
        }
    }
    std::cout << cases.size() - failures << '/' << cases.size()
              << " model-load-policy tests passed\n";
    return failures == 0 ? 0 : 1;
}
