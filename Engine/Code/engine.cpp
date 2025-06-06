//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include "Structs.hpp"
#include "assimp_model_loading.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include "buffer_managment.h"

glm::mat4 TransformScale(const vec3& scaleFactor)
{
    return glm::scale(scaleFactor);
}

glm::mat4 TransformPositionScale(const vec3& pos, const vec3& scaleFactor)
{
    glm::mat4 transform = glm::translate(pos);
    transform = glm::scale(transform, scaleFactor);
    return transform;
}

GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(vertexShaderDefine),
        (GLint) programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(fragmentShaderDefine),
        (GLint) programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    Program program = {};
    program.handle = CreateProgramFromSource(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);

    if (program.handle != 0)
    {
        GLint attributeCount = 0UL;
        glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);
        for (size_t i = 0; i < attributeCount; ++i)
        {
            GLchar Name[248];
            GLsizei realNameSize = 0UL;
            GLsizei attribSize = 0UL;
            GLenum attribType;
            glGetActiveAttrib(program.handle, i, ARRAY_COUNT(Name), &realNameSize, &attribSize, &attribType, Name);
            GLuint attribLocation = glGetAttribLocation(program.handle, Name);

            program.vertexInputLayout.attributes.push_back({static_cast<u8>(attribLocation), static_cast<u8>(attribSize) });
        }
    }


    app->programs.push_back(program);

    return app->programs.size() - 1;
}

Image LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename);
    }
    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

GLuint CreateTexture2DFromImage(Image image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat     = GL_RGB;
    GLenum dataType       = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
        case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
        case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
        default: ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
        tex.filepath = filepath;

        u32 texIdx = app->textures.size();
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
    {
        return UINT32_MAX;
    }
}

void UpdateLights(App* app)
{
    MapBuffer(app->globalUBO, GL_WRITE_ONLY);
    PushVec3(app->globalUBO, app->worldCamera.position);

    PushUInt(app->globalUBO, app->lights.size());

    for (size_t i = 0; i < app->lights.size(); ++i)
    {
        AlignHead(app->globalUBO, sizeof(vec4));

        Light& light = app->lights[i];
        PushUInt(app->globalUBO, static_cast<unsigned int>(light.type));
        PushVec3(app->globalUBO, light.color * light.intensity);
        PushVec3(app->globalUBO, light.direction);
        PushVec3(app->globalUBO, light.position);
    }

    UnmapBuffer(app->globalUBO);
}

void RenderScreenFillQuad(App* app, const FrameBuffer& aFBO)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Program& quadProgram = app->programs[app->texturedGeometryProgramIdx];
    glUseProgram(quadProgram.handle);
    glBindVertexArray(app->vao);

    // Attach all the textures to the G-Buffer
    const char* uniformNames[] = { "uAlbedo", "uNormals", "uPosition", "uViewDir", "uDepth" };
    for (size_t i = 0; i < aFBO.attachments.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, aFBO.attachments[i].second);
        glUniform1i(glGetUniformLocation(quadProgram.handle, uniformNames[i]), i);
    }

    // Attach depth texture
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, aFBO.depthHandle);
    glUniform1i(glGetUniformLocation(quadProgram.handle, "uDepth"), 4);

    // Use debug mode
    GLuint debugModeLoc = glGetUniformLocation(quadProgram.handle, "uDebugMode");
    glUniform1i(debugModeLoc, static_cast<int>(app->debugMode));

    // Draw quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}

