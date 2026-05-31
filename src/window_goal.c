#include "app.h"

static TextLayer *s_goal_text;
static GBitmap *s_goal_icon;
static BitmapLayer *s_bitmap_layer;

void quitApp() {
  window_stack_pop_all(true);
}

void goal_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorSpringBud);

  s_goal_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TROPHY);
  GRect dimensions = gbitmap_get_bounds(s_goal_icon);

  // Dynamic layout calculation to prevent overlap on any platform
  int gap = 10;
  int text_height = 50;
  int total_height = dimensions.size.h + gap + text_height;
  int y_start = (bounds.size.h - total_height) / 2;

  s_bitmap_layer = bitmap_layer_create(GRect(bounds.size.w/2-dimensions.size.w/2, y_start, dimensions.size.w, dimensions.size.h));
  bitmap_layer_set_bitmap(s_bitmap_layer, s_goal_icon);
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));

  s_goal_text = text_layer_create(GRect(0, y_start + dimensions.size.h + gap, bounds.size.w, text_height));
  text_layer_set_background_color(s_goal_text, GColorClear);
  text_layer_set_text_color(s_goal_text, GColorBlack);
  text_layer_set_text(s_goal_text, "Daily Step Goal Met!");
  text_layer_set_font(s_goal_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_goal_text, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_goal_text));

  vibes_double_pulse();
  app_timer_register(5000, quitApp, NULL);
}

void goal_window_unload(Window *window) {
  bitmap_layer_destroy(s_bitmap_layer);
  text_layer_destroy(s_goal_text);
}
