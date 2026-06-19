# SingleWnd YUV Resource State Phase

**Goal:** Add testable YUV render-node texture state so the node can track Y/U/V plane sizes and frame serial upload status before the shader pipeline phase.

**Scope:**
- Keep `Yuv420RenderNode` as the SceneGraph-owned render node.
- Track Y, U, and V texture dimensions from the current YUV420 snapshot.
- Track the latest frame serial considered uploaded by the render-node resource layer.
- Avoid introducing a Qt private `GuiPrivate` dependency in this phase.

**Ownership / Lifetime:**
- `Yuv420RenderNode` remains owned by the SceneGraph node tree.
- YUV frame bytes are still deep-copied into the node snapshot.
- Texture-state fields are value types stored in the node and cleared with the node.

**TDD Steps:**
1. Add a failing `Yuv420RenderNode` unit test for texture sizes and pending upload serial state.
2. Add minimal state accessors and upload completion tracking to the node.
3. Re-run the targeted test, then the full build and CTest suite.

**Deferred:**
- Actual `QRhiTexture` allocation.
- Resource update batch uploads.
- Shader resource bindings and draw commands.
