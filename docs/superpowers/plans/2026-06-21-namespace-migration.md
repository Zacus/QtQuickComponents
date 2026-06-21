# Namespace Migration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add enterprise-grade C++ namespace boundaries to QtQuickComponents without changing QML imports or QML type names.

**Architecture:** Public API will eventually live in `QuickUI::Components`; implementation details will live in `QuickUI::Components::Internal`. The migration proceeds from low-risk private rendering classes toward QML-registered internals, while public API remains deferred until compatibility policy is explicitly chosen.

**Tech Stack:** C++17, Qt 6 QML type registration, CMake, CTest, Qt QuickTest.

---

## File Structure

- `docs/superpowers/specs/2026-06-21-namespace-migration-design.md`: migration strategy and compatibility rules.
- `src/window/singleWnd/rendering/*`: Phase 1 internal namespace migration.
- `src/window/singleWnd/video/*`: Phase 2 QML-registered internal namespace migration.
- `src/timeline/*`: Phase 3 timeline internal namespace migration.
- `src/theme/*`: Phase 3 theme internal namespace migration.
- `tests/*`: use qualified names or local `using` declarations in `.cpp` files after each migration.
- `examples/single_wnd_demo/*`: keep QML names unchanged; update C++ references only.

## Task 0: Commit Design And Plan

**Files:**
- Create: `docs/superpowers/specs/2026-06-21-namespace-migration-design.md`
- Create: `docs/superpowers/plans/2026-06-21-namespace-migration.md`

- [ ] **Step 1: Review design for no placeholders**

Run:
```sh
rg -n "T[B]D|T[O]DO|P[L]ACEHOLDER|fill in d[e]tails" docs/superpowers/specs/2026-06-21-namespace-migration-design.md docs/superpowers/plans/2026-06-21-namespace-migration.md
```

Expected: no matches.

- [ ] **Step 2: Commit design and plan**

Run:
```sh
git add docs/superpowers/specs/2026-06-21-namespace-migration-design.md docs/superpowers/plans/2026-06-21-namespace-migration.md
git commit -m "[功能修改] 添加命名空间迁移设计"
```

Expected: one docs-only commit.

## Task 1: Rendering Internals Namespace

**Files:**
- Modify: `src/window/singleWnd/rendering/Yuv420Vertex.h`
- Modify: `src/window/singleWnd/rendering/Yuv420GeometryBuffer.h`
- Modify: `src/window/singleWnd/rendering/Yuv420GeometryBuffer.cpp`
- Modify: `src/window/singleWnd/rendering/Yuv420GraphicsPipeline.h`
- Modify: `src/window/singleWnd/rendering/Yuv420GraphicsPipeline.cpp`
- Modify: `src/window/singleWnd/rendering/Yuv420ShaderPipeline.h`
- Modify: `src/window/singleWnd/rendering/Yuv420ShaderPipeline.cpp`
- Modify: `src/window/singleWnd/rendering/Yuv420ShaderUniforms.h`
- Modify: `src/window/singleWnd/rendering/Yuv420ShaderUniforms.cpp`
- Modify: `src/window/singleWnd/rendering/Yuv420TextureSet.h`
- Modify: `src/window/singleWnd/rendering/Yuv420TextureSet.cpp`
- Modify: `src/window/singleWnd/rendering/Yuv420RenderNode.h`
- Modify: `src/window/singleWnd/rendering/Yuv420RenderNode.cpp`
- Modify tests that reference `Yuv420*`.

- [ ] **Step 1: Add namespace wrappers to rendering headers**

Wrap each rendering type in:
```cpp
namespace QuickUI::Components::Internal {

class Yuv420RenderNode;

} // namespace QuickUI::Components::Internal
```

Use the actual class or struct declaration in place of the example.

- [ ] **Step 2: Add namespace wrappers to rendering sources**

Wrap each source implementation in:
```cpp
namespace QuickUI::Components::Internal {

// existing method definitions

} // namespace QuickUI::Components::Internal
```

Keep existing anonymous namespaces inside the source files for file-local helpers.

- [ ] **Step 3: Update dependent implementation code**

Update `VideoSurface.cpp` and any rendering cross-includes so dependent types resolve through `QuickUI::Components::Internal`.

- [ ] **Step 4: Update tests**

For each rendering test `.cpp`, add a local declaration after includes:
```cpp
using QuickUI::Components::Internal::Yuv420RenderNode;
```

Only add the types used by that test.

- [ ] **Step 5: Verify**

