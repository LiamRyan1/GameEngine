# 3D Game Engine

A custom 3D game engine built from scratch in C++ as a Final Year Project, featuring real-time rendering, physics simulation, and a complete editor toolset.

---

## Features

### Rendering System
- OpenGL-based 3D rendering with custom shader pipeline
- Phong lighting model with ambient, diffuse, and specular components
- Skybox system with cubemap texture support
- Model loading from .obj files with auto-normalization
- Texture management with caching system (PNG/JPG support)
- Procedural mesh generation for primitives (cubes, spheres, cylinders)
- Wireframe visualization for debugging
- External GLSL shaders for flexible visual development

### Physics Integration
- Bullet Physics integration for realistic simulation
- Collision detection with multiple shape types (box, sphere, cylinder, capsule)
- Physics materials system with customizable friction and restitution
- Spatial grid optimization for efficient proximity queries
- Raycasting for object selection and ground detection
- Rigid body dynamics with dynamic and static objects

### Editor & Tools
- ImGui-based debug UI with real-time controls
- Editor mode with object selection and manipulation
- Transform gizmo for translating objects in 3D space
- Inspector panel for live property editing
- Runtime model importer with dropdown selection
- Lighting controls with direction, color, and intensity sliders
- Performance statistics display (FPS, frame time)

### Architecture
- Component-based GameObject system for modularity
- Separate rendering and physics meshes for optimization
- Fixed timestep game loop for stable physics updates
- Scene management with object spawning/destruction
- Camera system with free-fly and editor modes
- Input management with keyboard and mouse support

---

## Tech Stack

| Category | Technology |
|----------|-----------|
| **Language** | C++ |
| **Graphics API** | OpenGL 4.x |
| **Physics Engine** | Bullet Physics |
| **UI Framework** | ImGui |
| **Build System** | CMake |

**Libraries:**
- GLFW (windowing)
- GLAD (OpenGL loader)
- GLM (mathematics)
- stb_image (image loading)
- tinyobjloader (model loading)

---

## Team

This project was developed as a collaborative Final Year Project:

- **Radoslaw Rodak** - Rendering Systems
- **Brian Walsh** - Framework Architecture
- **Liam Ryan** - Physics Integration

---


## Academic Context

This engine was developed as a Final Year Project for Computing in Software Engineering, demonstrating practical application of:
- Real-time graphics programming
- Physics simulation algorithms
- Software architecture patterns
- Collaborative development practices
- Modern C++ techniques

---

## License

This project is for academic purposes.

---

## Acknowledgments

- Thanks to our project supervisor Dr Aaron Hurley for guidance and feedback
- External libraries used are credited to their respective authors

---
