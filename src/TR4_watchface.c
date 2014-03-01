/*

Display an image of my TR-4

 */

#include "pebble.h"

static Window *window;

static BitmapLayer *image_layer;
static TextLayer *time_layer; 
static TextLayer *battery_layer;

static GBitmap *image;

static int show_seconds = 0;

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "chg");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(battery_layer, battery_text);
}

// Called once per second, or per minute depending on value of show_seconds
static void handle_clock_tick(struct tm* tick_time, TimeUnits units_changed) {
  static char time_text[] = "00:00:00"; // Needs to be static because it's used by the system later.

  if ( show_seconds )
    strftime(time_text, sizeof(time_text), "%T", tick_time);
  else
    strftime(time_text, sizeof(time_text), "%R", tick_time);
  text_layer_set_text(time_layer, time_text);

  handle_battery(battery_state_service_peek());
}

// Handle the start-up of the app
static void do_init(void) {

  // Create our app's base window
  window = window_create();
  window_stack_push(window, true);
  /* window_set_background_color(window, GColorBlack); */

  Layer *root_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(root_layer);

  // This needs to be deinited on app exit which is when the event loop ends
  image = gbitmap_create_with_resource(RESOURCE_ID_GRANTS_TR4);

  // The bitmap layer holds the image for display
  image_layer = bitmap_layer_create(frame);
  bitmap_layer_set_bitmap(image_layer, image);
  bitmap_layer_set_alignment(image_layer, GAlignCenter);
  layer_add_child(root_layer, bitmap_layer_get_layer(image_layer));
  
  // Init the text layer used to show the time
  time_layer = text_layer_create(GRect(65, 10, frame.size.w - 65 /* width */, 34/* height */));
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);

  // Init the text layer used to show the battery
  battery_layer = text_layer_create(GRect(105, 40, /* width */ frame.size.w - 105, 34 /* height */));
  text_layer_set_text_color(battery_layer, GColorWhite);
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentCenter);
  text_layer_set_text(battery_layer, "100%");

  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_clock_tick(current_time, SECOND_UNIT);

  // Set up the timer service
  if ( show_seconds )
    tick_timer_service_subscribe(SECOND_UNIT, &handle_clock_tick);
  else
    tick_timer_service_subscribe(MINUTE_UNIT, &handle_clock_tick);
  battery_state_service_subscribe(&handle_battery);

  layer_add_child(root_layer, text_layer_get_layer(time_layer));
  layer_add_child(root_layer, text_layer_get_layer(battery_layer));
}

static void do_deinit(void) {
  /* Unsubscribe from services */
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  
  /* Destroy the layers */
  text_layer_destroy(time_layer);
  text_layer_destroy(battery_layer);
  
  /* Destroy bitmap and bitmap layer */
  gbitmap_destroy(image);
  bitmap_layer_destroy(image_layer);

  /* Destroy the Window */
  window_destroy(window);
}

// The main event/run loop for our app
int main(void) {
  do_init();
  app_event_loop();
  do_deinit();
  return( 0 );
}