void Init(App* app)
{

    //Get OpenGL extensions
    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    app->glExtensions.reserve(numExtensions);
    for (GLint i = 0; i < numExtensions; ++i)
    {
        const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
        app->glExtensions.emplace_back(extension);
    }

    struct VertexV3V2
    {
        glm::vec3 pos;
        glm::vec2 uv;
    };

    const VertexV3V2 vertices[] = {
        { glm::vec3(-1.0, -1.0, 0.0), glm::vec2(0.0, 0.0)}, //Bottom-left
        { glm::vec3(1.0, -1.0, 0.0), glm::vec2(1.0, 0.0)}, //Bottom-right
        { glm::vec3(1.0, 1.0, 0.0), glm::vec2(1.0, 1.0)}, //Top-right
        { glm::vec3(-1.0, 1.0, 0.0), glm::vec2(0.0, 1.0)}, //Top-left
    };

    const u16 indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    //Geometry
    
    glEnable(GL_DEPTH_TEST);
    
    //VBO Init
    glGenBuffers(1, &app->embeddedVertices);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //EBO Init
    glGenBuffers(1, &app->embeddedElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //VAO Init
    // Attribute state
    glGenVertexArrays(1, &app->vao); 
    glBindVertexArray(app->vao);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)0); 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)12); 
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBindVertexArray(0);

    //Program Init
    app->patrickIdx = LoadModel(app, "./Patrick/Patrick.obj");
    u32 planeIdx = LoadModel(app, "./Patrick/plane.obj");

    app->texturedGeometryProgramIdx = LoadProgram(app, "RENDER_QUAD.glsl", "RENDER_QUAD");
    app->geometryProgramIdx = LoadProgram(app, "RENDER_GEOMETRY.glsl", "RENDER_GEOMETRY");

    Program& quadProgram = app->programs[app->texturedGeometryProgramIdx];
    //quadProgram.vertexInputLayout.attributes.push_back({ 0, 3 });
    //quadProgram.vertexInputLayout.attributes.push_back({ 1, 2 });

    app->programUniformTexture = glGetUniformLocation(app->programs[app->texturedGeometryProgramIdx].handle, "uTexture");

    ////Texture Init
    app->diceTexIdx = LoadTexture2D(app, "dice.png");
    app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
    app->blackTexIdx = LoadTexture2D(app, "color_black.png");
    app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
    app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");

    app->materials[app->models[planeIdx].materialIdx[0]].albedoTextureIdx = app->whiteTexIdx;

    app->patrickTextureUniform = glGetUniformLocation(app->programs[app->geometryProgramIdx].handle, "uTexture");

    //WIP - Camera setup
    {
        float aspectRatio = (float)app->displaySize.x / (float)app->displaySize.y;
        float znear = 0.1f;
        float zfar = 1000.0f;
        app->worldCamera.projectionMatrix = glm::perspective(glm::radians(60.0f), aspectRatio, znear, zfar);
        app->worldCamera.position = vec3(-2, 8, 25);
        app->worldCamera.viewMatrix = glm::lookAt(app->worldCamera.position, vec3(0, 2, 0), vec3(0, 1, 0));

        // Extract initial front, right, and up from the view matrix
        glm::mat4 view = app->worldCamera.viewMatrix;
        app->worldCamera.right = glm::vec3(view[0][0], view[1][0], view[2][0]);
        app->worldCamera.up = glm::vec3(view[0][1], view[1][1], view[2][1]);
        app->worldCamera.front = -glm::vec3(view[0][2], view[1][2], view[2][2]);

        // Calculate initial yaw and pitch
        vec3 front = app->worldCamera.front;
        app->worldCamera.yaw = glm::degrees(atan2(front.z, front.x));
        app->worldCamera.pitch = glm::degrees(asin(front.y));

        // Initialize other camera parameters
        app->worldCamera.isRotating = false;
        app->worldCamera.movementSpeed = 5.0f;
    }

    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &app->maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBlockAlignment);

    app->globalUBO = CreateConstantBuffer(app->maxUniformBufferSize);
    app->entityUBO = CreateConstantBuffer(app->maxUniformBufferSize);

    
    //Lueces - entre otras cosas
    app->lights.push_back({ LightType::Light_Directional, vec3(0.75), vec3(1.0, -1.0, -0.5), vec3(0.0), 1 });
    for (size_t i = 0; i < 20; ++i)
    {
        for (size_t j = 0; j < 20; ++j)
        {
            float lIntensity = 0.1f;
            app->lights.push_back({ LightType::Light_Point, vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(i * 5 -40, 2.0, j * 5), lIntensity });
            app->lights.push_back({ LightType::Light_Point, vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(i * 5, 2.0, j * 5), lIntensity });
        }
    }

    UpdateLights(app);


    //Entidades
    MapBuffer(app->entityUBO, GL_WRITE_ONLY);

    glm::mat4 VP = app->worldCamera.projectionMatrix * app->worldCamera.viewMatrix;
    // Create plane entity
    {
        Entity entity;
        AlignHead(app->entityUBO, app->uniformBlockAlignment);
        entity.entityBufferOffset = app->entityUBO.head;

        entity.worldMatrix = glm::translate(glm::vec3(0, -3.5, 0));
        entity.modelIndex = planeIdx;

        PushMat4(app->entityUBO, entity.worldMatrix);
        PushMat4(app->entityUBO, VP * entity.worldMatrix);

        entity.entityBufferSize = app->entityUBO.head - entity.entityBufferOffset;
        app->entities.push_back(entity);
    }

    for (int z = -2; z != 2; z++)
    {
        for (int x = -2; x != 2; x++)
        {
            Entity entity;
            AlignHead(app->entityUBO, app->uniformBlockAlignment);
            entity.entityBufferOffset = app->entityUBO.head;

            entity.worldMatrix = glm::translate(glm::vec3(x * 4, 0, z * 4));
            entity.modelIndex = app->patrickIdx;

            PushMat4(app->entityUBO, entity.worldMatrix);
            PushMat4(app->entityUBO, VP * entity.worldMatrix);

            entity.entityBufferSize = app->entityUBO.head - entity.entityBufferOffset;
            app->entities.push_back(entity);
        }
    }

    Entity floatingPatrick;
    AlignHead(app->entityUBO, app->uniformBlockAlignment);
    floatingPatrick.entityBufferOffset = app->entityUBO.head;
    floatingPatrick.modelIndex = app->patrickIdx;
    floatingPatrick.worldMatrix = glm::mat4(1.0); // Initial position matrix
    
    PushMat4(app->entityUBO, floatingPatrick.worldMatrix);
    PushMat4(app->entityUBO, VP * floatingPatrick.worldMatrix);

    floatingPatrick.entityBufferSize = app->entityUBO.head - floatingPatrick.entityBufferOffset;
    app->entities.push_back(floatingPatrick);
    app->floatingPatrickEntityIndex = app->entities.size() - 1;

    UnmapBuffer(app->entityUBO);

    app->debugMode = DebugMode_Combined;
    app->mode = Mode_Deferred_Geometry;

    app->primaryFBO.CreateFBO(4, app->displaySize.x, app->displaySize.y);

}

