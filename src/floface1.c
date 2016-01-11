#include <pebble.h>

static Window *window;
static TextLayer *s_time_layer_hour;
static TextLayer *s_time_layer_min;
static TextLayer *s_time_layer_sec;
static TextLayer *s_date_layer_dayname;
static TextLayer *s_date_layer_daynum;
static GFont s_time_font;
static GFont s_date_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static int s_battery_level;
static Layer *s_battery_layer;

static void update_time_hourmin() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    
    // Write the current hours and minutes into a buffer
    static char s_buffer_hour[4];
    static char s_buffer_min[4];
    strftime(s_buffer_hour, sizeof(s_buffer_hour), clock_is_24h_style() ?
    "%H" : "%I", tick_time);
    strftime(s_buffer_min, sizeof(s_buffer_min), "%M", tick_time);
    
    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer_hour, s_buffer_hour);
    text_layer_set_text(s_time_layer_min, s_buffer_min);
    
    static char s_buffer_daynum[4];
    static char s_buffer_dayname[4];
    static char s_buffer_dayname_alt[2];
    strftime(s_buffer_dayname, sizeof(s_buffer_dayname), "%a", tick_time);
    strftime(s_buffer_daynum, sizeof(s_buffer_daynum), "%e", tick_time);
    s_buffer_dayname_alt[0]=s_buffer_dayname[0];
    s_buffer_dayname_alt[1]=s_buffer_dayname[1];

    text_layer_set_text(s_date_layer_dayname, s_buffer_dayname_alt);
    text_layer_set_text(s_date_layer_daynum, s_buffer_daynum);
}

static void update_time_sec() {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    
    static char s_buffer_sec[4];
    strftime(s_buffer_sec, sizeof(s_buffer_sec), "%S", tick_time);
    
    text_layer_set_text(s_time_layer_sec, s_buffer_sec);
}

static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  //layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

  if(!connected) {
    // Issue a vibrating alert
    vibes_long_pulse();
  }
}

static void tick_handler_hourmin(struct tm *tick_time, TimeUnits units_changed) {
    update_time_hourmin();
}
static void tick_handler_sec(struct tm *tick_time, TimeUnits units_changed) {
    update_time_hourmin();
    update_time_sec();
}

static void battery_callback(BatteryChargeState state) {
    // Record the new battery level
    s_battery_level = state.charge_percent;
    // Update meter
    layer_mark_dirty(s_battery_layer);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);

    // Find the height of the bar
    int height = (int)(float)(((float)s_battery_level / 100.0F) * 168.0F);

    // Draw the background
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    // Draw the bar
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(0, bounds.size.h-height, bounds.size.w, height), 0, GCornerNone);
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    // Create GBitmap
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
    
    // Create BitmapLayer to display the GBitmap
    s_background_layer = bitmap_layer_create(bounds);
    
    // Set the bitmap onto the layer and add to the window
    bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
    //layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
    
    // Create the TextLayer with specific bounds
    s_time_layer_hour = text_layer_create(
    GRect(52, -10, 95, 60));
    s_time_layer_min = text_layer_create(
    GRect(52, 45, 95, 60));
    s_time_layer_sec = text_layer_create(
    GRect(52, 105, 95, 63));
    s_date_layer_dayname = text_layer_create(
    GRect(6, 54, 40, 25));
//    GRect(6, 3, 40, 25));
    s_date_layer_daynum = text_layer_create(
    GRect(6, 78, 42, 25));
//    GRect(6, 28, 40, 25));
    
    // Improve the layout to be more like a watchface
    text_layer_set_background_color(s_time_layer_hour, GColorBlack);
    text_layer_set_text_color(s_time_layer_hour, GColorWhite);
    text_layer_set_background_color(s_time_layer_min, GColorBlack);
    text_layer_set_text_color(s_time_layer_min, GColorWhite);
    text_layer_set_background_color(s_time_layer_sec, GColorBlack);
    text_layer_set_text_color(s_time_layer_sec, GColorWhite);
    //text_layer_set_background_color(s_date_layer_daynum, GColorBlack);
    //text_layer_set_text_color(s_date_layer_daynum, GColorWhite);
    
    // Create GFont
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_JOYSTIX_56));
    s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_JOYSTIX_24));
    
    // Apply to TextLayer
    text_layer_set_font(s_time_layer_hour, s_time_font);
    text_layer_set_font(s_time_layer_min, s_time_font);
    text_layer_set_font(s_time_layer_sec, s_time_font);
    text_layer_set_font(s_time_layer_sec, s_time_font);
    text_layer_set_font(s_date_layer_dayname, s_date_font);
    text_layer_set_font(s_date_layer_daynum, s_date_font);
    text_layer_set_text_alignment(s_time_layer_hour, GTextAlignmentRight);
    text_layer_set_text_alignment(s_time_layer_min, GTextAlignmentRight);
    text_layer_set_text_alignment(s_time_layer_sec, GTextAlignmentRight);
    text_layer_set_text_alignment(s_date_layer_dayname, GTextAlignmentLeft);
    text_layer_set_text_alignment(s_date_layer_daynum, GTextAlignmentLeft);
    
    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer_hour));
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer_min));
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer_sec));
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer_daynum));
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer_dayname));


    // Create battery meter Layer
    s_battery_layer = layer_create(GRect(50, 0, 2, 168));
    layer_set_update_proc(s_battery_layer, battery_update_proc);

    // Add to Window
    layer_add_child(window_get_root_layer(window), s_battery_layer);
}

static void window_unload(Window *window) {
    text_layer_destroy(s_time_layer_hour);
    text_layer_destroy(s_time_layer_min);
    text_layer_destroy(s_time_layer_sec);
    text_layer_destroy(s_date_layer_dayname);
    text_layer_destroy(s_date_layer_daynum);
    // Destroy GBitmap
    gbitmap_destroy(s_background_bitmap);
    
    // Destroy BitmapLayer
    bitmap_layer_destroy(s_background_layer);
    // Unload GFont
    fonts_unload_custom_font(s_time_font);
    fonts_unload_custom_font(s_date_font);
    
    layer_destroy(s_battery_layer);

}

static void init(void) {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    const bool animated = true;
    window_stack_push(window, animated);
    update_time_hourmin();
    update_time_sec();
    // Register with TickTimerService
    //tick_timer_service_subscribe(MINUTE_UNIT, tick_handler_hourmin);
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler_sec);
    connection_service_subscribe((ConnectionHandlers) {
      .pebble_app_connection_handler = bluetooth_callback
    });
    battery_state_service_subscribe(battery_callback);
    // Ensure battery level is displayed from the start
    battery_callback(battery_state_service_peek());
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
