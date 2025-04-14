#ifndef PRINT_UTILS_H
#define PRINT_UTILS_H
#include <string>
#include <glib.h>

namespace nyxpiri::ovrpenguin
{
static inline std::string format_gerror_string(const std::string& generic_msg, const GError& gerror)
{
    return generic_msg + "\n Here's what I was told \"" + gerror.message + "\"";
}
} // nyxpiri::ovrpenguin
#endif // PRINT_UTILS_H