void Gui(App* app)
{
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);
    ImGui::End();

    ImGui::Begin("Settings");
    ImGui::Text("===== View Mode =====");
    const char* debugModes[] = { "Combined", "Albedo", "Normals", "Position", "Depth" };
    int currentMode = static_cast<int>(app->debugMode);
    if (ImGui::Combo("Render Mode", &currentMode, debugModes, DebugMode_Count)) {
        app->debugMode = static_cast<DebugMode>(currentMode);
    }
    ImGui::End();

    ImGui::Begin("OpenGL Info");
    // Display basic OpenGL information
    ImGui::Text("OpenGL version: %s", (const char*)glGetString(GL_VERSION));
    ImGui::Text("OpenGL renderer: %s", (const char*)glGetString(GL_RENDERER));
    ImGui::Text("OpenGL vendor: %s", (const char*)glGetString(GL_VENDOR));
    ImGui::Text("OpenGL GLSL version: %s", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    ImGui::Separator();
    bool lightChanged = false;
    ImGui::Text("===== Lights Setup =====");
    ImGui::Text("Light Count: %d", app->lights.size());
    for (auto& light : app->lights)
    {
        vec3 checkVector;

        ImGui::PushID(&light);
        float color[3] = { light.color.x, light.color.y, light.color.z };
        ImGui::Text("ID: %d", (int) &light);
        ImGui::DragFloat3("Color", color, 0.01, 0.0, 1.0);
        checkVector = vec3(color[0], color[1], color[2]);
        if (checkVector != light.color)
        {
            light.color = checkVector;
            lightChanged = true;
        }

        float dir[3] = { light.direction.x, light.direction.y, light.direction.z };
        ImGui::DragFloat3("Direction", dir, 0.01, -1.0, 1.0);
        checkVector = vec3(dir[0], dir[1], dir[2]);
        if (checkVector != light.direction)
        {
            light.direction = checkVector;
            lightChanged = true;
        }

        float position[3] = { light.position.x, light.position.y, light.position.z };
        ImGui::DragFloat3("Position", position);
        checkVector = vec3(position[0], position[1], position[2]);
        if (checkVector != light.position)
        {
            light.position = checkVector;
            lightChanged = true;
        }

        ImGui::PopID();
        ImGui::Separator();
    }

    if (lightChanged)
    {
        UpdateLights(app);
    }

    ImGui::Separator();
    ImGui::Text("OpenGL Extensions:");
    ImGui::BeginChild("Extensions", ImVec2(0, 100), false, ImGuiWindowFlags_HorizontalScrollbar);

    // Use cached list
    for (const auto& ext : app->glExtensions)
    {
        ImGui::Text("%s", ext.c_str());
    }

    ImGui::EndChild();
    ImGui::End();
}

