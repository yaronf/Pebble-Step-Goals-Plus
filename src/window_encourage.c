#include "app.h"

static Window *s_window;
static MenuLayer *s_menu_layer;

static uint16_t get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return 1;
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return 2;
}

static int16_t get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  return 44;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  GRect bounds = layer_get_bounds(cell_layer);
  
  // Draw checkbox box
  int box_size = 14;
  GRect box_rect = GRect(bounds.size.w - box_size - 15, (bounds.size.h - box_size) / 2, box_size, box_size);

  bool enabled = true;
  if (cell_index->row == 0) {
    enabled = persist_exists(ENCOURAGE_50) ? persist_read_bool(ENCOURAGE_50) : true;
    menu_cell_basic_draw(ctx, cell_layer, "Vibe at 50% Goal", "Milestone encouragement", NULL);
  } else {
    enabled = persist_exists(ENCOURAGE_75) ? persist_read_bool(ENCOURAGE_75) : true;
    menu_cell_basic_draw(ctx, cell_layer, "Vibe at 75% Goal", "Milestone encouragement", NULL);
  }

  graphics_context_set_stroke_color(ctx, GColorBlack);
  GColor theme_color = get_custom_theme_color();
  graphics_context_set_fill_color(ctx, theme_color);

  if (menu_cell_layer_is_highlighted(cell_layer)) {
    graphics_context_set_stroke_color(ctx, GColorWhite);
  }

  // Draw square outline
  graphics_draw_rect(ctx, box_rect);

  // If enabled, fill inner square
  if (enabled) {
    GRect inner_rect = GRect(box_rect.origin.x + 3, box_rect.origin.y + 3, box_rect.size.w - 6, box_rect.size.h - 6);
    graphics_fill_rect(ctx, inner_rect, 0, GCornerNone);
  }
}

static void select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->row == 0) {
    bool current = persist_exists(ENCOURAGE_50) ? persist_read_bool(ENCOURAGE_50) : true;
    persist_write_bool(ENCOURAGE_50, !current);
  } else {
    bool current = persist_exists(ENCOURAGE_75) ? persist_read_bool(ENCOURAGE_75) : true;
    persist_write_bool(ENCOURAGE_75, !current);
  }
  menu_layer_reload_data(s_menu_layer);
}

static void window_load(Window *window) {
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

  GColor theme_color = get_custom_theme_color();
  menu_layer_set_highlight_colors(s_menu_layer, theme_color, GColorWhite);

  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  s_window = NULL;
}

void show_encourage_window(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}
