// main.src
#include <snek.h>


typedef struct{string* data;u64 length;}arr_0_string;
typedef u32 Uint32;
typedef Uint32 SDL_InitFlags;
struct SDL_Window;
struct SDL_Renderer;
typedef u64 Uint64;
typedef Uint64 SDL_WindowFlags;
typedef u8 Uint8;
struct SDL_CommonEvent{
	Uint32 type;
	Uint32 reserved;
	Uint64 timestamp;
};
enum SDL_EventType:i32{
	SDL_EVENT_FIRST=0,
	SDL_EVENT_QUIT=0x100,
	SDL_EVENT_TERMINATING,
	SDL_EVENT_LOW_MEMORY,
	SDL_EVENT_WILL_ENTER_BACKGROUND,
	SDL_EVENT_DID_ENTER_BACKGROUND,
	SDL_EVENT_WILL_ENTER_FOREGROUND,
	SDL_EVENT_DID_ENTER_FOREGROUND,
	SDL_EVENT_LOCALE_CHANGED,
	SDL_EVENT_SYSTEM_THEME_CHANGED,
	SDL_EVENT_DISPLAY_ORIENTATION=0x151,
	SDL_EVENT_DISPLAY_ADDED,
	SDL_EVENT_DISPLAY_REMOVED,
	SDL_EVENT_DISPLAY_MOVED,
	SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED,
	SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED,
	SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED,
	SDL_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED,
	SDL_EVENT_DISPLAY_FIRST,
	SDL_EVENT_DISPLAY_LAST,
	SDL_EVENT_WINDOW_SHOWN=0x202,
	SDL_EVENT_WINDOW_HIDDEN,
	SDL_EVENT_WINDOW_EXPOSED,
	SDL_EVENT_WINDOW_MOVED,
	SDL_EVENT_WINDOW_RESIZED,
	SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED,
	SDL_EVENT_WINDOW_METAL_VIEW_RESIZED,
	SDL_EVENT_WINDOW_MINIMIZED,
	SDL_EVENT_WINDOW_MAXIMIZED,
	SDL_EVENT_WINDOW_RESTORED,
	SDL_EVENT_WINDOW_MOUSE_ENTER,
	SDL_EVENT_WINDOW_MOUSE_LEAVE,
	SDL_EVENT_WINDOW_FOCUS_GAINED,
	SDL_EVENT_WINDOW_FOCUS_LOST,
	SDL_EVENT_WINDOW_CLOSE_REQUESTED,
	SDL_EVENT_WINDOW_HIT_TEST,
	SDL_EVENT_WINDOW_ICCPROF_CHANGED,
	SDL_EVENT_WINDOW_DISPLAY_CHANGED,
	SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED,
	SDL_EVENT_WINDOW_SAFE_AREA_CHANGED,
	SDL_EVENT_WINDOW_OCCLUDED,
	SDL_EVENT_WINDOW_ENTER_FULLSCREEN,
	SDL_EVENT_WINDOW_LEAVE_FULLSCREEN,
	SDL_EVENT_WINDOW_DESTROYED,
	SDL_EVENT_WINDOW_HDR_STATE_CHANGED,
	SDL_EVENT_WINDOW_SETTINGS_CHANGED,
	SDL_EVENT_WINDOW_FIRST,
	SDL_EVENT_WINDOW_LAST,
	SDL_EVENT_KEY_DOWN=0x300,
	SDL_EVENT_KEY_UP,
	SDL_EVENT_TEXT_EDITING,
	SDL_EVENT_TEXT_INPUT,
	SDL_EVENT_KEYMAP_CHANGED,
	SDL_EVENT_KEYBOARD_ADDED,
	SDL_EVENT_KEYBOARD_REMOVED,
	SDL_EVENT_TEXT_EDITING_CANDIDATES,
	SDL_EVENT_SCREEN_KEYBOARD_SHOWN,
	SDL_EVENT_SCREEN_KEYBOARD_HIDDEN,
	SDL_EVENT_MOUSE_MOTION=0x400,
	SDL_EVENT_MOUSE_BUTTON_DOWN,
	SDL_EVENT_MOUSE_BUTTON_UP,
	SDL_EVENT_MOUSE_WHEEL,
	SDL_EVENT_MOUSE_ADDED,
	SDL_EVENT_MOUSE_REMOVED,
	SDL_EVENT_JOYSTICK_AXIS_MOTION=0x600,
	SDL_EVENT_JOYSTICK_BALL_MOTION,
	SDL_EVENT_JOYSTICK_HAT_MOTION,
	SDL_EVENT_JOYSTICK_BUTTON_DOWN,
	SDL_EVENT_JOYSTICK_BUTTON_UP,
	SDL_EVENT_JOYSTICK_ADDED,
	SDL_EVENT_JOYSTICK_REMOVED,
	SDL_EVENT_JOYSTICK_BATTERY_UPDATED,
	SDL_EVENT_JOYSTICK_UPDATE_COMPLETE,
	SDL_EVENT_GAMEPAD_AXIS_MOTION=0x650,
	SDL_EVENT_GAMEPAD_BUTTON_DOWN,
	SDL_EVENT_GAMEPAD_BUTTON_UP,
	SDL_EVENT_GAMEPAD_ADDED,
	SDL_EVENT_GAMEPAD_REMOVED,
	SDL_EVENT_GAMEPAD_REMAPPED,
	SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN,
	SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION,
	SDL_EVENT_GAMEPAD_TOUCHPAD_UP,
	SDL_EVENT_GAMEPAD_SENSOR_UPDATE,
	SDL_EVENT_GAMEPAD_UPDATE_COMPLETE,
	SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED,
	SDL_EVENT_GAMEPAD_CAPSENSE_TOUCH,
	SDL_EVENT_GAMEPAD_CAPSENSE_RELEASE,
	SDL_EVENT_FINGER_DOWN=0x700,
	SDL_EVENT_FINGER_UP,
	SDL_EVENT_FINGER_MOTION,
	SDL_EVENT_FINGER_CANCELED,
	SDL_EVENT_PINCH_BEGIN=0x710,
	SDL_EVENT_PINCH_UPDATE,
	SDL_EVENT_PINCH_END,
	SDL_EVENT_CLIPBOARD_UPDATE=0x900,
	SDL_EVENT_DROP_FILE=0x1000,
	SDL_EVENT_DROP_TEXT,
	SDL_EVENT_DROP_BEGIN,
	SDL_EVENT_DROP_COMPLETE,
	SDL_EVENT_DROP_POSITION,
	SDL_EVENT_AUDIO_DEVICE_ADDED=0x1100,
	SDL_EVENT_AUDIO_DEVICE_REMOVED,
	SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED,
	SDL_EVENT_SENSOR_UPDATE=0x1200,
	SDL_EVENT_PEN_PROXIMITY_IN=0x1300,
	SDL_EVENT_PEN_PROXIMITY_OUT,
	SDL_EVENT_PEN_DOWN,
	SDL_EVENT_PEN_UP,
	SDL_EVENT_PEN_BUTTON_DOWN,
	SDL_EVENT_PEN_BUTTON_UP,
	SDL_EVENT_PEN_MOTION,
	SDL_EVENT_PEN_AXIS,
	SDL_EVENT_CAMERA_DEVICE_ADDED=0x1400,
	SDL_EVENT_CAMERA_DEVICE_REMOVED,
	SDL_EVENT_CAMERA_DEVICE_APPROVED,
	SDL_EVENT_CAMERA_DEVICE_DENIED,
	SDL_EVENT_RENDER_TARGETS_RESET=0x2000,
	SDL_EVENT_RENDER_DEVICE_RESET,
	SDL_EVENT_RENDER_DEVICE_LOST,
	SDL_EVENT_PRIVATE0=0x4000,
	SDL_EVENT_PRIVATE1,
	SDL_EVENT_PRIVATE2,
	SDL_EVENT_PRIVATE3,
	SDL_EVENT_POLL_SENTINEL=0x7f00,
	SDL_EVENT_USER=0x8000,
	SDL_EVENT_LAST=0xffff,
	SDL_EVENT_ENUM_PADDING=0x7fffffff,
};
typedef Uint32 SDL_DisplayID;
typedef i32 Sint32;
struct SDL_DisplayEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_DisplayID displayID;
	Sint32 data1;
	Sint32 data2;
};
typedef Uint32 SDL_WindowID;
struct SDL_WindowEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	Sint32 data1;
	Sint32 data2;
};
typedef Uint32 SDL_KeyboardID;
struct SDL_KeyboardDeviceEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_KeyboardID which;
};
enum SDL_Scancode:i32{
	SDL_SCANCODE_UNKNOWN=0,
	SDL_SCANCODE_A=4,
	SDL_SCANCODE_B=5,
	SDL_SCANCODE_C=6,
	SDL_SCANCODE_D=7,
	SDL_SCANCODE_E=8,
	SDL_SCANCODE_F=9,
	SDL_SCANCODE_G=10,
	SDL_SCANCODE_H=11,
	SDL_SCANCODE_I=12,
	SDL_SCANCODE_J=13,
	SDL_SCANCODE_K=14,
	SDL_SCANCODE_L=15,
	SDL_SCANCODE_M=16,
	SDL_SCANCODE_N=17,
	SDL_SCANCODE_O=18,
	SDL_SCANCODE_P=19,
	SDL_SCANCODE_Q=20,
	SDL_SCANCODE_R=21,
	SDL_SCANCODE_S=22,
	SDL_SCANCODE_T=23,
	SDL_SCANCODE_U=24,
	SDL_SCANCODE_V=25,
	SDL_SCANCODE_W=26,
	SDL_SCANCODE_X=27,
	SDL_SCANCODE_Y=28,
	SDL_SCANCODE_Z=29,
	SDL_SCANCODE_1=30,
	SDL_SCANCODE_2=31,
	SDL_SCANCODE_3=32,
	SDL_SCANCODE_4=33,
	SDL_SCANCODE_5=34,
	SDL_SCANCODE_6=35,
	SDL_SCANCODE_7=36,
	SDL_SCANCODE_8=37,
	SDL_SCANCODE_9=38,
	SDL_SCANCODE_0=39,
	SDL_SCANCODE_RETURN=40,
	SDL_SCANCODE_ESCAPE=41,
	SDL_SCANCODE_BACKSPACE=42,
	SDL_SCANCODE_TAB=43,
	SDL_SCANCODE_SPACE=44,
	SDL_SCANCODE_MINUS=45,
	SDL_SCANCODE_EQUALS=46,
	SDL_SCANCODE_LEFTBRACKET=47,
	SDL_SCANCODE_RIGHTBRACKET=48,
	SDL_SCANCODE_BACKSLASH=49,
	SDL_SCANCODE_NONUSHASH=50,
	SDL_SCANCODE_SEMICOLON=51,
	SDL_SCANCODE_APOSTROPHE=52,
	SDL_SCANCODE_GRAVE=53,
	SDL_SCANCODE_COMMA=54,
	SDL_SCANCODE_PERIOD=55,
	SDL_SCANCODE_SLASH=56,
	SDL_SCANCODE_CAPSLOCK=57,
	SDL_SCANCODE_F1=58,
	SDL_SCANCODE_F2=59,
	SDL_SCANCODE_F3=60,
	SDL_SCANCODE_F4=61,
	SDL_SCANCODE_F5=62,
	SDL_SCANCODE_F6=63,
	SDL_SCANCODE_F7=64,
	SDL_SCANCODE_F8=65,
	SDL_SCANCODE_F9=66,
	SDL_SCANCODE_F10=67,
	SDL_SCANCODE_F11=68,
	SDL_SCANCODE_F12=69,
	SDL_SCANCODE_PRINTSCREEN=70,
	SDL_SCANCODE_SCROLLLOCK=71,
	SDL_SCANCODE_PAUSE=72,
	SDL_SCANCODE_INSERT=73,
	SDL_SCANCODE_HOME=74,
	SDL_SCANCODE_PAGEUP=75,
	SDL_SCANCODE_DELETE=76,
	SDL_SCANCODE_END=77,
	SDL_SCANCODE_PAGEDOWN=78,
	SDL_SCANCODE_RIGHT=79,
	SDL_SCANCODE_LEFT=80,
	SDL_SCANCODE_DOWN=81,
	SDL_SCANCODE_UP=82,
	SDL_SCANCODE_NUMLOCKCLEAR=83,
	SDL_SCANCODE_KP_DIVIDE=84,
	SDL_SCANCODE_KP_MULTIPLY=85,
	SDL_SCANCODE_KP_MINUS=86,
	SDL_SCANCODE_KP_PLUS=87,
	SDL_SCANCODE_KP_ENTER=88,
	SDL_SCANCODE_KP_1=89,
	SDL_SCANCODE_KP_2=90,
	SDL_SCANCODE_KP_3=91,
	SDL_SCANCODE_KP_4=92,
	SDL_SCANCODE_KP_5=93,
	SDL_SCANCODE_KP_6=94,
	SDL_SCANCODE_KP_7=95,
	SDL_SCANCODE_KP_8=96,
	SDL_SCANCODE_KP_9=97,
	SDL_SCANCODE_KP_0=98,
	SDL_SCANCODE_KP_PERIOD=99,
	SDL_SCANCODE_NONUSBACKSLASH=100,
	SDL_SCANCODE_APPLICATION=101,
	SDL_SCANCODE_POWER=102,
	SDL_SCANCODE_KP_EQUALS=103,
	SDL_SCANCODE_F13=104,
	SDL_SCANCODE_F14=105,
	SDL_SCANCODE_F15=106,
	SDL_SCANCODE_F16=107,
	SDL_SCANCODE_F17=108,
	SDL_SCANCODE_F18=109,
	SDL_SCANCODE_F19=110,
	SDL_SCANCODE_F20=111,
	SDL_SCANCODE_F21=112,
	SDL_SCANCODE_F22=113,
	SDL_SCANCODE_F23=114,
	SDL_SCANCODE_F24=115,
	SDL_SCANCODE_EXECUTE=116,
	SDL_SCANCODE_HELP=117,
	SDL_SCANCODE_MENU=118,
	SDL_SCANCODE_SELECT=119,
	SDL_SCANCODE_STOP=120,
	SDL_SCANCODE_AGAIN=121,
	SDL_SCANCODE_UNDO=122,
	SDL_SCANCODE_CUT=123,
	SDL_SCANCODE_COPY=124,
	SDL_SCANCODE_PASTE=125,
	SDL_SCANCODE_FIND=126,
	SDL_SCANCODE_MUTE=127,
	SDL_SCANCODE_VOLUMEUP=128,
	SDL_SCANCODE_VOLUMEDOWN=129,
	SDL_SCANCODE_KP_COMMA=133,
	SDL_SCANCODE_KP_EQUALSAS400=134,
	SDL_SCANCODE_INTERNATIONAL1=135,
	SDL_SCANCODE_INTERNATIONAL2=136,
	SDL_SCANCODE_INTERNATIONAL3=137,
	SDL_SCANCODE_INTERNATIONAL4=138,
	SDL_SCANCODE_INTERNATIONAL5=139,
	SDL_SCANCODE_INTERNATIONAL6=140,
	SDL_SCANCODE_INTERNATIONAL7=141,
	SDL_SCANCODE_INTERNATIONAL8=142,
	SDL_SCANCODE_INTERNATIONAL9=143,
	SDL_SCANCODE_LANG1=144,
	SDL_SCANCODE_LANG2=145,
	SDL_SCANCODE_LANG3=146,
	SDL_SCANCODE_LANG4=147,
	SDL_SCANCODE_LANG5=148,
	SDL_SCANCODE_LANG6=149,
	SDL_SCANCODE_LANG7=150,
	SDL_SCANCODE_LANG8=151,
	SDL_SCANCODE_LANG9=152,
	SDL_SCANCODE_ALTERASE=153,
	SDL_SCANCODE_SYSREQ=154,
	SDL_SCANCODE_CANCEL=155,
	SDL_SCANCODE_CLEAR=156,
	SDL_SCANCODE_PRIOR=157,
	SDL_SCANCODE_RETURN2=158,
	SDL_SCANCODE_SEPARATOR=159,
	SDL_SCANCODE_OUT=160,
	SDL_SCANCODE_OPER=161,
	SDL_SCANCODE_CLEARAGAIN=162,
	SDL_SCANCODE_CRSEL=163,
	SDL_SCANCODE_EXSEL=164,
	SDL_SCANCODE_KP_00=176,
	SDL_SCANCODE_KP_000=177,
	SDL_SCANCODE_THOUSANDSSEPARATOR=178,
	SDL_SCANCODE_DECIMALSEPARATOR=179,
	SDL_SCANCODE_CURRENCYUNIT=180,
	SDL_SCANCODE_CURRENCYSUBUNIT=181,
	SDL_SCANCODE_KP_LEFTPAREN=182,
	SDL_SCANCODE_KP_RIGHTPAREN=183,
	SDL_SCANCODE_KP_LEFTBRACE=184,
	SDL_SCANCODE_KP_RIGHTBRACE=185,
	SDL_SCANCODE_KP_TAB=186,
	SDL_SCANCODE_KP_BACKSPACE=187,
	SDL_SCANCODE_KP_A=188,
	SDL_SCANCODE_KP_B=189,
	SDL_SCANCODE_KP_C=190,
	SDL_SCANCODE_KP_D=191,
	SDL_SCANCODE_KP_E=192,
	SDL_SCANCODE_KP_F=193,
	SDL_SCANCODE_KP_XOR=194,
	SDL_SCANCODE_KP_POWER=195,
	SDL_SCANCODE_KP_PERCENT=196,
	SDL_SCANCODE_KP_LESS=197,
	SDL_SCANCODE_KP_GREATER=198,
	SDL_SCANCODE_KP_AMPERSAND=199,
	SDL_SCANCODE_KP_DBLAMPERSAND=200,
	SDL_SCANCODE_KP_VERTICALBAR=201,
	SDL_SCANCODE_KP_DBLVERTICALBAR=202,
	SDL_SCANCODE_KP_COLON=203,
	SDL_SCANCODE_KP_HASH=204,
	SDL_SCANCODE_KP_SPACE=205,
	SDL_SCANCODE_KP_AT=206,
	SDL_SCANCODE_KP_EXCLAM=207,
	SDL_SCANCODE_KP_MEMSTORE=208,
	SDL_SCANCODE_KP_MEMRECALL=209,
	SDL_SCANCODE_KP_MEMCLEAR=210,
	SDL_SCANCODE_KP_MEMADD=211,
	SDL_SCANCODE_KP_MEMSUBTRACT=212,
	SDL_SCANCODE_KP_MEMMULTIPLY=213,
	SDL_SCANCODE_KP_MEMDIVIDE=214,
	SDL_SCANCODE_KP_PLUSMINUS=215,
	SDL_SCANCODE_KP_CLEAR=216,
	SDL_SCANCODE_KP_CLEARENTRY=217,
	SDL_SCANCODE_KP_BINARY=218,
	SDL_SCANCODE_KP_OCTAL=219,
	SDL_SCANCODE_KP_DECIMAL=220,
	SDL_SCANCODE_KP_HEXADECIMAL=221,
	SDL_SCANCODE_LCTRL=224,
	SDL_SCANCODE_LSHIFT=225,
	SDL_SCANCODE_LALT=226,
	SDL_SCANCODE_LGUI=227,
	SDL_SCANCODE_RCTRL=228,
	SDL_SCANCODE_RSHIFT=229,
	SDL_SCANCODE_RALT=230,
	SDL_SCANCODE_RGUI=231,
	SDL_SCANCODE_MODE=257,
	SDL_SCANCODE_SLEEP=258,
	SDL_SCANCODE_WAKE=259,
	SDL_SCANCODE_CHANNEL_INCREMENT=260,
	SDL_SCANCODE_CHANNEL_DECREMENT=261,
	SDL_SCANCODE_MEDIA_PLAY=262,
	SDL_SCANCODE_MEDIA_PAUSE=263,
	SDL_SCANCODE_MEDIA_RECORD=264,
	SDL_SCANCODE_MEDIA_FAST_FORWARD=265,
	SDL_SCANCODE_MEDIA_REWIND=266,
	SDL_SCANCODE_MEDIA_NEXT_TRACK=267,
	SDL_SCANCODE_MEDIA_PREVIOUS_TRACK=268,
	SDL_SCANCODE_MEDIA_STOP=269,
	SDL_SCANCODE_MEDIA_EJECT=270,
	SDL_SCANCODE_MEDIA_PLAY_PAUSE=271,
	SDL_SCANCODE_MEDIA_SELECT=272,
	SDL_SCANCODE_AC_NEW=273,
	SDL_SCANCODE_AC_OPEN=274,
	SDL_SCANCODE_AC_CLOSE=275,
	SDL_SCANCODE_AC_EXIT=276,
	SDL_SCANCODE_AC_SAVE=277,
	SDL_SCANCODE_AC_PRINT=278,
	SDL_SCANCODE_AC_PROPERTIES=279,
	SDL_SCANCODE_AC_SEARCH=280,
	SDL_SCANCODE_AC_HOME=281,
	SDL_SCANCODE_AC_BACK=282,
	SDL_SCANCODE_AC_FORWARD=283,
	SDL_SCANCODE_AC_STOP=284,
	SDL_SCANCODE_AC_REFRESH=285,
	SDL_SCANCODE_AC_BOOKMARKS=286,
	SDL_SCANCODE_SOFTLEFT=287,
	SDL_SCANCODE_SOFTRIGHT=288,
	SDL_SCANCODE_CALL=289,
	SDL_SCANCODE_ENDCALL=290,
	SDL_SCANCODE_RESERVED=400,
	SDL_SCANCODE_COUNT=512,
};
typedef Uint32 SDL_Keycode;
typedef u16 Uint16;
typedef Uint16 SDL_Keymod;
struct SDL_KeyboardEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_KeyboardID which;
	enum SDL_Scancode scancode;
	SDL_Keycode key;
	SDL_Keymod mod;
	Uint16 raw;
	bool down;
	bool repeat;
};
struct SDL_TextEditingEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	i8* text;
	Sint32 start;
	Sint32 length;
};
struct SDL_TextEditingCandidatesEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	i8** candidates;
	Sint32 num_candidates;
	Sint32 selected_candidate;
	bool horizontal;
	Uint8 padding1;
	Uint8 padding2;
	Uint8 padding3;
};
struct SDL_TextInputEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	i8* text;
};
typedef Uint32 SDL_MouseID;
struct SDL_MouseDeviceEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_MouseID which;
};
typedef Uint32 SDL_MouseButtonFlags;
struct SDL_MouseMotionEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_MouseID which;
	SDL_MouseButtonFlags state;
	float x;
	float y;
	float xrel;
	float yrel;
};
struct SDL_MouseButtonEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_MouseID which;
	Uint8 button;
	bool down;
	Uint8 clicks;
	Uint8 padding;
	float x;
	float y;
};
enum SDL_MouseWheelDirection:i32{
	SDL_MOUSEWHEEL_NORMAL,
	SDL_MOUSEWHEEL_FLIPPED,
};
struct SDL_MouseWheelEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_MouseID which;
	float x;
	float y;
	enum SDL_MouseWheelDirection direction;
	float mouse_x;
	float mouse_y;
	Sint32 integer_x;
	Sint32 integer_y;
};
typedef Uint32 SDL_JoystickID;
struct SDL_JoyDeviceEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
};
typedef i16 Sint16;
struct SDL_JoyAxisEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Uint8 axis;
	Uint8 padding1;
	Uint8 padding2;
	Uint8 padding3;
	Sint16 value;
	Uint16 padding4;
};
struct SDL_JoyBallEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Uint8 ball;
	Uint8 padding1;
	Uint8 padding2;
	Uint8 padding3;
	Sint16 xrel;
	Sint16 yrel;
};
struct SDL_JoyHatEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Uint8 hat;
	Uint8 value;
	Uint8 padding1;
	Uint8 padding2;
};
struct SDL_JoyButtonEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Uint8 button;
	bool down;
	Uint8 padding1;
	Uint8 padding2;
};
enum SDL_PowerState:i32{
	SDL_POWERSTATE_ERROR=-1,
	SDL_POWERSTATE_UNKNOWN,
	SDL_POWERSTATE_ON_BATTERY,
	SDL_POWERSTATE_NO_BATTERY,
	SDL_POWERSTATE_CHARGING,
	SDL_POWERSTATE_CHARGED,
};
struct SDL_JoyBatteryEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	enum SDL_PowerState state;
	i32 percent;
};
struct SDL_GamepadDeviceEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
};
struct SDL_GamepadAxisEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Uint8 axis;
	Uint8 padding1;
	Uint8 padding2;
	Uint8 padding3;
	Sint16 value;
	Uint16 padding4;
};
struct SDL_GamepadButtonEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Uint8 button;
	bool down;
	Uint8 padding1;
	Uint8 padding2;
};
struct SDL_GamepadTouchpadEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Sint32 touchpad;
	Sint32 finger;
	float x;
	float y;
	float pressure;
};
typedef struct{float data[3];}arr_3_float;
struct SDL_GamepadSensorEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Sint32 sensor;
	arr_3_float data;
	Uint64 sensor_timestamp;
};
struct SDL_GamepadCapSenseEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_JoystickID which;
	Uint8 capsense;
	bool down;
	Uint8 padding1;
	Uint8 padding2;
};
typedef Uint32 SDL_AudioDeviceID;
struct SDL_AudioDeviceEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_AudioDeviceID which;
	bool recording;
	Uint8 padding1;
	Uint8 padding2;
	Uint8 padding3;
};
typedef Uint32 SDL_CameraID;
struct SDL_CameraDeviceEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_CameraID which;
};
typedef Uint32 SDL_SensorID;
typedef struct{float data[6];}arr_6_float;
struct SDL_SensorEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_SensorID which;
	arr_6_float data;
	Uint64 sensor_timestamp;
};
struct SDL_QuitEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
};
struct SDL_UserEvent{
	Uint32 type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	Sint32 code;
	u8* data1;
	u8* data2;
};
typedef Uint64 SDL_TouchID;
typedef Uint64 SDL_FingerID;
struct SDL_TouchFingerEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_TouchID touchID;
	SDL_FingerID fingerID;
	float x;
	float y;
	float dx;
	float dy;
	float pressure;
	SDL_WindowID windowID;
};
struct SDL_PinchFingerEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	float scale;
	SDL_WindowID windowID;
};
typedef Uint32 SDL_PenID;
struct SDL_PenProximityEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_PenID which;
};
typedef Uint32 SDL_PenInputFlags;
struct SDL_PenTouchEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_PenID which;
	SDL_PenInputFlags pen_state;
	float x;
	float y;
	bool eraser;
	bool down;
};
struct SDL_PenMotionEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_PenID which;
	SDL_PenInputFlags pen_state;
	float x;
	float y;
};
struct SDL_PenButtonEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_PenID which;
	SDL_PenInputFlags pen_state;
	float x;
	float y;
	Uint8 button;
	bool down;
};
enum SDL_PenAxis:i32{
	SDL_PEN_AXIS_PRESSURE,
	SDL_PEN_AXIS_XTILT,
	SDL_PEN_AXIS_YTILT,
	SDL_PEN_AXIS_DISTANCE,
	SDL_PEN_AXIS_ROTATION,
	SDL_PEN_AXIS_SLIDER,
	SDL_PEN_AXIS_TANGENTIAL_PRESSURE,
	SDL_PEN_AXIS_COUNT,
};
struct SDL_PenAxisEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	SDL_PenID which;
	SDL_PenInputFlags pen_state;
	float x;
	float y;
	enum SDL_PenAxis axis;
	float value;
};
struct SDL_RenderEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
};
struct SDL_DropEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	SDL_WindowID windowID;
	float x;
	float y;
	i8* source;
	i8* data;
};
struct SDL_ClipboardEvent{
	enum SDL_EventType type;
	Uint32 reserved;
	Uint64 timestamp;
	bool owner;
	Sint32 num_mime_types;
	i8** mime_types;
};
typedef struct{Uint8 data[128];}arr_128_Uint8;
union SDL_Event{
	Uint32 type;
	struct SDL_CommonEvent common;
	struct SDL_DisplayEvent display;
	struct SDL_WindowEvent window;
	struct SDL_KeyboardDeviceEvent kdevice;
	struct SDL_KeyboardEvent key;
	struct SDL_TextEditingEvent edit;
	struct SDL_TextEditingCandidatesEvent edit_candidates;
	struct SDL_TextInputEvent text;
	struct SDL_MouseDeviceEvent mdevice;
	struct SDL_MouseMotionEvent motion;
	struct SDL_MouseButtonEvent button;
	struct SDL_MouseWheelEvent wheel;
	struct SDL_JoyDeviceEvent jdevice;
	struct SDL_JoyAxisEvent jaxis;
	struct SDL_JoyBallEvent jball;
	struct SDL_JoyHatEvent jhat;
	struct SDL_JoyButtonEvent jbutton;
	struct SDL_JoyBatteryEvent jbattery;
	struct SDL_GamepadDeviceEvent gdevice;
	struct SDL_GamepadAxisEvent gaxis;
	struct SDL_GamepadButtonEvent gbutton;
	struct SDL_GamepadTouchpadEvent gtouchpad;
	struct SDL_GamepadSensorEvent gsensor;
	struct SDL_GamepadCapSenseEvent gcapsense;
	struct SDL_AudioDeviceEvent adevice;
	struct SDL_CameraDeviceEvent cdevice;
	struct SDL_SensorEvent sensor;
	struct SDL_QuitEvent quit;
	struct SDL_UserEvent user;
	struct SDL_TouchFingerEvent tfinger;
	struct SDL_PinchFingerEvent pinch;
	struct SDL_PenProximityEvent pproximity;
	struct SDL_PenTouchEvent ptouch;
	struct SDL_PenMotionEvent pmotion;
	struct SDL_PenButtonEvent pbutton;
	struct SDL_PenAxisEvent paxis;
	struct SDL_RenderEvent render;
	struct SDL_DropEvent drop;
	struct SDL_ClipboardEvent clipboard;
	arr_128_Uint8 padding;
};


