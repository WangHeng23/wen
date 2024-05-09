#pragma once

namespace wen {

class RayTracingScene {
public:
    enum class SceneType {
        eGLTFScene,
    };

public:
    virtual SceneType getType() const = 0;
    virtual ~RayTracingScene() = default;
};

} // namespace wen