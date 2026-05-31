#include "app.h"

static Window *s_window;
static MenuLayer *s_menu_layer;

static uint16_t get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return 1;
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return 4;
}

static int16_t get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  return 44;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  GRect bounds = layer_get_bounds(cell_layer);
  GPoint p = GPoint(bounds.size.w - 15, (bounds.size.h / 2));

  switch (cell_index->row) {
    case THEME_PURPLE: menu_cell_basic_draw(ctx, cell_layer, "Purple theme", "Lavender Indigo", NULL); break;
    case THEME_BLUE:   menu_cell_basic_draw(ctx, cell_layer, "Blue theme", "Cobalt Blue", NULL); break;
    case THEME_GREEN:  menu_cell_basic_draw(ctx, cell_layer, "Green theme", "Tiffany Blue", NULL); break;
    case THEME_RED:    menu_cell_basic_draw(ctx, cell_layer, "Red theme", "Folly Red", NULL); break;
  }

  graphics_context_set_stroke_color(ctx, GColorBlack);
  
  // Set custom fill colors based on rows for radio button dots!
  switch (cell_index->row) {
    case THEME_PURPLE: graphics_context_set_fill_color(ctx, GColorLavenderIndigo); break;
    case THEME_BLUE:   graphics_context_set_fill_color(ctx, GColorCobaltBlue); break;
    case THEME_GREEN:  graphics_context_set_fill_color(ctx, GColorTiffanyBlue); break;
    case THEME_RED:    graphics_context_set_fill_color(ctx, GColorFolly); break;
  }

  if (menu_cell_layer_is_highlighted(cell_layer)) {
    graphics_context_set_stroke_color(ctx, GColorWhite);
  }
  graphics_draw_circle(ctx, p, 7);

  int current_theme = THEME_PURPLE;
  if (persist_exists(CUSTOM_COLOR)) {
    current_theme = persist_read_int(CUSTOM_COLOR);
  }

  if (cell_index->row == current_theme) {
    graphics_fill_circle(ctx, p, 5);
  }
}

static void select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  persist_write_int(CUSTOM_COLOR, cell_index->row);
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

void show_customize_window(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}
