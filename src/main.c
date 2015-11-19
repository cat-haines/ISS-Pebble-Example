#include <pebble.h>

#define AppMessage_FetchData 0
#define AppMessage_NextPass 1

Window *my_window;
TextLayer *text_layer;

static char nextPass[32];

static void send_message() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (!iter) {
    // Error creating outbound message
    text_layer_set_text(text_layer, "Something went wrong!");
    return;
  }

  text_layer_set_text(text_layer, "Fetching data..");
  
  dict_write_int8(iter, AppMessage_FetchData, 1);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *reply_tuple = dict_find(iter, AppMessage_NextPass);
  
  if (reply_tuple) {
    snprintf(nextPass, 32, "ISS Overhead in: %s", reply_tuple->value->cstring);
    
    text_layer_set_text(text_layer, nextPass);
  } else {
    text_layer_set_text(text_layer, "Error :(");
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Up pressed!");
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  send_message();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Down pressed!");
}

static void click_config_provider(void *context) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

void window_load(Window *window) {
  Layer* root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);
  
  text_layer = text_layer_create((GRect) { 
    .size = { bounds.size.w, 20 },
    .origin = { 0, 72 }
  });

  text_layer_set_text(text_layer, "Press a button!");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  
  layer_add_child(root_layer, text_layer_get_layer(text_layer));
}

void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

void handle_init(void) {
  my_window = window_create();

  window_set_window_handlers(my_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  
  app_message_register_inbox_received(inbox_received_handler);
  
  app_message_open(128, 128);  
  
  window_set_click_config_provider(my_window, click_config_provider);
  window_stack_push(my_window, true);  // true means we want there to be an animation
}

void handle_deinit(void) {
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
