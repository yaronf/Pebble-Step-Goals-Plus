#include "app.h"

static Window *s_window;
static Layer *s_progress_layer;
static TextLayer *s_steps_text;
static TextLayer *s_target_text;
static TextLayer *s_streak_text;
static BitmapLayer *s_streak_icon;
static GBitmap *s_streak_icon_bitmap;
static GRect s_card_rect;
static char s_steps_buffer[16];
static char s_target_buffer[32];
static char s_streak_buffer[16];

static GRect get_card_rect(GRect bounds) {
  int card_w = (bounds.size.w * 80) / 100;
  int card_h = (bounds.size.h * 48) / 100;
  return GRect((bounds.size.w - card_w) / 2, (bounds.size.h - card_h) / 2, card_w, card_h);
}

static void layout_streak_row(int streak_y) {
  if (!show_streak_in_app()) {
    layer_set_hidden(bitmap_layer_get_layer(s_streak_icon), true);
    layer_set_hidden(text_layer_get_layer(s_streak_text), true);
    return;
  }

  int streak = get_streak_count();
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_28);

  if (streak > 0) {
    snprintf(s_streak_buffer, sizeof(s_streak_buffer), "%d day%s",
      streak, streak == 1 ? "" : "s");
    text_layer_set_text(s_streak_text, s_streak_buffer);
    text_layer_set_text_alignment(s_streak_text, GTextAlignmentLeft);

    GSize text_size = graphics_text_layout_get_content_size(
      s_streak_buffer, font, GRect(0, 0, s_card_rect.size.w, 32),
      GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);

    GRect icon_bounds = gbitmap_get_bounds(s_streak_icon_bitmap);
    int gap = 6;
    int total_w = icon_bounds.size.w + gap + text_size.w;
    int start_x = s_card_rect.origin.x + (s_card_rect.size.w - total_w) / 2;
    int icon_y = streak_y + (32 - icon_bounds.size.h) / 2;

    layer_set_frame(bitmap_layer_get_layer(s_streak_icon),
      GRect(start_x, icon_y, icon_bounds.size.w, icon_bounds.size.h));
    layer_set_frame(text_layer_get_layer(s_streak_text),
      GRect(start_x + icon_bounds.size.w + gap, streak_y, text_size.w + 4, 32));
    layer_set_hidden(bitmap_layer_get_layer(s_streak_icon), false);
    layer_set_hidden(text_layer_get_layer(s_streak_text), false);
  } else {
    snprintf(s_streak_buffer, sizeof(s_streak_buffer), "No active streak");
    text_layer_set_text(s_streak_text, s_streak_buffer);
    text_layer_set_text_alignment(s_streak_text, GTextAlignmentCenter);
    layer_set_frame(text_layer_get_layer(s_streak_text),
      GRect(s_card_rect.origin.x, streak_y, s_card_rect.size.w, 32));
    layer_set_hidden(bitmap_layer_get_layer(s_streak_icon), true);
    layer_set_hidden(text_layer_get_layer(s_streak_text), false);
  }
}

static void refresh_progress_screen() {
  int steps = get_step_count();
  int main_goal = get_step_goal_value();
  int bonus = get_bonus_goal_value();
  int total_goal = main_goal + bonus;

  snprintf(s_steps_buffer, sizeof(s_steps_buffer), "%d", steps);
  text_layer_set_text(s_steps_text, s_steps_buffer);

  if (bonus > 0) {
    snprintf(s_target_buffer, sizeof(s_target_buffer), "of %d + %d bonus", main_goal, bonus);
  } else {
    snprintf(s_target_buffer, sizeof(s_target_buffer), "of %d steps", main_goal);
  }
  text_layer_set_text(s_target_text, s_target_buffer);

  layout_streak_row(s_card_rect.origin.y + s_card_rect.size.h - 32);

  // Check milestones and vibrate
  int today_epoch = get_local_epoch_day();

  bool encourage_50 = persist_exists(ENCOURAGE_50) ? persist_read_bool(ENCOURAGE_50) : true;
  int last_vibe_50 = persist_exists(LAST_50_VIBRATED_DAY) ? persist_read_int(LAST_50_VIBRATED_DAY) : 0;
  if (encourage_50 && last_vibe_50 != today_epoch && steps >= (total_goal * 50) / 100) {
    vibes_short_pulse();
    persist_write_int(LAST_50_VIBRATED_DAY, today_epoch);
  }

  bool encourage_75 = persist_exists(ENCOURAGE_75) ? persist_read_bool(ENCOURAGE_75) : true;
  int last_vibe_75 = persist_exists(LAST_75_VIBRATED_DAY) ? persist_read_int(LAST_75_VIBRATED_DAY) : 0;
  if (encourage_75 && last_vibe_75 != today_epoch && steps >= (total_goal * 75) / 100) {
    vibes_double_pulse();
    persist_write_int(LAST_75_VIBRATED_DAY, today_epoch);
  }

  if (s_progress_layer) {
    layer_mark_dirty(s_progress_layer);
  }
}

