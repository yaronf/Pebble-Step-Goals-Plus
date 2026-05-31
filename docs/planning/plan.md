# Implementation Plan: Pebble Step Goals Plus Upgrade for Pebble Time 2 (Emery)

This plan details the steps required to upgrade the Pebble Step Goals app to support the Pebble Time 2 (`emery` platform), resolve compilation errors under the current Pebble SDK, improve the layout algorithm to automatically scale across platforms, and produce high-resolution assets.

## User Review Required

> [!IMPORTANT]
> The Pebble Time 2 (`emery` platform) features a higher resolution (200x228) and a 64-color screen.
> To support this screen, we must:
> 1. Bump the app version to `1.0` (next major version).
> 2. Fix a compiler error in `worker_src/StepGoal_worker.c` caused by missing enum cases in `health_updates()`.
> 3. Add a high-resolution, color-quantized asset for the trophy (`trophy~emery.png`) using the Pebble 64-color palette.
> 4. Modify the layout math in `src/window_goal.c` to prevent image/text overlap when using larger assets.

## Future Roadmap

> [!TIP]
> **Trophy Graphic Upgrade:** For future versions, consider replacing the flat black silhouette trophy with a colorful 64-color pixel-art style golden trophy to make better use of Pebble Time 2's color capabilities.

---


## Proposed Changes

### Configuration & Compilation Fixes

#### [MODIFY] [appinfo.json](file:///Users/dauletle/Development/pebble-step-goal-plus_pebble/appinfo.json)
- Add `"emery"` to the `"targetPlatforms"` array.
- Bump `"versionLabel"` to `"1.0"`.

#### [MODIFY] [StepGoal_worker.c](file:///Users/dauletle/Development/pebble-step-goal-plus_pebble/worker_src/StepGoal_worker.c)
- Add missing enum cases (`HealthEventMetricAlert`, `HealthEventHeartRateUpdate`) or a `default` case to the `health_updates()` event handler switch to resolve the `-Werror=switch` compilation error.

---

### UI & Resource Upgrades

#### [NEW] [trophy~emery.png](file:///Users/dauletle/Development/pebble-step-goal-plus_pebble/resources/images/trophy~emery.png)
- Scale/upsample the existing flat black silhouette trophy to `120x120` pixels.
- Ensure transparency is preserved so it looks crisp and unpixelated on Pebble Time 2.


#### [MODIFY] [window_goal.c](file:///Users/dauletle/Development/pebble-step-goal-plus_pebble/src/window_goal.c)
- Replace static coordinate offset logic with dynamic layout positioning:
  - Center the combination of the trophy bitmap and the text layer as a single group.
  - Dynamically read `gbitmap_get_bounds(s_goal_icon)` so it automatically works for `120x120` (emery) and `85x85` (basalt/chalk) without overlaps.

---

## Task List

### Phase 1: Foundation (Fixes & Config)
- [ ] **Task 1: Resolve Worker Switch Compiler Error**
  - **Description:** Add handling for all `HealthEventType` enums in `worker_src/StepGoal_worker.c` to fix the build warning-as-error under SDK 4.9.
  - **Acceptance criteria:** `pebble build` compiles successfully for existing platforms (`basalt`, `chalk`).
  - **Verification:** Run `pebble build` and check for compile success.
- [ ] **Task 2: Configure targetPlatforms & Version**
  - **Description:** Add `"emery"` platform to `"targetPlatforms"` in `appinfo.json` and bump version label to `"1.0"`.
  - **Acceptance criteria:** `appinfo.json` has `emery` and version `1.0`.
  - **Verification:** Run `pebble build` and check that the build system attempts to compile for all three platforms.

### Checkpoint: Foundation
- [ ] Compilation succeeds for `basalt`, `chalk`, and `emery`.

### Phase 2: UI & Resource Enhancements
- [ ] **Task 3: Create High-Resolution Emery Asset**
  - **Description:** Scale/upsample the existing flat black silhouette trophy (`resources/images/trophy.png`) to a clean `120x120` pixels asset and place it at `resources/images/trophy~emery.png`.
  - **Acceptance criteria:** Asset exists at the correct path, has resolution `120x120` px, and maintains flat black color with transparency.
  - **Verification:** Run a Python script to verify size is `120x120` px and transparency is maintained.

- [ ] **Task 4: Implement Dynamic Layout Math**
  - **Description:** Update `src/window_goal.c` to dynamically position elements to avoid text-image overlap on all screen sizes.
  - **Acceptance criteria:** Window centers the trophy and text dynamically as a vertical group.
  - **Verification:** Review math in code; verify there is no hardcoded overlap on any platform.

### Checkpoint: Core Features
- [ ] Compilation succeeds for all platforms.
- [ ] Asset resource pack is built correctly for `emery`.

### Phase 3: Polish & Emulation Check
- [ ] **Task 5: Verify build & run emulator**
  - **Description:** Run full compile and boot the app on the Emery emulator to visually verify layout.
  - **Acceptance criteria:** Emulator runs Emery and displays the step goals screen/goal met screen correctly.
  - **Verification:** Run `pebble install --emulator emery` and check the visual layout.

---

## Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| Emery compiler fails on deprecation issues | High | Investigate and modify deprecated API calls in C files. |
| PNG quantization loses too much quality | Medium | Tweak the Python quantization script or try different source designs to ensure a clean appearance at 64 colors. |
| Emulator does not launch on macOS host | Low | Rely on C layout reviews and successful compiler output for the `emery` platform. |
