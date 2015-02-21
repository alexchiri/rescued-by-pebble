#include <pebble.h>

static Window *window;
static TextLayer *s_time_text_layer;
static TextLayer *s_day_name_text_layer;
static TextLayer *s_date_week_time_layer;
static Layer *s_bt_image_layer;
static Layer *s_status_image_layer;
static GBitmap *s_status_image;
static GBitmap *s_bt_image;
static Layer *s_progress_layer;
static InverterLayer *inverter_layer;
static GFont *s_banana_brick_font_42;
static int productivity;
static bool bluetooth;
static bool inverted;

#define API_KEY 0
#define PRODUCTIVITY 1
#define INVERTED 2

#define INVERTED_SETTINGS_KEY 0

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message received!");
  
  static char string_productivity_buffer[32];
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case PRODUCTIVITY:
      productivity = (int)t->value->uint8;
      layer_mark_dirty(s_progress_layer);
      layer_mark_dirty(s_status_image_layer);
      break;
    case INVERTED:
      if(strcmp("on", t->value->cstring) == 0) {
        persist_write_bool(INVERTED_SETTINGS_KEY, true);

        if(inverted == false) {
          Layer *window_layer = window_get_root_layer(window);
          GRect bounds = layer_get_bounds(window_layer);

          inverter_layer = inverter_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, bounds.size.h } });
          layer_add_child(window_layer, inverter_layer_get_layer(inverter_layer));
        }
        inverted = true;
      } else {
        persist_write_bool(INVERTED_SETTINGS_KEY, false);

        if(inverted == true) {
          inverter_layer_destroy(inverter_layer);
        }

        inverted = false;
      }
      break;
    case API_KEY:
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  
//  text_layer_set_text(s_rescue_time_layer, string_productivity_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void update_time(struct tm *tick_time) {
  // Create a long-lived buffer
  static char time_buffer[] = "00:00";
  static char day_name_buffer[] = "Wednesday";
  static char date_week_buffer[] = "00 Mon YEAR, wk no";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(time_buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(time_buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  strftime(day_name_buffer, sizeof("Wednesday"), "%A", tick_time);
  strftime(date_week_buffer, sizeof("00 Mon YEAR, wk no"), "%d %b %Y, wk %W", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_text_layer, time_buffer);
  text_layer_set_text(s_day_name_text_layer, day_name_buffer);
  text_layer_set_text(s_date_week_time_layer, date_week_buffer);
}

static void get_rescue_time_info() {
  // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Sent message to get RescueTime productivity data!");
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "tick_handler: start, Heap Available: %d", heap_bytes_free());
  update_time(tick_time);

  if(tick_time->tm_min % 5 == 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Time to get some more RescueTime productivity data!");
    get_rescue_time_info();
  }

  APP_LOG(APP_LOG_LEVEL_DEBUG, "tick_handler: end, Heap Available: %d", heap_bytes_free());
}

static void update_bt_image_layer_proc(Layer *layer, GContext *ctx) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "update_bt_image_layer_proc: start, Heap Available: %d", heap_bytes_free());

  GBitmap *old_s_bt_image = s_bt_image;

  if(bluetooth) {
    s_bt_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ON);
  } else {
    s_bt_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_OFF);
  }

  graphics_draw_bitmap_in_rect(ctx, s_bt_image, layer_get_bounds(s_bt_image_layer));

  // destroy old image to free up heap memory
  gbitmap_destroy(old_s_bt_image);
  APP_LOG(APP_LOG_LEVEL_ERROR, "update_bt_image_layer_proc: end, Heap Available: %d", heap_bytes_free());
}

void bluetooth_callback(bool connected) {
  bluetooth = connected;

//TODO: to figure out why this dirty causes a crash
//  layer_mark_dirty(s_bt_image_layer);
//  APP_LOG(APP_LOG_LEVEL_ERROR, "Marked layer dirty");
}

static void update_status_image_layer_proc(Layer *layer, GContext *ctx) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "update_status_image_layer_proc: start, Heap Available: %d", heap_bytes_free());

  GBitmap *old_s_status_image = s_status_image;

  if(productivity < 25) {
    s_status_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_25_PERCENT);
  } else if(productivity < 50) {
    s_status_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_50_PERCENT);
  } else if(productivity < 75) {
    s_status_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_75_PERCENT);
  } else if(productivity <= 100) {
    s_status_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_100_PERCENT);
  }

  APP_LOG(APP_LOG_LEVEL_ERROR, "update_status_image_layer_proc: before draw, Heap Available: %d", heap_bytes_free());
  graphics_draw_bitmap_in_rect(ctx, s_status_image, layer_get_bounds(s_status_image_layer));
  APP_LOG(APP_LOG_LEVEL_ERROR, "update_status_image_layer_proc: after draw, Heap Available: %d", heap_bytes_free());

  // destroy old image to free up heap memory
  gbitmap_destroy(old_s_status_image);
  APP_LOG(APP_LOG_LEVEL_ERROR, "update_status_image_layer_proc: end, Heap Available: %d", heap_bytes_free());
}

