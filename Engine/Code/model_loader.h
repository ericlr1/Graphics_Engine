//#ifndef MODEL_LOADER_H
//#define MODEL_LOADER_H
//
//#include "platform.h"
//#include <assimp/scene.h>
//#include <vector>
//#include <cstdint>
//
//// Forward declarations
//struct App;
////struct vec3;    //TODO: Parece que vec3 no existe, rarete
//
//// Basic math types (assuming vec3 is used in the code)
//struct vec3 { float x, y, z; };
//
//// Vertex buffer attributes
//struct VertexBufferAttribute {
//    u32 location;
//    u32 componentCount;
//    u32 offset;
//};
//
//// Vertex buffer layout
//struct VertexBufferLayout {
//    std::vector<VertexBufferAttribute> attributes;
//    u32 stride;
//};
//
//// Submesh container
//struct Submesh {
//    VertexBufferLayout vertexBufferLayout;
//    std::vector<float> vertices;
//    std::vector<u32> indices;
//    u32 vertexOffset;
//    u32 indexOffset;
//};
//
//// Mesh container
//struct Mesh {
//    std::vector<Submesh> submeshes;
//    u32 vertexBufferHandle;
//    u32 indexBufferHandle;
//};
//
//// Material properties
//struct Material {
//    std::string name;
//    vec3 albedo;
//    vec3 emissive;
//    float smoothness;
//
//    // Texture indices (as OpenGL handles)
//    u32 albedoTextureIdx;
//    u32 emissiveTextureIdx;
//    u32 specularTextureIdx;
//    u32 normalsTextureIdx;
//    u32 bumpTextureIdx;
//};
//
//// Model reference
//struct Model {
//    u32 meshIdx;
//    std::vector<u32> materialIdx;  // Per-submesh material indices
//};
//
//// Function declarations
//void ProcessAssimpMesh(const aiScene* scene, aiMesh* mesh, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);
//void ProcessAssimpMaterial(App* app, aiMaterial* material, Material& myMaterial, String directory);
//void ProcessAssimpNode(const aiScene* scene, aiNode* node, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);
//u32 LoadModel(App* app, const char* filename);
//
//#endif // MODEL_LOADER_H