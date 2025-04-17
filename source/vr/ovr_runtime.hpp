#ifndef OVR_RUNTIME_HPP
#define OVR_RUNTIME_HPP

#include "io/stdio_handler.hpp"
#include "scene/node.hpp"
#include "graphics/gl_context.hpp"
#include "math/mat4x4.hpp"

namespace vr
{
class IVRSystem;
}

namespace nyxpiri::ovrpenguin
{
class OvrRuntime : public Node
{
public:
    OvrRuntime(WeakPtr<StdIoHandler> in_io_handler);
    ~OvrRuntime();

    virtual void on_start() override;
    virtual void on_tick(real delta_seconds) override;
    virtual void on_stop() override;

    void initialize();

    bool is_running();

private:
    using Super = Node;

private:
    struct OvrData;
    vr::IVRSystem* vr_system = nullptr;
    WeakPtr<StdIoHandler> io_handler;
    OvrData* ovr_data;
};

} // namespace nyxpiri::ovrpenguin

#endif // OVR_RUNTIME_HPP
