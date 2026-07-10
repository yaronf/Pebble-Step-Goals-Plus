#include <pebble_worker.h>

static int step_goal = 10000;
static bool goal_met = false;

enum {
  GOAL = 0,
  BONUS_GOAL = 1,
  STREAK_COUNT = 2,
  BEST_STREAK = 3,
  LAST_MET_DATE = 4,
  ENCOURAGE_50 = 5,
  ENCOURAGE_75 = 6,
  CUSTOM_COLOR = 7,
  LAST_50_VIBRATED_DAY = 8,
  LAST_75_VIBRATED_DAY = 9
};

static int get_local_epoch_day() {
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

static void set_goal(int goal) {
  switch (goal) {
    case 0: step_goal = 2000; break;
    case 1: step_goal = 4000; break;
    case 2: step_goal = 6000; break;
    case 3: step_goal = 8000; break;
    case 4: step_goal = 10000;  break;
    case 5: step_goal = 15000;  break;
    case 6: step_goal = 20000;  break;
    default: step_goal = 10000; break;
  }
}

static int get_bonus_steps() {
  if (!persist_exists(BONUS_GOAL)) return 0;
  int bonus_choice = persist_read_int(BONUS_GOAL);
  switch (bonus_choice) {
    case 0: return 0;
    case 1: return 2000;
    case 2: return 4000;
    case 3: return 6000;
    case 4: return 8000;
    case 5: return 10000;
    default: return 0;
  }
}

static void update_step_count(HealthMetric type) {
  HealthServiceAccessibilityMask request = health_service_metric_accessible(type,
    time_start_of_today(), time(NULL));
  if (request == HealthServiceAccessibilityMaskAvailable) {
    int steps = (int)health_service_sum_today(HealthMetricStepCount);
    int bonus_steps = get_bonus_steps();
    int total_goal = step_goal + bonus_steps;
    if (total_goal <= 0) total_goal = 10000;

    int8_t goal_percent = (100 * steps) / total_goal;
    APP_LOG(APP_LOG_LEVEL_INFO, "Taken %d steps today. %d percent of goal", steps, goal_percent);
    
    int today_epoch = get_local_epoch_day();
    int last_met = persist_exists(LAST_MET_DATE) ? persist_read_int(LAST_MET_DATE) : 0;

    // Clear stale streak count once a day is missed
    if (last_met > 0 && last_met < today_epoch - 1) {
      persist_write_int(STREAK_COUNT, 0);
    }

    if (last_met != today_epoch || steps == 0) {
      goal_met = false;
    }

    // Goal met check (haptics and app launch done by foreground app upon load)
    if (steps >= total_goal && !goal_met) {
      goal_met = true;

      // Update streak
      if (last_met != today_epoch) {
        int streak = persist_exists(STREAK_COUNT) ? persist_read_int(STREAK_COUNT) : 0;
        if (last_met == today_epoch - 1) {
          streak++;
        } else {
          streak = 1;
        }
        persist_write_int(STREAK_COUNT, streak);
        persist_write_int(LAST_MET_DATE, today_epoch);

        int best_streak = persist_exists(BEST_STREAK) ? persist_read_int(BEST_STREAK) : 0;
        if (streak > best_streak) {
          persist_write_int(BEST_STREAK, streak);
        }
      }

      worker_launch_app();
    }
  }
}

static void health_updates(HealthEventType eventType, void *context) {
  switch (eventType) {
    case HealthEventSignificantUpdate:
      update_step_count(HealthMetricStepCount);
      break;
    case HealthEventMovementUpdate:
      update_step_count(HealthMetricStepCount);
      break;
    case HealthEventSleepUpdate:
      break;
    default:
      break;
  }
}

static void worker_message_handler(uint16_t type, AppWorkerMessage *data) {
  set_goal(data->data0);
}

static void worker_init() {
  app_worker_message_subscribe(worker_message_handler);
  if (persist_exists(GOAL)) {
    set_goal(persist_read_int(GOAL));
  }
  health_service_events_subscribe(health_updates, NULL);
  update_step_count(HealthMetricStepCount);
}

static void worker_deinit() {
  health_service_events_unsubscribe();
}

int main(void) {
  worker_init();
  worker_event_loop();
  worker_deinit();
}
