#ifndef OVR_OVERLAY_HPP
#define OVR_OVERLAY_HPP

#include "scene/node.hpp"
#include "types/color.hpp"
#include "math/vec3.hpp"

#include <string>

namespace vr
{
    class Texture_t;
}
namespace nyxpiri::ovrpenguin
{
enum class OverlayParent : u8
{
    PlaySpace,
    LeftMotionController, RightMotionController,
    HeadMountedDisplay
};

class OvrOverlay : public Node
{
    using Super = Node;
public:
    OvrOverlay(WeakPtr<class GlContext> in_gl_context);
    ~OvrOverlay();
    enum class Type: u8
    {
        null, dashboard, world
    };

    [[nodiscard]] bool set_overlay_type(Type new_type);
    [[nodiscard]] Type get_overlay_type() const;
    [[nodiscard]] bool is_null() const;

    void set_texture_data(DynArray<Color>&& data, usize width, usize height);

    [[nodiscard]] const std::string& get_overlay_name() const { return overlay_name; }
    [[nodiscard]] bool set_overlay_name(const std::string& in_name); 

    void set_overlay_parent(OverlayParent new_parent);
    void set_overlay_position(const Vec3& new_pos);
    void set_overlay_rotation(const Vec3& new_rot);

    void set_size(f64 new_size);
    void set_curve(f64 new_curve);

    void refresh_overlay_properties();

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
    std::string overlay_name = "ovr-penguin-overlay!";
    OverlayParent overlay_parent = OverlayParent::PlaySpace;

    Vec3 position;
    Vec3 rotation;

    f64 size = 1.0;
    f64 curve = 0.0;
};

} // namespace nyxpiri::ovrpenguin

#endif // OVR_OVERLAY_HPP