static void update_progress_layer_proc(Layer *layer, GContext *ctx) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "update_progress_layer_proc: start, Heap Available: %d", heap_bytes_free());
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, (GRect) { .origin = { 2, 90 }, .size = { 100, 3 } }, 0, GCornerNone);
  graphics_fill_rect(ctx, (GRect) { .origin = { 2, 108 }, .size = { 100, 3 } }, 0, GCornerNone);

  graphics_fill_rect(ctx, (GRect) { .origin = { 2, 85 }, .size = { 3, 5 } }, 0, GCornerNone);
  graphics_fill_rect(ctx, (GRect) { .origin = { 27, 87 }, .size = { 3, 3 } }, 0, GCornerNone);
  graphics_fill_rect(ctx, (GRect) { .origin = { 52, 85 }, .size = { 3, 5 } }, 0, GCornerNone);
  graphics_fill_rect(ctx, (GRect) { .origin = { 77, 87 }, .size = { 3, 3 } }, 0, GCornerNone);
  graphics_fill_rect(ctx, (GRect) { .origin = { 99, 85 }, .size = { 3, 5 } }, 0, GCornerNone);

  graphics_fill_rect(ctx, (GRect) { .origin = { 2,  111}, .size = { 3, 5 } }, 0, GCornerNone);
  graphics_fill_rect(ctx, (GRect) { .origin = { 27, 111 }, .size = { 3, 3 } }, 0, GCornerNone);
  graphics_fill_rect(ctx, (GRect) { .origin = { 52, 111 }, .size = { 3, 5 } }, 0, GCornerNone);
  graphics_fill_rect(ctx, (GRect) { .origin = { 77, 111 }, .size = { 3, 3 } }, 0, GCornerNone);
  graphics_fill_rect(ctx, (GRect) { .origin = { 99, 111 }, .size = { 3, 5 } }, 0, GCornerNone);

  graphics_fill_rect(ctx, (GRect) { .origin = { 2, 93 }, .size = { productivity, 15 } }, 0, GCornerNone);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "update_progress_layer_proc: end, Heap Available: %d", heap_bytes_free());
}

static void window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Got into window_load!");

  bool is_connected = bluetooth_connection_service_peek();
  bluetooth_callback(is_connected);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_day_name_text_layer = text_layer_create((GRect) { .origin = { 0, 8 }, .size = { bounds.size.w - 35, 25 } });
  text_layer_set_background_color(s_day_name_text_layer, GColorClear);
  text_layer_set_text_color(s_day_name_text_layer, GColorBlack);
  text_layer_set_font(s_day_name_text_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(s_day_name_text_layer, GTextAlignmentCenter);

  s_banana_brick_font_42 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BANANA_BRICK_42));
  s_time_text_layer = text_layer_create((GRect) { .origin = { 0, 30 }, .size = { bounds.size.w - 35, 43 } });
  text_layer_set_background_color(s_time_text_layer, GColorClear);
  text_layer_set_text_color(s_time_text_layer, GColorBlack);
  text_layer_set_font(s_time_text_layer, s_banana_brick_font_42);
  text_layer_set_text_alignment(s_time_text_layer, GTextAlignmentCenter);

  s_date_week_time_layer = text_layer_create((GRect) { .origin = { 0, 130 }, .size = { bounds.size.w, 20 } });
  text_layer_set_background_color(s_date_week_time_layer, GColorClear);
  text_layer_set_text_color(s_date_week_time_layer, GColorBlack);
  text_layer_set_font(s_date_week_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_date_week_time_layer, GTextAlignmentCenter);

  s_bt_image_layer = layer_create((GRect) { .origin = { bounds.size.w - 30, 20 }, .size = { 22, 52 } });
  layer_set_update_proc(s_bt_image_layer, update_bt_image_layer_proc);

  s_status_image_layer = layer_create((GRect) { .origin = { bounds.size.w - 40, 80 }, .size = { 40, 40 } });
  layer_set_update_proc(s_status_image_layer, update_status_image_layer_proc);

  s_progress_layer = layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, bounds.size.h } });
  layer_set_update_proc(s_progress_layer, update_progress_layer_proc);

  layer_add_child(window_layer, text_layer_get_layer(s_day_name_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_week_time_layer));
  layer_add_child(window_layer, s_bt_image_layer);
  layer_add_child(window_layer, s_status_image_layer);
  layer_add_child(window_layer, s_progress_layer);

  if(inverted) {

  }
}

static void window_unload(Window *window) {
  fonts_unload_custom_font(s_banana_brick_font_42);

  text_layer_destroy(s_time_text_layer);
  text_layer_destroy(s_day_name_text_layer);
  text_layer_destroy(s_date_week_time_layer);

  gbitmap_destroy(s_status_image);
  gbitmap_destroy(s_bt_image);

  inverter_layer_destroy(inverter_layer);

  layer_destroy(s_bt_image_layer);
  layer_destroy(s_status_image_layer);
  layer_destroy(s_progress_layer);
}

static void init(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Got into init!");
  // read the settings
  inverted = persist_read_bool(INVERTED_SETTINGS_KEY);

  window = window_create();

  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;

  window_stack_push(window, animated);

  // Update time immediately to avoid flash of "timeless" clock
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_time(t);

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // start the mailbox
  app_message_open(64, 64);

  bluetooth_connection_service_subscribe(bluetooth_callback);
}

static void deinit(void) {
  window_destroy(window);
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