void Update(App* app) {

    // Update the shaders in rundtime - Hot reload
    {
        for (auto& program : app->programs) {
            u64 currentTimestamp = GetFileLastWriteTimestamp(program.filepath.c_str());
            if (currentTimestamp > program.lastWriteTimestamp) {
                // Recompile the shader
                GLuint newHandle = CreateProgramFromSource(ReadTextFile(program.filepath.c_str()), program.programName.c_str());
                if (newHandle != 0) {
                    // Cleanup old shader
                    glDeleteProgram(program.handle);
                    program.handle = newHandle;
                    program.lastWriteTimestamp = currentTimestamp;
                    ILOG("Reloaded shader: %s", program.programName.c_str());
                }
            }
        }
    }

    // Agregar 200 nuevas luces en grid con la tecla L (sin borrar existentes)
    if (app->input.keys[K_L] == BUTTON_PRESS)
    {
        const int gridSize = 20;
        const float spacing = 5.0f;
        const float offset = (gridSize * spacing) * 0.5f;

        for (int x = 0; x < gridSize; ++x)
        {
            for (int z = 0; z < gridSize; ++z)
            {
                // Solo crear 200 luces (20x10)
                if (x * gridSize + z >= 200) break;

                vec3 position = vec3(x * spacing - offset, 2.0f, z * spacing - offset);
                app->lights.push_back({ LightType::Light_Point, vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 0.0), position, 1.0 });

            }
        }
        UpdateLights(app); // Actualizar UBO de luces
    }

    // Mouse rotation
    if (app->input.mouseButtons[RIGHT] == BUTTON_PRESS) {
        app->worldCamera.isRotating = true;
    }
    if (app->input.mouseButtons[RIGHT] == BUTTON_RELEASE) {
        app->worldCamera.isRotating = false;
    }

    if (app->worldCamera.isRotating) {
        ProcessMouseMovement(&app->worldCamera,
            app->input.mouseDelta.x,
            app->input.mouseDelta.y);
    }

    // Increment velocity with shift
    float baseSpeed = app->worldCamera.movementSpeed;
    float speedMultiplier = 1.0f;

    if (app->input.keys[K_LSHIFT] == BUTTON_PRESSED) {
        speedMultiplier = 2.0f; // Boost speed by x2
    }

    float velocity = baseSpeed * speedMultiplier * app->deltaTime;

    // Key bindings
    if (app->input.keys[K_W] == BUTTON_PRESSED)
    {
        app->worldCamera.position += app->worldCamera.front * velocity;
    }
    if (app->input.keys[K_S] == BUTTON_PRESSED)
    {
        app->worldCamera.position -= app->worldCamera.front * velocity;
    }
    if (app->input.keys[K_A] == BUTTON_PRESSED)
    {
        app->worldCamera.position -= app->worldCamera.right * velocity;
    }
    if (app->input.keys[K_D] == BUTTON_PRESSED)
    {
        app->worldCamera.position += app->worldCamera.right * velocity;
    }
    if (app->input.keys[K_Q] == BUTTON_PRESSED)
    {
        app->worldCamera.position -= app->worldCamera.up * velocity;
    }
    if (app->input.keys[K_E] == BUTTON_PRESSED)
    {
        app->worldCamera.position += app->worldCamera.up * velocity;
    }

    // Update matrices
    app->worldCamera.viewMatrix = glm::lookAt(
        app->worldCamera.position,
        app->worldCamera.position + app->worldCamera.front,
        app->worldCamera.up
    );

    // Update UBOs
    glm::mat4 VP = app->worldCamera.projectionMatrix * app->worldCamera.viewMatrix;

    // Update entities
    MapBuffer(app->entityUBO, GL_WRITE_ONLY);
    for (auto& entity : app->entities) {
        size_t matrixOffset = entity.entityBufferOffset + sizeof(glm::mat4);
        glm::mat4 newVPMatrix = VP * entity.worldMatrix;
        memcpy((char*)app->entityUBO.data + matrixOffset, &newVPMatrix, sizeof(glm::mat4));
    }
    UnmapBuffer(app->entityUBO);

    // Update floating patrick
    MapBuffer(app->entityUBO, GL_WRITE_ONLY);
    for (size_t i = 0; i < app->entities.size(); ++i) {
        auto& entity = app->entities[i];
        if (i == app->floatingPatrickEntityIndex) {
            static float time = 0.0f;
            time += app->deltaTime;

            // Floating animation
            float yPos = sin(time) * 2.0f; // Adjust height and speed
            // Spinning animation
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), time, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, yPos, 5.0f));
            entity.worldMatrix = translation * rotation;

            // Update both matrices in UBO
            glm::mat4 newVPMatrix = VP * entity.worldMatrix;
            memcpy((char*)app->entityUBO.data + entity.entityBufferOffset, &entity.worldMatrix, sizeof(glm::mat4));
            memcpy((char*)app->entityUBO.data + entity.entityBufferOffset + sizeof(glm::mat4), &newVPMatrix, sizeof(glm::mat4));
        }
        else {
            // Existing VP update
            size_t matrixOffset = entity.entityBufferOffset + sizeof(glm::mat4);
            glm::mat4 newVPMatrix = VP * entity.worldMatrix;
            memcpy((char*)app->entityUBO.data + matrixOffset, &newVPMatrix, sizeof(glm::mat4));
        }
    }
    UnmapBuffer(app->entityUBO);
}

