#pragma once
#include <GL/glew.h>

class ShadowMap {
private:
    unsigned int depthMapFBO;      // Framebuffer for shadow pass
    unsigned int depthMapTexture;  // Depth texture
    unsigned int shadowWidth;      // Shadow map resolution
    unsigned int shadowHeight;

public:
    ShadowMap(unsigned int width = 2048, unsigned int height = 2048);
    ~ShadowMap();

    void initialize();
    void cleanup();

    // Bind for writing depth (shadow pass)
    void bindForWriting();

    // Bind depth texture for reading (main pass)
    void bindForReading(unsigned int textureUnit);

    // Unbind and return to default framebuffer
    void unbind();

    unsigned int getWidth() const { return shadowWidth; }
    unsigned int getHeight() const { return shadowHeight; }
    unsigned int getDepthTexture() const { return depthMapTexture; }
};