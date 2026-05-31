#include "app.h"

static Window *s_window;
static MenuLayer *s_menu_layer;

static uint16_t get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return 1;
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return 5;
}

static int16_t get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  return 44;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  static char subtitle_buffer[32];

  switch (cell_index->row) {
    case 0:
      snprintf(subtitle_buffer, sizeof(subtitle_buffer), "%d steps", get_step_goal_value());
      menu_cell_basic_draw(ctx, cell_layer, "Main Daily Goal", subtitle_buffer, NULL);
      break;
    case 1: {
      int bonus = get_bonus_goal_value();
      if (bonus > 0) {
        snprintf(subtitle_buffer, sizeof(subtitle_buffer), "+%d steps", bonus);
      } else {
        snprintf(subtitle_buffer, sizeof(subtitle_buffer), "None");
      }
      menu_cell_basic_draw(ctx, cell_layer, "Bonus Daily Goal", subtitle_buffer, NULL);
      break;
    }
    case 2: {
      int streak = 0;
      if (persist_exists(STREAK_COUNT)) {
        streak = persist_read_int(STREAK_COUNT);
      }
      snprintf(subtitle_buffer, sizeof(subtitle_buffer), "%d days", streak);
      menu_cell_basic_draw(ctx, cell_layer, "Streak Count", subtitle_buffer, NULL);
      break;
    }
    case 3: {
      bool v50 = persist_exists(ENCOURAGE_50) ? persist_read_bool(ENCOURAGE_50) : true;
      bool v75 = persist_exists(ENCOURAGE_75) ? persist_read_bool(ENCOURAGE_75) : true;
      if (v50 && v75) {
        snprintf(subtitle_buffer, sizeof(subtitle_buffer), "Vibe at 50%%, 75%%");
      } else if (v50) {
        snprintf(subtitle_buffer, sizeof(subtitle_buffer), "Vibe at 50%%");
      } else if (v75) {
        snprintf(subtitle_buffer, sizeof(subtitle_buffer), "Vibe at 75%%");
      } else {
        snprintf(subtitle_buffer, sizeof(subtitle_buffer), "Off");
      }
      menu_cell_basic_draw(ctx, cell_layer, "Encouragements", subtitle_buffer, NULL);
      break;
    }
    case 4: {
      int theme = THEME_PURPLE;
      if (persist_exists(CUSTOM_COLOR)) {
        theme = persist_read_int(CUSTOM_COLOR);
      }
      switch (theme) {
        case THEME_PURPLE: snprintf(subtitle_buffer, sizeof(subtitle_buffer), "Purple"); break;
        case THEME_BLUE:   snprintf(subtitle_buffer, sizeof(subtitle_buffer), "Blue"); break;
        case THEME_GREEN:  snprintf(subtitle_buffer, sizeof(subtitle_buffer), "Green"); break;
        case THEME_RED:    snprintf(subtitle_buffer, sizeof(subtitle_buffer), "Red"); break;
      }
      menu_cell_basic_draw(ctx, cell_layer, "Customizations", subtitle_buffer, NULL);
      break;
    }
  }
}

static void select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
    case 0:
      show_goal_select_window();
      break;
    case 1:
      show_bonus_select_window();
      break;
    case 2:
      show_streak_window();
      break;
    case 3:
      show_encourage_window();
      break;
    case 4:
      show_customize_window();
      break;
  }
}

static void main_menu_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_sections = get_num_sections_callback,
    .get_num_rows = get_num_rows_callback,
    .get_cell_height = get_cell_height_callback,
    .draw_row = draw_row_callback,
    .select_click = select_callback,
  });

  // Theme support for selected highlight
  GColor theme_color = get_custom_theme_color();
  menu_layer_set_highlight_colors(s_menu_layer, theme_color, GColorWhite);

  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void main_menu_window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  s_window = NULL;
}

static void main_menu_window_appear(Window *window) {
  // Update theme highlights on return
  GColor theme_color = get_custom_theme_color();
  menu_layer_set_highlight_colors(s_menu_layer, theme_color, GColorWhite);
  menu_layer_reload_data(s_menu_layer);
}

void show_main_menu_window(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = main_menu_window_load,
      .unload = main_menu_window_unload,
      .appear = main_menu_window_appear,
    });
  }
  window_stack_push(s_window, true);
}
