#include <pebble.h>

#define TIMESTEP 75
#define MAXTRACK 1200

static Window *window;
static Layer *axis;
static Layer *track;
static AppTimer *timer;
static int x[MAXTRACK], y[MAXTRACK];
static int counter;

static void draw_axis(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint top = GPoint(bounds.size.w / 2, 0);
  GPoint left = GPoint(0, bounds.size.h / 2);
  GPoint right = GPoint(bounds.size.w, bounds.size.h / 2);
  GPoint bottom = GPoint(bounds.size.w / 2, bounds.size.h);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx, top, bottom);
  graphics_draw_line(ctx, left, right);
}

static void draw_track(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorBlack);
  GRect bounds = layer_get_bounds(layer);
  for (int i = 1; i <= counter; i++) {
    GPoint last = GPoint(bounds.size.w / 2 + x[i - 1], bounds.size.h / 2 - y[i- 1]);
    GPoint cur = GPoint(bounds.size.w / 2 + x[i], bounds.size.h / 2 - y[i]);
    graphics_draw_line(ctx, last, cur);
  }
  graphics_draw_circle(ctx, GPoint(bounds.size.w / 2 + x[counter], bounds.size.h / 2 - y[counter]), 5);
}

static void timer_callback(void *data) {
  AccelData adata;
  accel_service_peek(&adata);

  x[counter] = adata.x;
  y[counter] = adata.y;

  layer_mark_dirty(track);

  counter++;
  if (counter < MAXTRACK) {
    timer = app_timer_register(TIMESTEP, timer_callback, NULL);
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //reset();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  axis = layer_create(bounds);
  layer_set_update_proc(axis, &draw_axis);
  layer_add_child(window_layer, axis);

  track = layer_create(bounds);
  layer_set_update_proc(track, &draw_track);
  layer_add_child(window_layer, track);
}

static void window_unload(Window *window) {
  layer_destroy(axis);
  layer_destroy(track);
}

static void init(void) {
  counter = 1;
  x[0] = y[0] = 0;
  accel_data_service_subscribe(0, NULL);
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_set_click_config_provider(window, click_config_provider);
  const bool animated = true;
  window_stack_push(window, animated);

  timer = app_timer_register(TIMESTEP, timer_callback, NULL);
}

static void deinit(void) {
  accel_data_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
