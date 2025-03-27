
#ifndef STRUCTS
#define STRUCTS

#include "platform.h"
#include <glad/glad.h>

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
};

// Declaración anticipada de "Buffer" si es necesario en otros headers
struct Buffer {
    GLuint handle;
    u32 size;
    GLenum type;
    u8* data;
    u64 head;
};

struct FrameBuffer {
    GLuint handle;
    vec2 buffersSize;
    std::vector<std::pair<GLenum, GLuint>> attachments;
    GLuint depthHandle;
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
    Mode_Forward_Geometry,
    Mode_Count
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
    u32 patrickTextureUniform;

    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

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
};

#endif // STRUCTS
