#include "app.h"
#include "window_menu.h"
#include "window_goal.h"

static int step_goal = 4; // default 10k steps (index 4)

int getStepGoal() {
  if (persist_exists(GOAL)) {
    step_goal = persist_read_int(GOAL);
  }
  return step_goal;
}

void setStepGoal(int choice) {
  step_goal = choice;
  persist_write_int(GOAL, choice);
}

int get_step_count() {
  HealthServiceAccessibilityMask request = health_service_metric_accessible(HealthMetricStepCount,
    time_start_of_today(), time(NULL));
  if (request == HealthServiceAccessibilityMaskAvailable) {
    return (int)health_service_sum_today(HealthMetricStepCount);
  } else {
    return 0;
  }
}

int get_step_goal_value() {
  int goal = getStepGoal();
  switch (goal) {
    case 0: return 2000;
    case 1: return 4000;
    case 2: return 6000;
    case 3: return 8000;
    case 4: return 10000;
    case 5: return 15000;
    case 6: return 20000;
    default: return 10000;
  }
}

int get_bonus_goal_value() {
  int choice = 0;
  if (persist_exists(BONUS_GOAL)) {
    choice = persist_read_int(BONUS_GOAL);
  }
  switch (choice) {
    case 0: return 0;
    case 1: return 2000;
    case 2: return 4000;
    case 3: return 6000;
    case 4: return 8000;
    case 5: return 10000;
    default: return 0;
  }
}

GColor get_custom_theme_color() {
  int theme = THEME_PURPLE;
  if (persist_exists(CUSTOM_COLOR)) {
    theme = persist_read_int(CUSTOM_COLOR);
  }
  switch (theme) {
    case THEME_PURPLE: return GColorLavenderIndigo;
    case THEME_BLUE:   return GColorCobaltBlue;
    case THEME_GREEN:  return GColorTiffanyBlue;
    case THEME_RED:    return GColorFolly;
    default:           return GColorLavenderIndigo;
  }
}

int get_streak_count() {
  int today = get_local_epoch_day();
  int last_met = persist_exists(LAST_MET_DATE) ? persist_read_int(LAST_MET_DATE) : 0;

  // Streak is broken if goal was not met yesterday (or earlier)
  if (last_met == 0 || last_met < today - 1) {
    return 0;
  }

  if (persist_exists(STREAK_COUNT)) {
    return persist_read_int(STREAK_COUNT);
  }
  return 0;
}

int get_local_epoch_day() {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int y = t->tm_year + 1900;
  int m = t->tm_mon + 1;
  int d = t->tm_mday;
  if (m < 3) {
    y--;
    m += 12;
  }
  return 365 * y + y/4 - y/100 + y/400 + (153*m+3)/5 + d;
}

static Window *s_worker_goal_window;

static void init(void) {
  if (persist_exists(GOAL)) {
    step_goal = persist_read_int(GOAL);
  }

  if (!app_worker_is_running()) {
    app_worker_launch();
  }

  if (launch_reason() == APP_LAUNCH_WORKER) {
    s_worker_goal_window = window_create();
    window_set_window_handlers(s_worker_goal_window, (WindowHandlers) {
      .load = goal_window_load,
      .unload = goal_window_unload,
    });
    window_stack_push(s_worker_goal_window, true);
  } else {
    show_progress_window();
  }
}

static void deinit(void) {
  if (s_worker_goal_window) {
    window_destroy(s_worker_goal_window);
  }
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
