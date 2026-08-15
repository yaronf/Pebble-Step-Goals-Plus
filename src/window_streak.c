#include "app.h"

static Window *s_window;
static TextLayer *s_title_text;
static TextLayer *s_current_title;
static TextLayer *s_current_value;
static TextLayer *s_best_title;
static TextLayer *s_best_value;
static TextLayer *s_moto_text;
#ifdef PBL_PLATFORM_EMERY
static TextLayer *s_toggle_text;
static Layer *s_divider_layer;
#endif

static char s_current_buffer[16];
static char s_best_buffer[16];

// Screenshot demo: Up, Up, Down, Down, Select (≤2s between presses)
static int s_demo_seq;
static time_t s_demo_seq_time;

static void refresh_streak_stats(void) {
  int current_streak = get_streak_count();
  int best_streak = persist_exists(BEST_STREAK) ? persist_read_int(BEST_STREAK) : 0;

  snprintf(s_current_buffer, sizeof(s_current_buffer), "%d days", current_streak);
  text_layer_set_text(s_current_value, s_current_buffer);

  snprintf(s_best_buffer, sizeof(s_best_buffer), "%d days", best_streak);
  text_layer_set_text(s_best_value, s_best_buffer);

  if (current_streak > 0) {
    text_layer_set_text(s_moto_text, "Keep the streak alive!");
  } else {
    text_layer_set_text(s_moto_text, "Start a new streak today!");
  }

#ifdef PBL_PLATFORM_EMERY
  if (s_toggle_text) {
    text_layer_set_text(s_toggle_text,
      show_streak_in_app() ? "Show in app screen: ON" : "Show in app screen: OFF");
  }
#endif
}

static void demo_seq_note(void) {
  time_t now = time(NULL);
  if (s_demo_seq > 0 && (now - s_demo_seq_time) > 2) {
    s_demo_seq = 0;
  }
  s_demo_seq_time = now;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  demo_seq_note();
  if (s_demo_seq == 0 || s_demo_seq == 1) {
    s_demo_seq++;
  } else {
    s_demo_seq = 1;
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  demo_seq_note();
  if (s_demo_seq == 2) {
    s_demo_seq = 3;
  } else if (s_demo_seq == 3) {
    s_demo_seq = 4;
  } else {
    s_demo_seq = 0;
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  demo_seq_note();
  if (s_demo_seq == 4) {
    s_demo_seq = 0;
    toggle_screenshot_demo();
    refresh_streak_stats();
    return;
  }

  s_demo_seq = 0;
#ifdef PBL_PLATFORM_EMERY
  persist_write_bool(SHOW_STREAK_IN_APP, !show_streak_in_app());
  refresh_streak_stats();
#endif
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

#ifdef PBL_PLATFORM_EMERY
static void divider_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int line_y = bounds.size.h / 2;
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_draw_line(ctx, GPoint(0, line_y), GPoint(bounds.size.w, line_y));
}
#endif

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorWhite);

  int current_streak = get_streak_count();
  int best_streak = persist_exists(BEST_STREAK) ? persist_read_int(BEST_STREAK) : 0;

  s_title_text = text_layer_create(GRect(0, 10, bounds.size.w, 30));
  text_layer_set_background_color(s_title_text, GColorClear);
  text_layer_set_text_color(s_title_text, GColorBlack);
  text_layer_set_text_alignment(s_title_text, GTextAlignmentCenter);
  text_layer_set_font(s_title_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_title_text, "Streak Stats");
  layer_add_child(window_layer, text_layer_get_layer(s_title_text));

  int card_y = 45;

  s_current_title = text_layer_create(GRect(0, card_y, bounds.size.w, 20));
  text_layer_set_background_color(s_current_title, GColorClear);
  text_layer_set_text_color(s_current_title, GColorDarkGray);
  text_layer_set_text_alignment(s_current_title, GTextAlignmentCenter);
  text_layer_set_font(s_current_title, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text(s_current_title, "CURRENT STREAK");
  layer_add_child(window_layer, text_layer_get_layer(s_current_title));

  snprintf(s_current_buffer, sizeof(s_current_buffer), "%d days", current_streak);
  s_current_value = text_layer_create(GRect(0, card_y + 15, bounds.size.w, 35));
  text_layer_set_background_color(s_current_value, GColorClear);
  text_layer_set_text_color(s_current_value, GColorBlack);
  text_layer_set_text_alignment(s_current_value, GTextAlignmentCenter);
  text_layer_set_font(s_current_value, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(s_current_value, s_current_buffer);
  layer_add_child(window_layer, text_layer_get_layer(s_current_value));

  s_best_title = text_layer_create(GRect(0, card_y + 55, bounds.size.w, 20));
  text_layer_set_background_color(s_best_title, GColorClear);
  text_layer_set_text_color(s_best_title, GColorDarkGray);
  text_layer_set_text_alignment(s_best_title, GTextAlignmentCenter);
  text_layer_set_font(s_best_title, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text(s_best_title, "BEST STREAK");
  layer_add_child(window_layer, text_layer_get_layer(s_best_title));

  snprintf(s_best_buffer, sizeof(s_best_buffer), "%d days", best_streak);
  s_best_value = text_layer_create(GRect(0, card_y + 70, bounds.size.w, 30));
  text_layer_set_background_color(s_best_value, GColorClear);
  text_layer_set_text_color(s_best_value, GColorBlack);
  text_layer_set_text_alignment(s_best_value, GTextAlignmentCenter);
  text_layer_set_font(s_best_value, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_best_value, s_best_buffer);
  layer_add_child(window_layer, text_layer_get_layer(s_best_value));

#ifdef PBL_PLATFORM_EMERY
  int motto_y = bounds.size.h - 68;
#else
  int motto_y = bounds.size.h - 30;
#endif
  s_moto_text = text_layer_create(GRect(0, motto_y, bounds.size.w, 25));
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

#ifdef PBL_PLATFORM_EMERY
  int divider_x = (bounds.size.w * 20) / 100;
  int divider_w = (bounds.size.w * 60) / 100;
  s_divider_layer = layer_create(GRect(divider_x, bounds.size.h - 38, divider_w, 4));
  layer_set_update_proc(s_divider_layer, divider_layer_update_proc);
  layer_add_child(window_layer, s_divider_layer);

  s_toggle_text = text_layer_create(GRect(0, bounds.size.h - 34, bounds.size.w, 30));
  text_layer_set_background_color(s_toggle_text, GColorClear);
  text_layer_set_text_color(s_toggle_text, GColorBlack);
  text_layer_set_text_alignment(s_toggle_text, GTextAlignmentCenter);
  text_layer_set_font(s_toggle_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_toggle_text));
  text_layer_set_text(s_toggle_text,
    show_streak_in_app() ? "Show in app screen: ON" : "Show in app screen: OFF");
#endif

  s_demo_seq = 0;
}

static void window_unload(Window *window) {
  text_layer_destroy(s_title_text);
  text_layer_destroy(s_current_title);
  text_layer_destroy(s_current_value);
  text_layer_destroy(s_best_title);
  text_layer_destroy(s_best_value);
  text_layer_destroy(s_moto_text);
#ifdef PBL_PLATFORM_EMERY
  layer_destroy(s_divider_layer);
  text_layer_destroy(s_toggle_text);
  s_toggle_text = NULL;
#endif
  s_window = NULL;
}

void show_streak_window(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_click_config_provider(s_window, click_config_provider);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}
