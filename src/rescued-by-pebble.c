#include <pebble.h>

static Window *window;
static TextLayer *s_text_layer;
static TextLayer *s_rescue_time_layer;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_text_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create((GRect) { .origin = { 0, 55 }, .size = { bounds.size.w, 42 } });
  text_layer_set_background_color(s_text_layer, GColorClear);
  text_layer_set_text_color(s_text_layer, GColorBlack);

  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);

  s_rescue_time_layer = text_layer_create((GRect) { .origin = { 0, 100 }, .size = { bounds.size.w, 50 } });
  text_layer_set_background_color(s_rescue_time_layer, GColorClear);
  text_layer_set_text_color(s_rescue_time_layer, GColorBlack);

  text_layer_set_font(s_rescue_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_rescue_time_layer, GTextAlignmentCenter);
  text_layer_set_text(s_rescue_time_layer, "2");

  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_rescue_time_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void init(void) {
  window = window_create();

  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
