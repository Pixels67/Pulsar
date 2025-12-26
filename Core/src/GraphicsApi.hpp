#ifndef PULSAR_GRAPHICSAPI_HPP
#define PULSAR_GRAPHICSAPI_HPP

#include <cstdint>

namespace Pulsar {
    enum class GraphicsApi : uint8_t {
        OpenGl,
        Vulkan
    };
}

#endif //PULSAR_GRAPHICSAPI_HPP