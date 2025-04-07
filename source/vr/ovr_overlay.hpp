#ifndef OVR_OVERLAY_HPP
#define OVR_OVERLAY_HPP

#include "scene/node3d.hpp"
#include "types/color.hpp"

namespace vr
{
    class Texture_t;
}
namespace nyxpiri::ovrpenguin
{

class OvrOverlay : public Node3d
{
    using Super = Node3d;
public:
    OvrOverlay(WeakPtr<class GlContext> in_gl_context);
    ~OvrOverlay();
    enum class Type: u8
    {
        null, dashboard, world
    };

    [[nodiscard]] bool set_overlay_type(Type new_type);
    Type get_overlay_type() const;

    void set_texture_data(const Color* data, usize width, usize height);

protected:
    virtual void on_start() override;
    virtual void on_tick(real delta_seconds) override;
    virtual void on_stop() override;

private:
    void destroy_ovr_overlay();

private:
    u64 handle = -1;
    u64 thumbnail_handle = -1;
    Type overlay_type = Type::null;
    DynArray<Color> texture_data;
    WeakPtr<class GlContext> gl_context = nullptr;
    u32 gl_texture_id = 0;
    vr::Texture_t* vr_texture = nullptr;

};

} // namespace nyxpiri::ovrpenguin

#endif // OVR_OVERLAY_HPP
