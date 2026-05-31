#include "app.h"

static Window *s_window;
static MenuLayer *s_menu_layer;

#define BONUS_GOALS 6

static uint16_t get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return 1;
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return BONUS_GOALS;
}

static int16_t get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  return 44;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  GRect bounds = layer_get_bounds(cell_layer);
  GPoint p = GPoint(bounds.size.w - 15, (bounds.size.h / 2));

  switch (cell_index->row) {
    case 0: menu_cell_basic_draw(ctx, cell_layer, "None", NULL, NULL); break;
    case 1: menu_cell_basic_draw(ctx, cell_layer, "+2k steps", NULL, NULL); break;
    case 2: menu_cell_basic_draw(ctx, cell_layer, "+4k steps", NULL, NULL); break;
    case 3: menu_cell_basic_draw(ctx, cell_layer, "+6k steps", NULL, NULL); break;
    case 4: menu_cell_basic_draw(ctx, cell_layer, "+8k steps", NULL, NULL); break;
    case 5: menu_cell_basic_draw(ctx, cell_layer, "+10k steps", NULL, NULL); break;
  }

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorFolly);

  if (menu_cell_layer_is_highlighted(cell_layer)) {
    graphics_context_set_stroke_color(ctx, GColorWhite);
  }
  graphics_draw_circle(ctx, p, 7);

  int current_bonus = 0;
  if (persist_exists(BONUS_GOAL)) {
    current_bonus = persist_read_int(BONUS_GOAL);
  }

  if (cell_index->row == current_bonus) {
    graphics_fill_circle(ctx, p, 5);
  }
}

static void select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  persist_write_int(BONUS_GOAL, cell_index->row);
  window_stack_pop(true);
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

void show_bonus_select_window(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}
