//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include "Structs.hpp"
#include <glad/glad.h>

void UpdateLights(App* app);

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

void CleanUp(App* app);

u32 LoadTexture2D(App* app, const char* filepath);

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);

void ProcessMouseMovement(Camera* camera, float xOffset, float yOffset);

void CreateLight(App* app, LightType type, const glm::vec3& color, const glm::vec3& direction, const glm::vec3& position, float intensity);