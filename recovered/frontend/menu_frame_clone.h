#ifndef HD2_FRONTEND_MENU_FRAME_CLONE_H
#define HD2_FRONTEND_MENU_FRAME_CLONE_H
#include <stdexcept>
#include <utility>
#include <vector>
namespace hd2::frontend {
// LS3DF1001D680 links borrow child frames; they do not acquire references.
// Keep each allocation alive and destroy descendants before their parent.
template <class Backend> struct MenuFrameClone {
    typename Backend::Owner frame;
    std::vector<MenuFrameClone> children;
    typename Backend::Frame get() const { return frame.get(); }
};

//007DC3A0/0066D9F0: recursive frame/visual factory and copy, then child links.
// Native tree ownership replaces the original explicit frame lifetime management.
template <class Backend>
MenuFrameClone<Backend> clone_menu_frame(Backend &backend, typename Backend::Frame source,
                                       typename Backend::Frame parent = {}, unsigned depth = 0) {
    if (!source || depth > 32)
        throw std::runtime_error("Invalid menu clone hierarchy");
    MenuFrameClone<Backend> result{backend.create_like(source), {}};
    if (!result.get())
        throw std::runtime_error("Menu frame clone allocation failed");
    backend.copy(result.get(), source);
    if (parent)
        backend.link(result.get(), parent, 0);
    const auto count = backend.child_count(source);
    if (count > 1024)
        throw std::runtime_error("Invalid menu clone child count");
    result.children.reserve(count);
    for (unsigned i = 0; i < count; ++i)
        result.children.push_back(clone_menu_frame(backend, backend.child(source, i), result.get(), depth + 1));
    return result;
}
} // namespace hd2::frontend
#endif
