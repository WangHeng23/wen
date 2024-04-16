#pragma once

#include "context.hpp"

namespace wen {

void initialize();
Context& initializeRenderer();
bool shouldClose();
void pollEvents();
void destroyRenderer();
void destroy();

extern Context* manager;

} // namespace wen