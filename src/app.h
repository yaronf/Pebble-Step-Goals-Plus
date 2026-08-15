#include <pebble.h>

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
  LAST_75_VIBRATED_DAY = 9,
  SHOW_STREAK_IN_APP = 10,
  DEMO_MODE = 11,
  DEMO_BACKUP_STREAK = 12,
  DEMO_BACKUP_BEST = 13,
  DEMO_BACKUP_LAST_MET = 14,
  DEMO_BACKUP_SHOW_STREAK = 15
};

#define THEME_PURPLE 0
#define THEME_BLUE   1
#define THEME_GREEN  2
#define THEME_RED    3

int getStepGoal();
void setStepGoal(int choice);

int get_step_count();
int get_step_goal_value();
int get_bonus_goal_value();
GColor get_custom_theme_color();
int get_local_epoch_day();
int get_streak_count();
bool show_streak_in_app();
bool is_screenshot_demo(void);
void toggle_screenshot_demo(void);

void show_progress_window(void);
void show_main_menu_window(void);
void show_goal_select_window(void);
void show_bonus_select_window(void);
void show_streak_window(void);
void show_encourage_window(void);
void show_customize_window(void);
