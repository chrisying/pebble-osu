#include <pebble.h>

#define TIMESTEP 75
#define SENSITIVITY 20
#define RANGE 60
#define ACCURACY 10

static Window *window;
static Layer *axis;
static Layer *track;
static Layer *target;
static AppTimer *timer;
static int X, Y, tX, tY;

// Background axis
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

// Draw targets
static void draw_target(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_circle(ctx, GPoint(bounds.size.w / 2 + tX, bounds.size.h / 2 - tY), 8);
  graphics_draw_circle(ctx, GPoint(bounds.size.w / 2 + tX, bounds.size.h / 2 - tY), 5);
  graphics_draw_circle(ctx, GPoint(bounds.size.w / 2 + tX, bounds.size.h / 2 - tY), 2);
}

// Track graphics
static void draw_track(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(bounds.size.w / 2 + X, bounds.size.h / 2 - Y), 5);
}

// Distance between points squared
static int dist(int x1, int y1, int x2, int y2) {
  int difx = x1 - x2;
  int dify = y1 - y2;
  return difx * difx + dify * dify;
}

// On timer call
static void timer_callback(void *data) {
  AccelData adata;
  accel_service_peek(&adata);

  X = adata.x / SENSITIVITY;
  Y = adata.y / SENSITIVITY;

  if (dist(X, Y, tX, tY) < ACCURACY) {
    tX = rand() % RANGE - RANGE / 2;
    tY = rand() % RANGE - RANGE / 2;
  }

  layer_mark_dirty(track);

  timer = app_timer_register(TIMESTEP, timer_callback, NULL);
}

// Select handler
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //reset();
}

// Click handlers
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

// Initialize windows
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  axis = layer_create(bounds);
  layer_set_update_proc(axis, &draw_axis);
  layer_add_child(window_layer, axis);

  track = layer_create(bounds);
  layer_set_update_proc(track, &draw_track);
  layer_add_child(window_layer, track);

  target = layer_create(bounds);
  layer_set_update_proc(target, &draw_target);
  layer_add_child(window_layer, target);
}

// Deinitialize windows
static void window_unload(Window *window) {
  layer_destroy(axis);
  layer_destroy(track);
  layer_destroy(target);
}

// Initialize
static void init(void) {
  X = Y = 0;
  tX = rand() % RANGE - RANGE / 2;
  tY = rand() % RANGE - RANGE / 2;
  accel_data_service_subscribe(0, NULL);
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_set_click_config_provider(window, click_config_provider);
  const bool animated = true;
  window_stack_push(window, animated);

  // Calls timer_callback after TIMESTEP ms
  timer = app_timer_register(TIMESTEP, timer_callback, NULL);
}

// Deinitialize
static void deinit(void) {
  accel_data_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
