# Quarryfied

Quarryfied is a conceptual voxel engine designed to explore the limits of modern OpenGL (4.6+). The goal is to move beyond the traditional bottlenecks of voxel rendering by leveraging a GPU-driven pipeline and advanced culling techniques.

> Note: This project is currently in the architectural planning phase. The features listed below represent the implementation roadmap, not the current state of the codebase.

---

## Technical Vision

The engine is built around the philosophy of "GPU-first" rendering. Instead of the CPU managing individual draw calls and object visibility, Quarryfied offloads the heavy lifting to the hardware.

### Planned Optimizations
* **Bindless Multi-Draw Indirect (MDI):** Eliminating CPU overhead by batching all draw operations into a single call.
* **Mesh Shaders:** Real-time geometry synthesis and culling to discard faces before they hit the rasterizer.
* **Octree-based LOD:** A hierarchical level-of-detail system to support massive render distances with minimal memory footprint.
    * **LOD 0:** 1x1x1 (Native)
    * **LOD 1:** 2x2x2
    * **LOD 2:** 4x4x4
* **Variable Rate Shading (VRS):** Optimizing fragment shader execution based on visual importance.

---

## Planned Performance Presets

The rendering pipeline is designed to be highly modular, allowing the engine to scale across various hardware generations.

| Feature | Ultra | High | Medium | Low |
| :--- | :--- | :--- | :--- | :--- |
| **Shininess** | Cubemap | Blinn-Phong/Pixel | Blinn-Phong/Vertex | Off |
| **Shadows** | CSM Soft | CSM Hard | Shadow Map | Voxel Lighting |
| **Ambient Occlusion** | SSAO | SSAO | Vertex-Baked | Vertex-Baked |
| **Parallax Occlusion** | Full | Full | Simple | Off |
| **God Rays** | Volumetric Scatter | Screen-Space Blur | Static Fog | Off |
| **Upscaling** | FSR 3 | TAA + FSR 2 | FSR 1 | Native |

---

## Architecture

Quarryfied utilizes a lean, modern C++ stack focused on low-level control.

| Component | Technology |
| :--- | :--- |
| **Rendering API** | OpenGL 4.6 |
| **Bindings** | glbinding (Type-safe) |
| **Windowing** | SDL3 |

---

## Build Instructions

The project uses a streamlined CMake configuration designed for portability.

1. **Configure:**
   ```bash
   cmake -B build
   ```
2. **Build:**
   ```bash
   cmake --build build
   ```

---

## License
- MIT
