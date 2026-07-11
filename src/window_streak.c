#include "app.h"

static Window *s_window;
static TextLayer *s_title_text;
static TextLayer *s_current_title;
static TextLayer *s_current_value;
static TextLayer *s_best_title;
static TextLayer *s_best_value;
static TextLayer *s_moto_text;

static char s_current_buffer[16];
static char s_best_buffer[16];

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorSpringBud);

  int current_streak = get_streak_count();
  int best_streak = persist_exists(BEST_STREAK) ? persist_read_int(BEST_STREAK) : 0;

  // Title
  s_title_text = text_layer_create(GRect(0, 10, bounds.size.w, 30));
  text_layer_set_background_color(s_title_text, GColorClear);
  text_layer_set_text_color(s_title_text, GColorBlack);
  text_layer_set_text_alignment(s_title_text, GTextAlignmentCenter);
  text_layer_set_font(s_title_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_title_text, "Streak Stats");
  layer_add_child(window_layer, text_layer_get_layer(s_title_text));

  // Card background inside streak screen
  int card_y = 45;
  
  // Current Streak Label
  s_current_title = text_layer_create(GRect(0, card_y, bounds.size.w, 20));
  text_layer_set_background_color(s_current_title, GColorClear);
  text_layer_set_text_color(s_current_title, GColorDarkGray);
  text_layer_set_text_alignment(s_current_title, GTextAlignmentCenter);
  text_layer_set_font(s_current_title, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text(s_current_title, "CURRENT STREAK");
  layer_add_child(window_layer, text_layer_get_layer(s_current_title));

  // Current Streak Value
  snprintf(s_current_buffer, sizeof(s_current_buffer), "%d days", current_streak);
  s_current_value = text_layer_create(GRect(0, card_y + 15, bounds.size.w, 35));
  text_layer_set_background_color(s_current_value, GColorClear);
  text_layer_set_text_color(s_current_value, GColorBlack);
  text_layer_set_text_alignment(s_current_value, GTextAlignmentCenter);
  text_layer_set_font(s_current_value, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(s_current_value, s_current_buffer);
  layer_add_child(window_layer, text_layer_get_layer(s_current_value));

  // Best Streak Label
  s_best_title = text_layer_create(GRect(0, card_y + 55, bounds.size.w, 20));
  text_layer_set_background_color(s_best_title, GColorClear);
  text_layer_set_text_color(s_best_title, GColorDarkGray);
  text_layer_set_text_alignment(s_best_title, GTextAlignmentCenter);
  text_layer_set_font(s_best_title, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text(s_best_title, "BEST STREAK");
  layer_add_child(window_layer, text_layer_get_layer(s_best_title));

  // Best Streak Value
  snprintf(s_best_buffer, sizeof(s_best_buffer), "%d days", best_streak);
  s_best_value = text_layer_create(GRect(0, card_y + 70, bounds.size.w, 30));
  text_layer_set_background_color(s_best_value, GColorClear);
  text_layer_set_text_color(s_best_value, GColorBlack);
  text_layer_set_text_alignment(s_best_value, GTextAlignmentCenter);
  text_layer_set_font(s_best_value, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_best_value, s_best_buffer);
  layer_add_child(window_layer, text_layer_get_layer(s_best_value));

  // Motto
  s_moto_text = text_layer_create(GRect(0, bounds.size.h - 30, bounds.size.w, 25));
  text_layer_set_background_color(s_moto_text, GColorClear);
  text_layer_set_text_color(s_moto_text, GColorDarkGray);
  text_layer_set_text_alignment(s_moto_text, GTextAlignmentCenter);
  text_layer_set_font(s_moto_text, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  if (current_streak > 0) {
    text_layer_set_text(s_moto_text, "Keep the streak alive!");
  } else {
    text_layer_set_text(s_moto_text, "Start a new streak today!");
  }
  layer_add_child(window_layer, text_layer_get_layer(s_moto_text));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_title_text);
  text_layer_destroy(s_current_title);
  text_layer_destroy(s_current_value);
  text_layer_destroy(s_best_title);
  text_layer_destroy(s_best_value);
  text_layer_destroy(s_moto_text);
  s_window = NULL;
}

void show_streak_window(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}
