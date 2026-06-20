# SingleWnd YUV Texture Upload Phase

**Goal:** Queue YUV420 plane data uploads into the QRhi texture set and expose a render-node-level upload method for the current snapshot.

**Scope:**
- Add `Yuv420TextureSet::uploadFrame()`.
- Validate YUV420 dimensions, strides, and plane byte lengths before queuing uploads.
- Preserve decoder-provided row strides through `QRhiTextureSubresourceUploadDescription::setDataStride()`.
- Track the latest uploaded frame serial and skip duplicate serial uploads.
- Add `Yuv420RenderNode::uploadPendingTextureData()` to upload the current snapshot through the texture set.

**Ownership / Lifetime:**
- `QRhiResourceUpdateBatch*` is a non-owning, call-scoped pointer.
- The texture set owns textures; the upload method does not retain the batch pointer.
- Tests release QRhi update batches with `QRhiResourceUpdateBatch::release()` because batches are QRhi-managed resources.

**TDD Steps:**
1. Add failing texture-set tests for plane upload and serial tracking.
2. Implement upload validation and QRhi upload queuing.
3. Add failing render-node integration coverage for uploading the current snapshot.
4. Implement the render-node upload method.
5. Run targeted tests, full build, and full CTest.

**Deferred:**
- Calling this upload path from `QSGRenderNode::render()`.
- Shader resource bindings.
- YUV-to-RGB graphics pipeline and draw commands.
