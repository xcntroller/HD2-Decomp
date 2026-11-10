#ifndef HD2_GAME_MISSION_INTEGRITY_TABLES_H
#define HD2_GAME_MISSION_INTEGRITY_TABLES_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace hd2::game {

//006E9EA0's process tables. This is actual scalar storage/initialization, not
//the original-address integrity VM or a successful protection service. The
//containing startup supplies the real seed and SAME CRT stream; loader/API
//continuations must publish their actual observed values before use.
class MissionIntegrityTables {
public:
    static constexpr std::size_t count=128;
    template<class Random>void initialize(std::uint32_t seed,Random& random) {
        initialized_=false;
        for(std::size_t n=0;n<count;++n)expected_indices_[n]=observed_indices_[n]=static_cast<std::uint16_t>(n);
        random.seed(seed);
        for(std::size_t n=0;n<count;++n){
            const auto a=random()&0x7fu;const auto old_observed=observed_indices_[n];
            observed_indices_[n]=observed_indices_[a];observed_indices_[a]=old_observed;
            const auto b=random()&0x7fu;const auto old_expected=expected_indices_[n];
            expected_indices_[n]=expected_indices_[b];expected_indices_[b]=old_expected;
            observed_[n]=random();expected_[n]=static_cast<std::uint32_t>(random())<<16;
            random_a530_[n]=random();random_a930_[n]=random();random_b030_[n]=random();
        }
        initialized_=true;
    }
    bool initialized()const{return initialized_;}
    std::uint32_t& expected(std::size_t stage){require();return expected_.at(expected_indices_.at(stage));}
    std::uint32_t& observed(std::size_t stage){require();return observed_.at(observed_indices_.at(stage));}
    std::uint32_t& api_result(std::size_t stage){require();return api_results_.at(observed_indices_.at(stage));}
    const auto& expected_indices()const{return expected_indices_;}
    const auto& observed_indices()const{return observed_indices_;}
    const auto& expected_values()const{return expected_;}const auto& observed_values()const{return observed_;}
    const auto& random_a530()const{return random_a530_;}const auto& random_a930()const{return random_a930_;}
    const auto& random_b030()const{return random_b030_;}
    template<class Random>bool objective_failure_trial(Random& random) { //0069BA60 offline prelude, stage13.
        if(expected(13)==observed(13))return false;
        return static_cast<long double>(random())<3276.699951171875f; //008159E4; draw ONLY on mismatch.
    }
private:
    void require()const{if(!initialized_)throw std::logic_error("Mission integrity tables require their actual process initializer");}
    std::array<std::uint32_t,count> random_a530_{},expected_{},random_a930_{},api_results_{},observed_{},random_b030_{};
    std::array<std::uint16_t,count> expected_indices_{},observed_indices_{};
    bool initialized_=false;
};

} //namespace hd2::game
#endif