static void health_handler(HealthEventType eventType, void *context) {
  if (eventType == HealthEventSignificantUpdate || eventType == HealthEventMovementUpdate) {
    refresh_progress_screen();
  }
}

static void progress_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int steps = get_step_count();
  int main_goal = get_step_goal_value();
  int bonus = get_bonus_goal_value();
  int total_goal = main_goal + bonus;
  if (total_goal <= 0) total_goal = 10000;

  // Calculate percentage
  int percent = (steps * 100) / total_goal;
  if (percent > 100) percent = 100;
  if (percent < 0) percent = 0;

  int fill_height = (bounds.size.h * percent) / 100;
  GColor fill_color = get_custom_theme_color();

  // Draw background color (white at top, custom theme color at bottom)
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h - fill_height), 0, GCornerNone);

  graphics_context_set_fill_color(ctx, fill_color);
  graphics_fill_rect(ctx, GRect(0, bounds.size.h - fill_height, bounds.size.w, fill_height), 0, GCornerNone);

  GRect card_rect = get_card_rect(bounds);

  // Draw card background
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, card_rect, 4, GCornersAll);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_round_rect(ctx, card_rect, 4);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  show_main_menu_window();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void progress_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Progress layer
  s_progress_layer = layer_create(bounds);
  layer_set_update_proc(s_progress_layer, progress_layer_update_proc);
  layer_add_child(window_layer, s_progress_layer);

  s_card_rect = get_card_rect(bounds);

  // Steps count text layer
  s_steps_text = text_layer_create(GRect(s_card_rect.origin.x, s_card_rect.origin.y + 10, s_card_rect.size.w, 32));
  text_layer_set_background_color(s_steps_text, GColorClear);
  text_layer_set_text_color(s_steps_text, GColorBlack);
  text_layer_set_text_alignment(s_steps_text, GTextAlignmentCenter);
  text_layer_set_font(s_steps_text, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_steps_text));

  // Goal text layer
  s_target_text = text_layer_create(GRect(s_card_rect.origin.x, s_card_rect.origin.y + 42, s_card_rect.size.w, 32));
  text_layer_set_background_color(s_target_text, GColorClear);
  text_layer_set_text_color(s_target_text, GColorDarkGray);
  text_layer_set_text_alignment(s_target_text, GTextAlignmentCenter);
  text_layer_set_font(s_target_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_target_text));

  s_streak_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STREAK);
  GRect icon_bounds = gbitmap_get_bounds(s_streak_icon_bitmap);
  s_streak_icon = bitmap_layer_create(GRect(0, 0, icon_bounds.size.w, icon_bounds.size.h));
  bitmap_layer_set_bitmap(s_streak_icon, s_streak_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_streak_icon, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_streak_icon));

  s_streak_text = text_layer_create(GRect(s_card_rect.origin.x, s_card_rect.origin.y + s_card_rect.size.h - 32, s_card_rect.size.w, 32));
  text_layer_set_background_color(s_streak_text, GColorClear);
  text_layer_set_text_color(s_streak_text, GColorBlack);
  text_layer_set_font(s_streak_text, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  layer_add_child(window_layer, text_layer_get_layer(s_streak_text));

  // Enable live health update subscription
  health_service_events_subscribe(health_handler, NULL);

  refresh_progress_screen();
}

static void progress_window_appear(Window *window) {
  refresh_progress_screen();
}

static void progress_window_unload(Window *window) {
  health_service_events_unsubscribe();
  text_layer_destroy(s_steps_text);
  text_layer_destroy(s_target_text);
  bitmap_layer_destroy(s_streak_icon);
  gbitmap_destroy(s_streak_icon_bitmap);
  text_layer_destroy(s_streak_text);
  layer_destroy(s_progress_layer);
  s_window = NULL;
}

void show_progress_window(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_click_config_provider(s_window, click_config_provider);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = progress_window_load,
      .unload = progress_window_unload,
      .appear = progress_window_appear,
    });
  }
  window_stack_push(s_window, true);
}
