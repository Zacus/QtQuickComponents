# SingleWnd YUV Shader Resource Bindings Phase

**Goal:** Build the shader resource binding layer for YUV420 rendering by binding Y, U, and V textures as fragment sampled textures.

**Scope:**
- Keep texture ownership inside `Yuv420TextureSet`.
- Add a reusable linear/clamp `QRhiSampler`.
- Add `QRhiShaderResourceBindings` with bindings:
  - binding 0: Y plane texture
  - binding 1: U plane texture
  - binding 2: V plane texture
- Reuse SRB while texture resources are unchanged.
- Recreate SRB when texture resources change.
- Expose thin `Yuv420RenderNode` methods for preparing and checking shader resources.

**Ownership / Lifetime:**
- `Yuv420TextureSet` owns sampler and SRB via `std::unique_ptr` with out-of-line deleters.
- SRB is released before textures so it never outlives the textures it references.
- `Yuv420RenderNode::releaseResources()` continues to release the whole texture set.

**TDD Steps:**
1. Add failing texture-set tests for sampler/SRB creation, layout, reuse, and recreation.
2. Implement sampler and SRB resource management.
3. Add failing render-node integration assertions.
4. Implement render-node thin wrappers.
5. Run targeted tests, full build, and full CTest.

**Deferred:**
- Uniform buffer for transform/color matrix.
- Shader bytecode and graphics pipeline creation.
- `QSGRenderNode::render()` draw commands.
