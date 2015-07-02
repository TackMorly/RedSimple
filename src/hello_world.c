#include <pebble.h>
	
Window *window;
TextLayer *text_layer;
TextLayer *text_layer_sec;
TextLayer *battery_layer;
TextLayer *date_layer;
BitmapLayer *bitmap_layer;
static GBitmap *s_png_bitmap;

// GPath
Layer *path_layer;
GPath *s_my_path_ptr;
static const GPathInfo BOLT_PATH_INFO = {
  .num_points = 4,
  .points = (GPoint []) {{10, 50}, {30, 50}, {30, 55}, {10, 55}}
};
static int xPoint = 0;

// .update_proc of my_layer:
void my_layer_update_proc(Layer *my_layer, GContext* ctx) {
  // Fill the path:
  graphics_context_set_fill_color(ctx, GColorWhite);
  gpath_draw_filled(ctx, s_my_path_ptr);
  // Stroke the path:
  //graphics_context_set_stroke_color(ctx, GColorBlack);
  //gpath_draw_outline(ctx, s_my_path_ptr);
}

void handle_timechanges(struct tm *tick_time, TimeUnits units_changed){
	static char time_buffer[8];
	static char time_buffer_sec[4];
	static char date_buffer[10];
	
	// H:M:S
	strftime(time_buffer, sizeof(time_buffer), "%H:%M", tick_time);
	strftime(time_buffer_sec, sizeof(time_buffer_sec), "%S", tick_time);
	text_layer_set_text(text_layer,time_buffer);
	text_layer_set_text(text_layer_sec,time_buffer_sec);
	
	// Date
	strftime(date_buffer, sizeof(date_buffer), "%b %e", tick_time);
	text_layer_set_text(date_layer,date_buffer);
	
	layer_mark_dirty(path_layer);
		
}

static void battery_handler(BatteryChargeState new_state) {
  // Write to buffer and display
  static char s_battery_buffer[32];
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", new_state.charge_percent);
  text_layer_set_text(battery_layer, s_battery_buffer);
}

static void bt_handler(bool connected) {
  // Show current connection state
  if (connected) {
		layer_set_hidden((Layer *)bitmap_layer, false);
  } else {
    layer_set_hidden((Layer *)bitmap_layer, true);
  }
}

void window_load(Window *window) {
	
		// Add png image
	bitmap_layer = bitmap_layer_create(GRect(80, 3, 100, 32));
	s_png_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
	bitmap_layer_set_bitmap(bitmap_layer,s_png_bitmap);
	bitmap_layer_set_background_color(bitmap_layer, GColorFolly);	
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(bitmap_layer));
	
	// Set the text, font, and text alignment
	text_layer = text_layer_create(GRect(3, 0, 60, 32));
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_text_color(text_layer, GColorWhite);
	text_layer_set_background_color(text_layer, GColorClear);

	text_layer_sec = text_layer_create(GRect(60, 14, 80, 32));
	text_layer_set_font(text_layer_sec, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(text_layer_sec, GTextAlignmentLeft);
	text_layer_set_text_color(text_layer_sec, GColorWhite);
	text_layer_set_background_color(text_layer_sec, GColorClear);
	
	//battery
	battery_layer = text_layer_create(GRect(65, 14, 85, 32));
	text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(battery_layer, GTextAlignmentCenter);
	text_layer_set_text_color(battery_layer, GColorWhite);
	text_layer_set_background_color(battery_layer, GColorClear);
	
	//path
	path_layer = layer_create(GRect(0, 32, 144, 64));
	s_my_path_ptr = gpath_create(&BOLT_PATH_INFO);
		
	//date layer
	date_layer = text_layer_create(GRect(0, 112, 144, 156));
	text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
	text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_background_color(date_layer, GColorClear);
	
	// Add the text layer to the window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer_sec));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(battery_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
	
	
	// Get the current battery level
  battery_handler(battery_state_service_peek());
	

	
	
	//update gpath
	GPoint tmp = GPoint(xPoint, 50);
	gpath_move_to(s_my_path_ptr, tmp);
	xPoint++;
	if(xPoint > 60){
		xPoint = 0;
	}
	
	layer_set_update_proc(path_layer, my_layer_update_proc);
	layer_add_child(window_get_root_layer(window), path_layer);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "%d",xPoint);
}

void window_unload(Window *window) {
	// Destroy the text layer
	text_layer_destroy(text_layer);
	text_layer_destroy(text_layer_sec);
	text_layer_destroy(battery_layer);
	text_layer_destroy(date_layer);
}

void handle_init(void) {
	// Create a window and text layer
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

	// set background color
	window_set_background_color(window, GColorFolly);
	
	//set listener
	tick_timer_service_subscribe(SECOND_UNIT, handle_timechanges);
	
	// Subscribe to the Battery State Service
  battery_state_service_subscribe(battery_handler);
	
	// Subscribe to Bluetooth updates
  bluetooth_connection_service_subscribe(bt_handler);
		
	// Push the window
	window_stack_push(window, true);
	
	// App Logging!
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
}



void handle_deinit(void) {

	gpath_destroy(s_my_path_ptr);
	
	// Destroy the window
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