Run:
```sh
cmake --build /private/tmp/qtc-singlewnd-build
ctest --test-dir /private/tmp/qtc-singlewnd-build --output-on-failure
```

Expected: build succeeds and 20/20 tests pass.

- [ ] **Step 6: Commit**

Run:
```sh
git add src/window/singleWnd/rendering tests
git commit -m "[功能优化] 为渲染内部类添加命名空间"
```

## Task 2: Video Internals Namespace

**Files:**
- Modify: `src/window/singleWnd/video/GlobalVideoRenderer.h`
- Modify: `src/window/singleWnd/video/GlobalVideoRenderer.cpp`
- Modify: `src/window/singleWnd/video/VideoSurface.h`
- Modify: `src/window/singleWnd/video/VideoSurface.cpp`
- Modify: `src/window/singleWnd/rendering/Yuv420RenderNode.h`
- Modify: tests and `examples/single_wnd_demo/FramePump.*`.

- [ ] **Step 1: Move video classes into internal namespace**

Wrap `GlobalVideoRenderer` and `VideoSurface` in:
```cpp
namespace QuickUI::Components::Internal {

class GlobalVideoRenderer;
class VideoSurface;

} // namespace QuickUI::Components::Internal
```

- [ ] **Step 2: Preserve QML type names**

Replace:
```cpp
QML_ELEMENT
```

with:
```cpp
QML_NAMED_ELEMENT(GlobalVideoRenderer)
```

and:
```cpp
QML_NAMED_ELEMENT(VideoSurface)
```

- [ ] **Step 3: Update consumers**

Add qualified names or `.cpp`-local using declarations in tests and demo C++ code:
```cpp
using QuickUI::Components::Internal::GlobalVideoRenderer;
```

- [ ] **Step 4: Verify full suite**

Run:
```sh
cmake --build /private/tmp/qtc-singlewnd-build
cmake --build /private/tmp/qtc-singlewnd-examples --target qtc_single_wnd_demo qtc_single_wnd_demo_qmllint
ctest --test-dir /private/tmp/qtc-singlewnd-build --output-on-failure
QT_QPA_PLATFORM=offscreen /private/tmp/qtc-singlewnd-examples/examples/single_wnd_demo/qtc_single_wnd_demo --quit-after-ms 500
```

Expected: build succeeds, qml lint succeeds, 20/20 tests pass, demo starts and exits.

- [ ] **Step 5: Commit**

Run:
```sh
git add src/window/singleWnd/video src/window/singleWnd/rendering tests examples/single_wnd_demo
git commit -m "[功能优化] 为视频内部类添加命名空间"
```

## Task 3: Timeline And Theme Internal Namespaces

**Files:**
- Modify: `src/timeline/RulerModel.*`
- Modify: `src/timeline/TimelineViewport.*`
- Modify: `src/timeline/TimelineTrackModel.*`
- Modify: `src/timeline/RulerTick.h`
- Modify: `src/theme/ThemeFileWatcher.*`
- Modify: `src/theme/ThemeJsonLoader.*`
- Modify: `src/theme/ThemeTokens.h`
- Modify tests.

- [ ] **Step 1: Move timeline internal types into `QuickUI::Components::Internal`**

Use `QML_NAMED_ELEMENT` for QML-registered timeline internal types.

- [ ] **Step 2: Move theme internal types into `QuickUI::Components::Internal`**

Update `ComponentTheme.cpp` to reference the qualified theme helper types.

- [ ] **Step 3: Verify full suite**

Run the full verification command set from Task 2.

- [ ] **Step 4: Commit**

Run:
```sh
git add src/timeline src/theme tests CMakeLists.txt
git commit -m "[功能优化] 为时间线和主题内部类添加命名空间"
```

## Task 4: Public API Decision

**Files:**
- Modify only after explicit compatibility decision: `include/QtQuickComponents/*.h`
- Modify only after explicit compatibility decision: public API tests and docs.

- [ ] **Step 1: Choose compatibility mode**

Pick one:
```text
Keep global public API
Namespace public API with temporary aliases
Namespace public API as breaking API change
```

- [ ] **Step 2: Implement chosen mode**

Do not implement until the compatibility mode is recorded in the design document or a follow-up design.

- [ ] **Step 3: Verify and commit**

Run full verification and commit with a message that states whether the change is compatible or breaking.

## Self-Review

- Spec coverage: Tasks 0-4 cover design, rendering internals, QML-registered video internals, other internal types, and public API decision.
- Placeholder scan: The plan contains no intentional placeholders.
- Type consistency: Namespace names are consistently `QuickUI::Components` and `QuickUI::Components::Internal`.
