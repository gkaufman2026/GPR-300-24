#include "framebuffer.h"

namespace jk {
    Framebuffer createFramebuffer(unsigned int width, unsigned int height, int colorFormat) {
        Framebuffer framebuffer;

        glGenFramebuffers(1, &framebuffer.fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

        // Color attachment
        glGenTextures(1, &framebuffer.color0);
        glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.color0, 0);

        glGenTextures(1, &framebuffer.depth);
        glBindTexture(GL_TEXTURE_2D, framebuffer.depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, framebuffer.depth, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return framebuffer;
    }
}

