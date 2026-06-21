# Namespace Migration Design

## Objective

Introduce C++ namespaces to reduce symbol collisions when QtQuickComponents is embedded in larger applications, while preserving existing QML import URIs and QML type names.

## Namespace Policy

Use two stable namespace roots:

```cpp
namespace QuickUI::Components {
}

namespace QuickUI::Components::Internal {
}
```

- Public C++ API lives in `QuickUI::Components`.
- Private implementation types live in `QuickUI::Components::Internal`.
- File-local helpers stay in anonymous namespaces.
- Headers must not use `using namespace`.
- QML module URIs remain `QuickUI.Components` and `QuickUI.Components.impl`.
- QML-facing C++ types keep their QML names with `QML_NAMED_ELEMENT(TypeName)` when their C++ class enters a namespace.

## Current Type Classification

### Public API

These headers are installed from `include/QtQuickComponents` and are treated as public API:

- `ComponentTheme`
- `TimelineEnums`
- `TimelineModel`
- `TimelineSegment`
- `WndViewModel`

Public API migration is deferred until the internal namespaces are stable. When public API is migrated, the project must choose between a compatibility alias period and a breaking major-version change.

### Internal API

These types are build-private and can be migrated first:

- Rendering: `Yuv420RenderNode`, `Yuv420GeometryBuffer`, `Yuv420GraphicsPipeline`, `Yuv420ShaderPipeline`, `Yuv420ShaderUniforms`, `Yuv420TextureSet`, `Yuv420Vertex`
- Video: `GlobalVideoRenderer`, `VideoSurface`
- Timeline internals: `TimelineViewport`, `RulerModel`, `TimelineTrackModel`, `RulerTick`
- Theme internals: `ThemeFileWatcher`, `ThemeJsonLoader`, `ThemeTokens`, `ThemeLoadResult`

## Migration Strategy

### Phase 0: Design And Plan

Create this design document and a task-level implementation plan. No source behavior changes are made in this phase.

### Phase 1: Pure Rendering Internals

Move `src/window/singleWnd/rendering` types into `QuickUI::Components::Internal`.

This phase has the lowest risk because the rendering types are not QML-registered and are used only by other implementation code and tests.

### Phase 2: Video QML-Registered Internals

Move `GlobalVideoRenderer` and `VideoSurface` into `QuickUI::Components::Internal`.

Because both classes are QML-registered, replace `QML_ELEMENT` with `QML_NAMED_ELEMENT(GlobalVideoRenderer)` and `QML_NAMED_ELEMENT(VideoSurface)` so QML usage remains unchanged.

### Phase 3: Timeline And Theme Internals

Move timeline implementation types and theme implementation types into `QuickUI::Components::Internal`.

For QML-registered internal types, use `QML_NAMED_ELEMENT` to preserve QML names.

### Phase 4: Public API Decision

Evaluate public API migration separately. Acceptable enterprise options:

1. Keep public API in the global namespace for source compatibility.
2. Migrate public API into `QuickUI::Components` and provide temporary global aliases.
3. Migrate public API into `QuickUI::Components` without aliases as a breaking major-version change.

This phase must include versioning and release-note decisions before implementation.

Decision for this migration: choose option 2, migrate public C++ API into `QuickUI::Components` and provide temporary global aliases. This keeps existing source code compatible while giving new consumers a collision-resistant namespace. The aliases are a compatibility bridge and should remain until a future major-version decision explicitly removes them.

Versioning decision: this is a backward-compatible public C++ API expansion, so publishing this phase should use a minor version bump. The implementation updates `VERSION.txt` from `1.0.0` to `1.1.0`; QML imports and type names remain unchanged.

## Compatibility Requirements

- QML imports do not change.
- QML type names do not change.
- Public installed header paths do not change.
- Tests and examples may use namespace aliases in `.cpp` files, but not in library headers.
- Every phase must be independently buildable, testable, and committable.

## Verification

Run after every implementation phase:

```sh
cmake --build /private/tmp/qtc-singlewnd-build
cmake --build /private/tmp/qtc-singlewnd-examples --target qtc_single_wnd_demo qtc_single_wnd_demo_qmllint
ctest --test-dir /private/tmp/qtc-singlewnd-build --output-on-failure
QT_QPA_PLATFORM=offscreen /private/tmp/qtc-singlewnd-examples/examples/single_wnd_demo/qtc_single_wnd_demo --quit-after-ms 500
```

## Acceptance Criteria

- No implementation class remains in the global namespace after its migration phase.
- QML examples continue to load without import or type-name changes.
- Existing tests pass after each phase.
- Public API migration is not performed without an explicit compatibility decision.
