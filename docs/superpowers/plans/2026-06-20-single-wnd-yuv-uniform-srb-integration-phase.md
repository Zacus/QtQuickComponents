# SingleWnd YUV Uniform SRB Integration Phase

**Goal:** Integrate the YUV shader uniform buffer into the shader resource binding layout used by the YUV texture resources.

**Scope:**
- Change `Yuv420TextureSet::ensureShaderResources()` to require a non-owning `QRhiBuffer*` uniform buffer.
- Build one SRB layout:
  - binding 0: fragment-stage uniform buffer
  - binding 1: Y sampled texture
  - binding 2: U sampled texture
  - binding 3: V sampled texture
- Rebuild SRB when texture resources or uniform buffer identity changes.
- Update `Yuv420RenderNode` to pass its `Yuv420ShaderUniforms` buffer into texture resource binding setup.

**Ownership / Lifetime:**
- `Yuv420ShaderUniforms` owns the uniform buffer.
- `Yuv420TextureSet` stores only a non-owning pointer identity to decide whether SRB rebuild is needed.
- `Yuv420RenderNode::releaseResources()` releases uniforms before textures/SRB, so the SRB does not outlive the uniform buffer.

**TDD Steps:**
1. Update texture-set tests to require a 4-binding SRB layout.
2. Update render-node tests to upload uniforms before preparing shader resources.
3. Implement the new non-owning uniform-buffer input.
4. Run targeted tests, full build, and full CTest.

**Deferred:**
- Graphics pipeline creation.
- Vertex buffer and shader bytecode.
- Actual `QSGRenderNode::render()` commands.
