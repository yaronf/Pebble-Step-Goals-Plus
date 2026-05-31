#include <pebble_worker.h>

static int step_goal = 10000;
static bool goal_met = false;

enum {
  GOAL = 0
};

static void update_step_count(HealthMetric type) {
    HealthServiceAccessibilityMask request = health_service_metric_accessible(type,
    time_start_of_today(), time(NULL));
    if(request == HealthServiceAccessibilityMaskAvailable) {
      int steps = (int)health_service_sum_today(HealthMetricStepCount);
      int8_t goal = (100*steps)/step_goal;
      APP_LOG(APP_LOG_LEVEL_INFO, "Taken %d steps today. %d percent of goal", steps, goal);
      if (steps == 0) { goal_met = false ; } // day started. reset flag.
      if (goal >= 100 && !goal_met) {
        goal_met = true;
        worker_launch_app();
      }
    }
}

static void health_updates(HealthEventType eventType, void *context) {
  switch (eventType) {
    case HealthEventSignificantUpdate:
      //Day changed. Probably.
      update_step_count(HealthMetricStepCount);
    break;

    case HealthEventMovementUpdate:
      //Steps updated.
      update_step_count(HealthMetricStepCount);
    break;

    case HealthEventSleepUpdate:
    break;
    default:
    break;
  }
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
  };
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
}

static void worker_deinit() {
  health_service_events_unsubscribe();
}

int main(void) {
  worker_init();
  worker_event_loop();
  worker_deinit();
}
