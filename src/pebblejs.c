#include <pebble.h>

Window *window;
TextLayer *text_date_layer;
TextLayer *text_time_layer;
Layer *line_grid_layer;
int CellNum;

#define CELLNUM_KEY 0
#define CELLNUM_DEFAULT 2

void out_sent_handler(DictionaryIterator *sent, void *context) {
 // outgoing message was delivered
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Success to send message");
}


void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
 // outgoing message failed
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Failed to send message");
}


void in_received_handler(DictionaryIterator *received, void *context) {
 // incoming message received
  Tuple *int_tuple = dict_find(received, 0);

  if (int_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Message received: %d", int_tuple->value->uint8);
  }
}


void in_dropped_handler(AppMessageResult reason, void *context) {
 // incoming message dropped
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Failed to receive message");
}

void line_grid_layer_update_callback(Layer *layer, GContext* ctx) {
  int rows;
  int cols;
  int w = 144;
  int h = 168;
  int i = 0;
  int j = 0;
  int incr, incc;

  switch(CellNum) {
    case 2:
      rows = 2;
      cols = 1;
      break;
    case 4:
      rows = 2;
      cols = 2;
      break;
    case 6:
      rows = 3;
      cols = 2;
      break;
    case 9:
      rows = 3;
      cols = 3;
    default:
      rows = 2;
      cols = 1;
  }

  graphics_context_set_stroke_color(ctx, GColorWhite);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  //graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  incr = h/rows;
  incc = w/cols;

  for ( i=1; i<rows; i++ ) {
    graphics_draw_line(ctx, GPoint(0,i*incr), GPoint(w,i*incr));
  }

  for ( j=1; j<cols; j++) {
    graphics_draw_line(ctx, GPoint(j*incc, 0), GPoint(j*incc, h));
  }
  
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  static char date_text[] = "Xxxxxxxxx 00";

  char *time_format;


  // TODO: Only update the date when it's changed.
  strftime(date_text, sizeof(date_text), "%B %e", tick_time);
  text_layer_set_text(text_date_layer, date_text);


  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  text_layer_set_text(text_time_layer, time_text);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();
}

void handle_init(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);

  const uint32_t inbound_size = 64;
  const uint32_t outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  Layer *window_layer = window_get_root_layer(window);

  /*text_date_layer = text_layer_create(GRect(8, 68, 144-8, 168-68));
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_font(text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21)));
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));

  text_time_layer = text_layer_create(GRect(7, 92, 144-7, 168-92));
  text_layer_set_text_color(text_time_layer, GColorWhite);
  text_layer_set_background_color(text_time_layer, GColorClear);
  text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49)));
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));*/
  CellNum = persist_exists(CELLNUM_KEY) ? persist_read_int(CELLNUM_KEY) : CELLNUM_DEFAULT;

  GRect bounds = layer_get_bounds(window_layer);
  line_grid_layer = layer_create(bounds);
  layer_set_update_proc(line_grid_layer, line_grid_layer_update_callback);
  layer_add_child(window_layer, line_grid_layer);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  // TODO: Update display here to avoid blank display on launch?
}


int main(void) {
  handle_init();

  app_event_loop();
  
  handle_deinit();
}
