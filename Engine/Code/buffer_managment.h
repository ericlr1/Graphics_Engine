#ifndef C_BUFFER_MANAGMENT
#define C_BUFFER_MANAGMENT

#include "Structs.hpp"
#include <glad/glad.h>

#ifdef NDEBUG
#define ASSERT(condition, message) ((void)0)
#else
#define ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                fprintf(stderr, "[ASSERT] %s\nArchivo: %s\nLínea: %d\n", \
                        message, __FILE__, __LINE__); \
                std::abort(); \
            } \
        } while(0)
#endif


// Declaraciones de funciones
bool IsPowerOf2(u32 value);
u32 Align(u32 value, u32 alignment);
Buffer CreateBuffer(u32 size, GLenum type, GLenum usage);
void BindBuffer(const Buffer& buffer);
void MapBuffer(Buffer& buffer, GLenum access);
void UnmapBuffer(Buffer& buffer);
void AlignHead(Buffer& buffer, u32 alignment);
void PushAlignedData(Buffer& buffer, const void* data, u32 size, u32 alignment);

// Macros
#define CreateConstantBuffer(size) CreateBuffer(size, GL_UNIFORM_BUFFER, GL_STREAM_DRAW)
#define CreateStaticVertexBuffer(size) CreateBuffer(size, GL_ARRAY_BUFFER, GL_STATIC_DRAW)
#define CreateStaticIndexBuffer(size) CreateBuffer(size, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW)

#define PushData(buffer, data, size) PushAlignedData(buffer, data, size, 1)
#define PushUInt(buffer, value) do { u32 v = value; PushAlignedData(buffer, &v, sizeof(v), 4); } while(0)
#define PushVec3(buffer, value) PushAlignedData(buffer, glm::value_ptr(value), sizeof(glm::vec3), sizeof(glm::vec4))
#define PushVec4(buffer, value) PushAlignedData(buffer, glm::value_ptr(value), sizeof(glm::vec4), sizeof(glm::vec4))
#define PushMat3(buffer, value) PushAlignedData(buffer, glm::value_ptr(value), sizeof(glm::mat3), sizeof(glm::vec4))
#define PushMat4(buffer, value) PushAlignedData(buffer, glm::value_ptr(value), sizeof(glm::mat4), sizeof(glm::vec4))

#endif //C_BUFFER_MANAGMENT