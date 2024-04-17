#include "resources/shader_program.hpp"

namespace wen {

void GraphicsShaderProgram::setVertexShader(std::shared_ptr<Shader> shader, const std::string& entry) {
    vertexShader_.shader = std::dynamic_pointer_cast<Shader>(shader);
    vertexShader_.entry = entry;
}

void GraphicsShaderProgram::setFragmentShader(std::shared_ptr<Shader> shader, const std::string& entry) {
    fragmentShader_.shader = std::dynamic_pointer_cast<Shader>(shader);
    fragmentShader_.entry = entry;
}

GraphicsShaderProgram::~GraphicsShaderProgram() {
    vertexShader_.shader.reset();
    fragmentShader_.shader.reset();
}

} // namespace wen