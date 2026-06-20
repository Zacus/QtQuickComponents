# SingleWnd YUV RHI Texture Set Phase

**Goal:** Add an internal QRhi-backed texture set for YUV420 planes so the render node can allocate and release Y, U, and V textures before the upload and shader phases.

**Scope:**
- Add `Yuv420TextureSet` as an internal C++ helper.
- Allocate three `QRhiTexture::R8` textures for Y, U, and V planes.
- Reuse textures when the QRhi and plane sizes are unchanged.
- Release textures explicitly when the SceneGraph node releases resources.
- Link the implementation target to `Qt6::GuiPrivate` because Qt RHI headers are versioned private headers.

**Ownership / Lifetime:**
- `Yuv420TextureSet` owns its `QRhiTexture` objects via `std::unique_ptr` with an out-of-line deleter.
- `QRhi*` is non-owning and must outlive the texture set resources.
- `Yuv420RenderNode::releaseResources()` releases the texture set on SceneGraph teardown.

**TDD Steps:**
1. Add failing tests for creating, reusing, and releasing YUV QRhi textures with the Null backend.
2. Implement `Yuv420TextureSet`.
3. Add render-node integration coverage for `releaseResources()`.
4. Run targeted tests, full build, and full CTest.

**Deferred:**
- Plane byte upload through `QRhiResourceUpdateBatch`.
- YUV-to-RGB shader resource bindings.
- Graphics pipeline creation and draw commands.
