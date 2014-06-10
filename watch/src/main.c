#include <pebble.h>

static Window *window;
static SimpleMenuLayer* menuLayer;

typedef struct TGcString
{
	char* String;
	struct TGcString* Next;
} TGcString;

typedef struct
{
	char Departure[10];
	char Arrival[10];
	char Route[10];
} TTrain;

typedef struct
{
	TTrain Trains[10];
	int NumTrains;
	const char* StationName;
} TTimetable;

static TTimetable Timetable = {
		.Trains = {{
				.Departure = "10:00",
				.Arrival = "11:00",
				.Route = "1234"
			},
			{
				.Departure = "12:14",
				.Arrival = "13:07",
				.Route = "3456"
			},
			{
				.Departure = "15:25",
				.Arrival = "16:03",
				.Route = "4567"
			},
			{
				.Departure = "21:01",
				.Arrival = "22:11",
				.Route = "6789"
			}},
		.NumTrains = 4,
		.StationName = "Екатеринбург-Пассажирский"
};

static SimpleMenuSection* StationMenu = NULL;
static TGcString* StationMenuStrings = NULL;

char* AllocGcString(TGcString** where, size_t len)
{
	TGcString* next = NULL;

	if (*where != NULL) {
		next = (*where)->Next;
	}
	*where = malloc(sizeof(TGcString));
	(*where)->Next = next;
	char* result = malloc(len+1);
	(*where)->String = result;
	return result;
}

void DestroyGcStrings(TGcString* which)
{
	TGcString* curr = which;
	TGcString* next;
	while (curr != NULL) {
		next = curr->Next;
		free(curr->String);
		free(curr);
		curr = next;
	}
}

static void GenerateStationMenu(TTimetable* timetable)
{
	int i;

	StationMenu = malloc(sizeof(SimpleMenuSection));
	memset(StationMenu, 0, sizeof(SimpleMenuSection));
	StationMenu->title = timetable->StationName;

	StationMenu->num_items = timetable->NumTrains;
	SimpleMenuItem* items = malloc(sizeof(SimpleMenuItem)*timetable->NumTrains);
	for (i = 0; i < timetable->NumTrains; ++i) {
		SimpleMenuItem* item = &items[i];
		const TTrain* train = &timetable->Trains[i];

		const char* titleFormat = "%s - %s";
		size_t titleLen = strlen(train->Departure) + strlen(train->Arrival) + strlen(titleFormat);
		char* title = AllocGcString(&StationMenuStrings, titleLen);
		snprintf(title, titleLen+1, titleFormat, train->Departure, train->Arrival);

		static const char* subtitleFormat = "поезд %s";
		size_t subtitleLen = strlen(train->Route) + strlen(subtitleFormat);
		char* subtitle = AllocGcString(&StationMenuStrings, subtitleLen);
		snprintf(subtitle, subtitleLen+1, subtitleFormat, train->Route);

		item->callback = NULL;
		item->icon = NULL;
		item->title = title;
		item->subtitle = subtitle;
		//item->subtitle="12345678901234567890";
		//item->title="12345678901234567890";
	}
	StationMenu->items = items;
}

static void DestroyStationMenu()
{
	free((SimpleMenuItem*)(StationMenu->items));
	free(StationMenu);
	DestroyGcStrings(StationMenuStrings);
	StationMenu = NULL;
	StationMenuStrings = NULL;
}

static void window_load(Window *window) {
  Layer *windowLayer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(windowLayer);
  GenerateStationMenu(&Timetable);
  menuLayer = simple_menu_layer_create(bounds, window, StationMenu, 1, NULL);
  layer_add_child(windowLayer, simple_menu_layer_get_layer(menuLayer));
}

static void window_unload(Window *window) {
  simple_menu_layer_destroy(menuLayer);
  DestroyStationMenu();
}

static void init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
	    .unload = window_unload,
	});
	window_stack_push(window, true /* Animated */);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
