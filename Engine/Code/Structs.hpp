
#ifndef STRUCTS
#define STRUCTS

#include "platform.h"
#include <glad/glad.h>
#include <stdexcept>

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

struct Camera
{
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    vec3 position;
    vec3 front;
    vec3 right;
    vec3 up;

    float movementSpeed;

    bool isRotating;
    float yaw;
    float pitch;
};

// Declaración anticipada de "Buffer" si es necesario en otros headers
struct Buffer {
    GLuint handle;
    u32 size;
    GLenum type;
    u8* data;
    u64 head;
};

struct Image
{
    void* pixels;
    ivec2 size;
    i32   nchannels;
    i32   stride;
};

struct Vao {
    GLuint handle;
    GLuint programHandle;
};

struct Texture
{
    GLuint      handle;
    std::string filepath;
};

struct VertexShaderAttribute {
    u8 location;
    u8 componentCount;
};

struct VertexShaderLayout {
    std::vector<VertexShaderAttribute> attributes;
};

struct Program
{
    GLuint             handle;
    std::string        filepath;
    std::string        programName;
    u64                lastWriteTimestamp; // What is this for?
    VertexShaderLayout vertexInputLayout;
};

enum Mode
{
    Mode_TexturedQuad,
    Mode_Deferred_Geometry,
    Mode_Count
};

enum DebugMode {
    DebugMode_Combined,
    DebugMode_Albedo,
    DebugMode_Normals,
    DebugMode_Position,
    DebugMode_Depth,
    DebugMode_Count
};

// Structures
struct VertexBufferAttribute {
    u8 location;
    u8 componentCount;
    u8 offset;
};

struct Entity {
    
    glm::mat4 worldMatrix;
    u32 modelIndex;
    u32 entityBufferOffset;
    u32 entityBufferSize;
};

struct VertexBufferLayout {
    std::vector<VertexBufferAttribute> attributes;
    u8 stride;
};

struct Model {
    u32 meshIdx;
    std::vector<u32> materialIdx;
};


struct Submesh {
    VertexBufferLayout vertexBufferLayout;
    std::vector<float> vertices;
    std::vector<u32> indices;
    u32 vertexOffset;
    u32 indexOffset;

    std::vector<Vao> vaos;
};

struct Mesh {
    std::vector<Submesh> submeshes;
    GLuint vertexBufferHandle;
    GLuint indexBufferHandle;

    //std::vector<Vao> vaos;
};

struct Material {
    std::string name;
    glm::vec3 albedo;
    glm::vec3 emissive;
    f32 smoothness;
    u32 albedoTextureIdx;
    u32 emissiveTextureIdx;
    u32 specularTextureIdx;
    u32 normalsTextureIdx;
    u32 bumpTextureIdx;
};

enum class LightType {
    Light_Directional,
    Light_Point,

};

struct Light
{
    LightType type;
    vec3 color;
    vec3 direction;
    vec3 position;
    float intensity;
};

struct FrameBuffer {

    GLuint handle;
    vec2 buffersSize;
    std::vector<std::pair<GLenum, GLuint>> attachments;
    GLuint depthHandle;

    uint64_t _width;
    uint64_t _height;

    bool CreateFBO(const uint64_t aAttachments, const uint64_t aWidth, const uint64_t aHeight)
    {
        // Clean up if one already existed
        Clean();

        _width = aWidth;
        _height = aHeight;

        // 1. Create color attachments
        for (size_t i = 0; i < aAttachments; ++i)
        {
            GLuint colorAttachment;
            glGenTextures(1, &colorAttachment);
            glBindTexture(GL_TEXTURE_2D, colorAttachment);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, aWidth, aHeight, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            attachments.push_back({ GL_COLOR_ATTACHMENT0 + i, colorAttachment });
        }

        // 2. Create depth texture
        glGenTextures(1, &depthHandle);
        glBindTexture(GL_TEXTURE_2D, depthHandle);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, aWidth, aHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // 3. Create framebuffer
        glGenFramebuffers(1, &handle);
        glBindFramebuffer(GL_FRAMEBUFFER, handle);

        // Attach color textures
        std::vector<GLenum> drawBuffers;
        for (auto& attachment : attachments)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment.first, GL_TEXTURE_2D, attachment.second, 0);
            drawBuffers.push_back(attachment.first);
        }

        // Attach depth texture
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthHandle, 0);

        // Set draw buffers
        glDrawBuffers(drawBuffers.size(), drawBuffers.data());

        // Check framebuffer status
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            Clean();
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return true;
    }


    void Clean()
    {
        for (auto& texture : attachments)
        {
            glDeleteTextures(1, &texture.second);
            texture.second = 0;
        }
        attachments.clear();
        glDeleteTextures(1, &depthHandle);
        depthHandle = 0;
        glDeleteFramebuffers(1, &handle);
    }

    void Resize(uint64_t width, uint64_t height)
    {
        if (width == _width && height == _height || height == 0 || width == 0)
            return;

        Clean();
        CreateFBO(4, width, height);
    }
};

struct App
{
    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    // Graphics
    char gpuName[64];
    char openGlVersion[64];

    ivec2 displaySize;

    std::vector<Texture>  textures;
    std::vector<Material>  materials;
    std::vector<Mesh>  meshes;
    std::vector<Model>  models;
    std::vector<Program>  programs;

    // program indices
    u32 texturedGeometryProgramIdx;
    u32 geometryProgramIdx;

    u32 patrickIdx;
    u32 floatingPatrickEntityIndex;
    u32 patrickTextureUniform;


    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;
    u32 planeTexIdx;

    // Mode
    Mode mode;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    GLuint embeddedVertices;
    GLuint embeddedElements;

    // Location of the texture uniform in the textured quad shader
    GLuint programUniformTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;

    //Vector ith all the OpenGL extensions
    std::vector<std::string> glExtensions;

    Camera worldCamera;
    GLint maxUniformBufferSize;
    GLint uniformBlockAlignment;

    Buffer entityUBO;
    Buffer globalUBO;

    std::vector<Entity> entities;
    std::vector<Light> lights;

    FrameBuffer primaryFBO;

    DebugMode debugMode;
};

#endif // STRUCTS
