#ifndef HD2_GAME_ITEM_HAND_MODELS_H
#define HD2_GAME_ITEM_HAND_MODELS_H

#include <array>
#include <optional>
#include <stdexcept>
#include <utility>

namespace hd2::game {

//0098B868/86C/870, separate from the selected definition caches. Frames is
//borrowed only during acquisition; its model owners must have engine-lifetime
//deleters. The containing world clears these before retiring its engine scene.
template<class Frames>class ItemHandModels {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().create_item_template_model());
    Frame get(Frames& frames,unsigned kind) { //007D76C0, literal resource names.
        if(kind>=models_.size())return Frame{};
        if(models_[kind])return models_[kind]->get();
        auto model=frames.create_item_template_model();
        if(!model)throw std::runtime_error("Cannot create shared item hand model");
        constexpr std::array<const char*,3> names{{"m_karty","m_bottle","m_cigarette"}};
        if(frames.load_item_template_model(model.get(),names[kind])!=0)return Frame{};
        //Native publishes the model BEFORE configuring the optional lensf.
        models_[kind].emplace(std::move(model));
        if(kind==2)frames.configure_item_cigarette(models_[kind]->get());
        return models_[kind]->get();
    }
    void clear()noexcept { //007DD240, ascending cards/bottle/cigarette.
        for(auto& model:models_)model.reset();
    }
private:
    std::array<std::optional<Owner>,3> models_;
};
} //namespace hd2::game
#endif
