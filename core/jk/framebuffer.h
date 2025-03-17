#pragma once
#include "../ew/external/glad.h"

namespace jk {
    struct Framebuffer {
        GLuint fbo, color0, color1, color2, depth;
    };

    Framebuffer createFramebuffer(unsigned int width, unsigned int height, int colorFormat);
    Framebuffer createGTAFramebuffer(unsigned int width, unsigned int height, int colorFormat);
}