void Render(App* app)
{
    switch (app->mode)
    {
        case Mode_TexturedQuad:
        {
            //// Clear the framebuffer
            //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //// Set the viewport
            //glViewport(0, 0, app->displaySize.x, app->displaySize.y);

            ////Bind the program
            //Program& programTexturedGeometry = app->programs[app->texturedGeometryProgramIdx]; 
            //glUseProgram(programTexturedGeometry.handle); 
            ////Bind the VAO
            //glBindVertexArray(app->vao);

            ////Set the blending state
            //glEnable(GL_BLEND);
            //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            ////Bind the texture into unit 0 (and make its texture sample from unit 0)
            //glUniform1i(app->programUniformTexture, 0); 
            //glActiveTexture(GL_TEXTURE0);
            //GLuint textureHandle = app->textures[app->diceTexIdx].handle;
            //glBindTexture(GL_TEXTURE_2D, textureHandle);

            ////glDrawElements() -> De momento hardcoded a 6
            //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

            //glBindVertexArray(0); 
            //glUseProgram(0);
                
        }
        break;
        case Mode_Deferred_Geometry:
        {
            // Clear the framebuffer
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glBindFramebuffer(GL_FRAMEBUFFER, app->primaryFBO.handle);

            std::vector<GLuint> textures;
            for (auto& it : app->primaryFBO.attachments)
            {
                textures.push_back(it.second);
            }
            glDrawBuffers(textures.size(), textures.data());

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Set the viewport
            glViewport(0, 0, app->displaySize.x, app->displaySize.y);

            Program& geometryProgram = app->programs[app->geometryProgramIdx];
            glUseProgram(geometryProgram.handle);

            glBindBufferRange(GL_UNIFORM_BUFFER, 0, app->globalUBO.handle, 0, app->globalUBO.size);

            for (const auto& entity : app->entities)
            {
                glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->entityUBO.handle, entity.entityBufferOffset, entity.entityBufferSize);
                Model& model = app->models[entity.modelIndex];
                Mesh& mesh = app->meshes[model.meshIdx];

                for (u32 i = 0; i < mesh.submeshes.size(); ++i)
                {
                    GLuint vao = FindVAO(mesh, i, geometryProgram);

                    glBindVertexArray(vao);

                    u32 submeshMaterialIdx = model.materialIdx[i];
                    Material& submeshMaterial = app->materials[submeshMaterialIdx];

                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
                    glUniform1i(app->patrickTextureUniform, 0);

                    Submesh& submesh = mesh.submeshes[i];
                    glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);

                    glBindTexture(GL_TEXTURE_2D, 0);
                }
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            RenderScreenFillQuad(app, app->primaryFBO);
        }
        break;

        default:;
    }
}

