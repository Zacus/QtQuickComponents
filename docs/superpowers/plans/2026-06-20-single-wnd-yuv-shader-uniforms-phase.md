# SingleWnd YUV Shader Uniforms Phase

**Goal:** Add a focused uniform-buffer helper for YUV shader parameters before building the graphics pipeline.

**Scope:**
- Add `Yuv420ShaderUniforms`.
- Store a BT.601 limited-range YUV-to-RGB conversion matrix.
- Store opacity in a 16-byte aligned vector.
- Allocate one dynamic `QRhiBuffer` with `UniformBuffer` usage.
- Queue uniform data updates through `QRhiResourceUpdateBatch::updateDynamicBuffer()`.
- Track serial and opacity to skip duplicate uploads.
- Add thin `Yuv420RenderNode` wrappers for upload and SceneGraph resource release.

**Ownership / Lifetime:**
- `Yuv420ShaderUniforms` owns its `QRhiBuffer` via `std::unique_ptr` with an out-of-line deleter.
- `QRhi*` and `QRhiResourceUpdateBatch*` are non-owning, call-scoped inputs.
- `Yuv420RenderNode::releaseResources()` releases uniform resources together with texture resources.

**Design Boundary:**
- This helper does not know about Y/U/V textures, samplers, SRB, pipeline, or draw commands.
- The later pipeline phase can combine this uniform buffer with texture bindings without expanding texture ownership responsibilities.

**TDD Steps:**
1. Add failing tests for uniform block layout and BT.601 constants.
2. Add failing tests for QRhi buffer upload and state tracking.
3. Implement `Yuv420ShaderUniforms`.
4. Add render-node integration coverage.
5. Run targeted tests, full build, and full CTest.

**Deferred:**
- Binding the uniform buffer into the shader resource binding set.
- Vertex buffer, shader bytecode, graphics pipeline, and draw commands.
