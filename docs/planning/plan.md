# Implementation Plan: Restoring Dashboard and Adding Settings Menu Flow

This plan details the steps required to restore the full-screen progress dashboard (where the screen fills with purple as steps increase) and implement a navigation menu accessible via the **Select** button containing:
- Main Daily Goal
- Bonus Daily Goal
- Streak Count
- Encouragements
- Customizations

## User Review Required

> [!IMPORTANT]
> - We will create a new main screen (**Progress Screen**) showing the step progress. The background will fill with the customized color (default: purple) from bottom-to-top based on current step progress towards the goal.
> - Clicking the **Select** button on the progress screen will push the **Main Settings Menu** showing the 5 required options.
> - The goal selection list (previously filling the screen on startup) will now be accessed via the "Main Daily Goal" sub-menu.
> - We will add streak tracking, bonus goals, milestone encouragements (vibration at 50%/75%), and customization (theme color selection) persisting across launches.

---

## Proposed Changes

### Core System Configuration

#### [MODIFY] [app.h](file:///Users/dauletle/Development/pebble-step-goal-plus_pebble/src/app.h)
- Define new persistent storage keys:
  - `GOAL` (0)
  - `BONUS_GOAL` (1)
  - `STREAK_COUNT` (2)
  - `BEST_STREAK` (3)
  - `LAST_MET_DATE` (4)
  - `ENCOURAGE_50` (5)
  - `ENCOURAGE_75` (6)
  - `CUSTOM_COLOR` (7)
  - `LAST_50_VIBRATED_DAY` (8)
  - `LAST_75_VIBRATED_DAY` (9)
- Declare window load functions and helper declarations for the new windows.
- Add local epoch day calculation helper signature.

#### [MODIFY] [app.c](file:///Users/dauletle/Development/pebble-step-goal-plus_pebble/src/app.c)
- Change standard startup behavior: push the new `window_progress` on normal launch instead of the settings menu directly.
- Implement timezone-aware `get_local_epoch_day()` to manage daily resets and streak calculations accurately.

---

### Dashboard & Menu UI Components

#### [NEW] [window_progress.c](file:///Users/dauletle/Development/pebble-step-goal-plus_pebble/src/window_progress.c)
- Implement full-screen dashboard showing the progress of steps today.
- Draw background filling dynamically with the chosen color (purple, blue, green, or red) from bottom to top.
- Draw a clear card in the center displaying steps and target goal.
- Subscribe to Pebble Health events so step count and filling animations update live as the user walks.
- Wire `Select` button click to push `window_main_menu`.

#### [NEW] [window_main_menu.c](file:///Users/dauletle/Development/pebble-step-goal-plus_pebble/src/window_main_menu.c)
- Create a MenuLayer listing:
  1. **Main Daily Goal** (shows current goal, e.g. "4,000 steps")
  2. **Bonus Daily Goal** (shows current bonus, e.g. "+2,000 steps")
  3. **Streak Count** (shows current streak, e.g. "5 days")
  4. **Encouragements** (shows vibrate alerts status)
  5. **Customizations** (shows selected theme color)
- Route selection clicks to launch the respective sub-menus/screens.

#### [MODIFY] [window_menu.c](file:///Users/dauletle/Development/pebble-step-goal-plus_pebble/src/window_menu.c)
- Refactor the file to act as the "Main Daily Goal" sub-selection screen rather than the home screen.
- Pop this window back to the main menu once a goal is selected.

#### [NEW] [window_bonus_select.c](file:///Users/dauletle/Development/pebble-step-goal-plus_pebble/src/window_bonus_select.c)
- Create a sub-menu to select a bonus goal (None, +2k, +4k, +6k, +8k, +10k steps).
- Save selection to `BONUS_GOAL` persistent key.

#### [NEW] [window_streak.c](file:///Users/dauletle/Development/pebble-step-goal-plus_pebble/src/window_streak.c)
- Create a simple dashboard screen displaying:
  - Current streak in days.
  - Best historical streak in days.
  - Motivation message.

#### [NEW] [window_encourage.c](file:///Users/dauletle/Development/pebble-step-goal-plus_pebble/src/window_encourage.c)
- Create a menu containing toggle options:
  - "Vibrate at 50%"
  - "Vibrate at 75%"
- Update toggles live and persist preferences.

#### [NEW] [window_customize.c](file:///Users/dauletle/Development/pebble-step-goal-plus_pebble/src/window_customize.c)
- Create a menu to select progress filling themes:
  - **Purple** (Lavender Indigo)
  - **Blue** (Cobalt Blue)
  - **Green** (Tiffany Blue)
  - **Red** (Folly / Red)
- Save to `CUSTOM_COLOR` persistent key.

---

### Background Worker Upgrades

#### [MODIFY] [StepGoal_worker.c](file:///Users/dauletle/Development/pebble-step-goal-plus_pebble/worker_src/StepGoal_worker.c)
- Integrate streak calculation:
  - When steps >= goal, check `last_met_date`.
  - If `last_met_date` is yesterday, increment `STREAK_COUNT` and update `BEST_STREAK`.
  - If `last_met_date` is before yesterday, reset `STREAK_COUNT` to 1.
  - Save `last_met_date` as today.
- Integrate mid-day milestones encouragements:
  - If steps reach 50% or 75% of the goal, and `ENCOURAGE_50`/`ENCOURAGE_75` is enabled, and the milestone wasn't already triggered today, vibrate the watch via the background worker.

---

## Task List

### Phase 1: Storage & Main Flow Setup
- [ ] **Task 1: Core Definitions and App Routing**
  - Update `src/app.h` and `src/app.c` to declare persistent storage keys, define `get_local_epoch_day()`, and route standard launch to progress window.
- [ ] **Task 2: Implement Progress Dashboard**
  - Implement `src/window_progress.c` with dynamic bottom-up background fill and live health subscription updates.

### Phase 2: Navigation & Settings Menus
- [ ] **Task 3: Implement Settings Navigation Menu**
  - Implement `src/window_main_menu.c` presenting the five list items.
- [ ] **Task 4: Refactor Main Goal Selection**
  - Refactor `src/window_menu.c` to be pushed/popped as a sub-menu.
- [ ] **Task 5: Implement Bonus, Streak, Customization & Encouragement Screens**
  - Create the sub-menu C files for configuring bonus goals, streak metrics, color customization, and vibe settings.

### Phase 3: Streak & Vibe Worker Upgrades
- [ ] **Task 6: Upgrade StepGoal_worker.c**
  - Add milestone checks and streak date calculations inside the background worker.
- [ ] **Task 7: Visual and Integration Validation**
  - Compile the code, start the emulator, take screenshots, and verify navigation transitions.

---

## Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| Timezone conversion bugs on daily resetting | Medium | Implement local time Gregorian-to-Julian/epoch calculation instead of UTC division, matching local Pebble watch settings. |
| Memory limits (RAM) on older Pebble models | Medium | Keep layouts simple and recycle windows appropriately by popping them off the stack instead of piling them up. |