void _main_1Ast(arr_0_string args);
extern void _runTests_0();
DLLIMPORT extern bool SDL_Init(SDL_InitFlags flags);
extern void _writeln_1st(string str);
DLLIMPORT extern bool SDL_CreateWindowAndRenderer(i8* title,i32 width,i32 height,SDL_WindowFlags window_flags,struct SDL_Window** window,struct SDL_Renderer** renderer);
DLLIMPORT extern bool SDL_SetRenderDrawColor(struct SDL_Renderer* renderer,Uint8 r,Uint8 g,Uint8 b,Uint8 a);
DLLIMPORT extern bool SDL_PollEvent(union SDL_Event* event);
DLLIMPORT extern bool SDL_RenderClear(struct SDL_Renderer* renderer);
DLLIMPORT extern bool SDL_RenderPresent(struct SDL_Renderer* renderer);
DLLIMPORT extern void SDL_DestroyRenderer(struct SDL_Renderer* renderer);
DLLIMPORT extern void SDL_DestroyWindow(struct SDL_Window* window);
DLLIMPORT extern void SDL_Quit();


static i8* const _G1="Failed to initialize SDL3";
static i8* const _G2="ABC";
static i8* const _G3="Failed to create window and renderer";


void _main_1Ast(arr_0_string args){
	_runTests_0();
	const SDL_InitFlags _1=(SDL_InitFlags)0x20u;
	const bool _2=SDL_Init(_1);
	const bool _3=!_2;
	if(_3){
		const string _4={_G1,25};
		_writeln_1st(_4);
		return;
	}
	struct SDL_Window* window={0};
	struct SDL_Renderer* renderer={0};
	const SDL_WindowFlags _5=(SDL_WindowFlags)0llu;
	struct SDL_Window** const _6=&window;
	struct SDL_Renderer** const _7=&renderer;
	const bool _8=SDL_CreateWindowAndRenderer(_G2,1280,720,_5,_6,_7);
	const bool _9=!_8;
	if(_9){
		const string _10={_G3,36};
		_writeln_1st(_10);
		return;
	}
	const Uint8 _11=(Uint8)100u;
	const Uint8 _12=(Uint8)149u;
	const Uint8 _13=(Uint8)237u;
	const Uint8 _14=(Uint8)0u;
	const bool _15=SDL_SetRenderDrawColor(renderer,_11,_12,_13,_14);
	bool running=true;
	while(1){
		if(!running)break;
		union SDL_Event event={0};
		while(1){
			union SDL_Event* const _16=&event;
			const bool _17=SDL_PollEvent(_16);
			if(!_17)break;
			Uint32* const _18=&event.type;
			const enum SDL_EventType _19=(enum SDL_EventType)(*_18);
			const bool _20=_19==256;
			if(_20){
				running=false;
			}
		}
		const bool _21=SDL_RenderClear(renderer);
		const bool _22=SDL_RenderPresent(renderer);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
int main(int argc, char* argv[]){
	string argsData[argc];
	for(int i=0;i<argc;i++)argsData[i]=(string){argv[i],__cstrl(argv[i])};
	arr_0_string args={argsData,argc};
	_main_1Ast(args);
	return 0;
}