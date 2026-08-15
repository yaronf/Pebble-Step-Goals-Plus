# Screenshot demo mode

Hidden toggle for store / marketing screenshots. No rebuild required.

## How to toggle

On the **Streak Stats** screen, press:

**Up → Up → Down → Down → Select**

Leave at most ~2 seconds between presses. Wrong presses reset the sequence.

| Feedback | Meaning |
|----------|---------|
| Short vibe | Demo **on** |
| Double vibe | Demo **off** |

A normal Select on Emery (without completing the sequence) still toggles “Show in app screen.”

## What demo mode does

| Value | Demo | Notes |
|-------|------|--------|
| Steps | 3764 | Via `get_step_count()` — progress fill uses this too |
| Current streak | 8 days | Persist `STREAK_COUNT` |
| Best streak | 25 days | Persist `BEST_STREAK` |
| Last met | Today | So the streak reads as active |
| Show in app (Emery) | ON | Previous setting is restored on exit |

Real streak-related persist values are backed up when entering demo and restored when leaving. Steps are not written to Health — only mocked in the app while demo is on.

After enabling demo, go **back to the progress screen** so it redraws with mocked steps and streak.

## Implementation

- `toggle_screenshot_demo()` / `is_screenshot_demo()` in `src/app.c`
- Sequence handling in `src/window_streak.c`
- Persist keys `DEMO_MODE` and `DEMO_BACKUP_*` in `src/app.h`
