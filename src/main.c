#include <pebble.h>

Window *my_window;

// For displaying the time
char time_string[] = "00:00 ";
TextLayer *time_text_layer = NULL;

// For displaying the image
BitmapLayer *bitmap_layer = NULL;
GBitmap *gbitmap_ptr = NULL;

// For displaying when ISS is overhead
AppTimer *timer_rise_time = NULL;
AppTimer *timer_overhead = NULL;

int is_overhead = 0;

#define APP_MESSAGE_TIME_TO_PASS 0
#define APP_MESSAGE_DURATION 1

void update_time() {
  clock_copy_time_string(time_string,sizeof(time_string));
  
  if (is_overhead) { 
    time_string[5] = '.';
  } else {
    time_string[5] = ' ';
  }
  
  text_layer_set_text(time_text_layer, time_string);
  layer_mark_dirty(text_layer_get_layer(time_text_layer));
}

void do_overhead(void* context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Overhead");
  is_overhead = true;
  update_time();
  
  vibes_long_pulse();
}

void done_overhead(void* context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Done Overhead!");
  is_overhead = false;
  update_time();
  
  vibes_short_pulse();
}

void handle_app_message(DictionaryIterator *msg, void *context) {
  Tuple *t_time_to_rise = dict_find(msg, APP_MESSAGE_TIME_TO_PASS);
  Tuple *t_duration = dict_find(msg, APP_MESSAGE_DURATION);

  // If we're missing information
  if (!t_time_to_rise || !t_duration) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Missing Information");
    return;
  }
  
  // Cancel timers if they exist
  if(timer_rise_time) app_timer_cancel(timer_rise_time);
  if(timer_overhead) app_timer_cancel(timer_overhead);

  // Extract values
  int16_t time_to_rise = t_time_to_rise->value->int16 * 1000;
  int16_t duration = t_duration->value->int16 * 1000;
  
  APP_LOG(APP_LOG_LEVEL_INFO, "%d, %d", time_to_rise, duration);
  
  // schedule overhead timer
  timer_rise_time = app_timer_register(time_to_rise, do_overhead, NULL);
  timer_overhead = app_timer_register(time_to_rise+duration, done_overhead, NULL);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time();
}

void load_image_resource(uint32_t resource_id){
  if (gbitmap_ptr) {
    gbitmap_destroy(gbitmap_ptr);
    gbitmap_ptr = NULL;
  }
  bitmap_layer_set_compositing_mode(bitmap_layer, GCompOpSet);
  gbitmap_ptr = gbitmap_create_with_resource(resource_id);
  layer_mark_dirty(bitmap_layer_get_layer(bitmap_layer));
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Load the image and fill the screen with it
  bitmap_layer = bitmap_layer_create(bounds);
  load_image_resource(RESOURCE_ID_IMAGE_ISS);
  bitmap_layer_set_bitmap(bitmap_layer, gbitmap_ptr);
  
  // Set the background color
  window_set_background_color(window, PBL_IF_COLOR_ELSE(GColorRichBrilliantLavender, GColorWhite));
  
  //Add background first
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));
  
  //Setup the time display
  time_text_layer = text_layer_create(GRect(0, 20, bounds.size.w, 40));
  text_layer_set_background_color(time_text_layer, GColorClear);
	text_layer_set_font(time_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(time_text_layer, GTextAlignmentCenter);

  update_time();
  
  //Add clock text second
  layer_add_child(window_layer, text_layer_get_layer(time_text_layer));
  
  //Setup hour and minute handlers
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Setup the app messages
  app_message_register_inbox_received(handle_app_message);
  // (inbox, outbox)
  app_message_open(64, 0);
}

void window_unload(Window *window) {
  text_layer_destroy(time_text_layer);
}

void handle_init(void) {
  my_window = window_create();
  window_set_window_handlers(my_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  window_stack_push(my_window, false/*animated*/);
}

void handle_deinit(void) {
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