void CleanUp(App* app)
{
    ELOG("Cleaning Up Engine");

    // Delete textures
    for (auto& texture : app->textures)
    {
        glDeleteTextures(1, &texture.handle);
    }
    app->textures.clear();

    // Delete shader programs
    for (auto& program : app->programs)
    {
        glDeleteProgram(program.handle);
    }
    app->programs.clear();

    // Delete VAO
    if (app->vao != 0)
    {
        glDeleteVertexArrays(1, &app->vao);
        app->vao = 0;
    }

    // Delete buffers
    if (app->embeddedElements != 0)
    {
        glDeleteBuffers(1, &app->embeddedElements);
        app->embeddedElements = 0;
    }

    if (app->embeddedVertices != 0)
    {
        glDeleteBuffers(1, &app->embeddedVertices);
        app->embeddedVertices = 0;
    }

    app->primaryFBO.Clean();
}

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];

    for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i)
    {
        if (submesh.vaos[i].programHandle == program.handle)
        {
            return submesh.vaos[i].handle;
        }
    }

    GLuint vaoHandle = 0;

    //Create new VAO for this submesh/program - TODO: This can be a function
    {
        glGenVertexArrays(1, &vaoHandle);
        glBindVertexArray(vaoHandle);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

        for (u32 i = 0; i < program.vertexInputLayout.attributes.size(); ++i)
        {
            bool attributeWasLinked = false;

            for (u32 j = 0; j < submesh.vertexBufferLayout.attributes.size(); ++j)
            {
                if (program.vertexInputLayout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
                {
                    const u32 index = submesh.vertexBufferLayout.attributes[j].location;
                    const u32 ncomp = submesh.vertexBufferLayout.attributes[j].componentCount;
                    const u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset;
                    const u32 stride = submesh.vertexBufferLayout.stride;

                    glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                    glEnableVertexAttribArray(index);

                    attributeWasLinked = true;
                    break;
                }
            }
            assert(attributeWasLinked);
        }
        glBindVertexArray(0);
    }

    Vao vao = { vaoHandle, program.handle };
    submesh.vaos.push_back(vao);
    
    return vaoHandle;
}

void ProcessMouseMovement(Camera* camera, float xOffset, float yOffset)
{
    const float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    camera->yaw += xOffset;
    camera->pitch -= yOffset;

    // Constrain pitch to avoid flipping
    camera->pitch = glm::clamp(camera->pitch, -89.0f, 89.0f);

    // Calculate new front vector
    glm::vec3 front;
    front.x = cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
    front.y = sin(glm::radians(camera->pitch));
    front.z = sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
    camera->front = glm::normalize(front);

    // Recalculate right and up vectors
    camera->right = glm::normalize(glm::cross(camera->front, glm::vec3(0.0f, 1.0f, 0.0f)));
    camera->up = glm::normalize(glm::cross(camera->right, camera->front));
}
