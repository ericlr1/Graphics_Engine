# Motor Gráfico con Deferred Rendering

Motor gráfico implementando técnicas modernas de renderizado, desarrollado en C++ con OpenGL y Assimp. Incluye deferred shading, gestión de materiales, iluminación dinámica y una interfaz de depuración.

## 🚀 Características Principales
- **Carga de modelos 3D** (formatos soportados por Assimp: .obj, .fbx, etc.)
- **Sistema de materiales** con mapas de albedo, normales y especulares
- **Deferred Rendering** con Geometry Buffer (G-Buffer)
- **Iluminación dinámica** con múltiples luces puntuales
- **Sistema de cámara** libre con movimiento suave
- **Modos de visualización** para depuración (albedo, normales, profundidad, etc.)
- **Interfaz de usuario integrada** (ImGui) para ajustes en tiempo real

## 🎮 Controles
### Movimiento de Cámara
- **WASD**: Movimiento frontal/lateral
- **Q/E**: Ascender/Descender
- **Botón derecho del ratón + arrastrar**: Rotar vista
- **Shift**: Aumentar velocidad de movimiento

### Modos de Visualización
 Para cambiar entre los diferentes modos de visualización existe un menu deplegable en la GUI de la derecha, hay diferentes modos de visualización:
 - Combined
 - Albedo
 - Normals
 - Position
 - Depth

### Interfaz de Usuario
- Ajustes en tiempo real para:
  - Intensidad y color de luces
  - Posición de objetos
  - Modo de visualización activo

## 📦 Dependencias
- **OpenGL 4.3+**
- GLFW 3.3
- Glad (Loader OpenGL)
- Assimp (Carga de modelos)
- GLM (Matemáticas)
- stb_image (Carga de texturas)
- Dear ImGui (Interfaz de usuario)

## 📸 Imágenes
