// day1
#include <cgl.h>


struct LaunchParams {
    i32 width;
    i32 height;
    i8* title;
    bool maximized;
    bool fullscreen;
    i32 fpsCap;
    i32 vsync;
};

typedef void(*__Fv0)();
typedef void(*__Fv0)();
typedef void(*__Fv0)();
typedef void(*__Fv1f32)(float);
typedef void(*__Fv0)();
typedef void(*__Fv1pi8)(i8*);
typedef void(*__Fv3i32i32u16)(i32, i32, u16);
typedef void(*__Fv2i32i32)(i32, i32);
typedef void(*__Fv2u8u32)(u8, u32);
typedef void(*__Fv3i32i32b)(i32, i32, bool);
typedef void(*__Fv2i32b)(i32, bool);
typedef void(*__Fv2i32i32)(i32, i32);
typedef void(*__Fv1pi8)(i8*);
typedef void(*__Fv1b)(bool);
struct ApplicationCallbacks {
    __Fv0 init;
    __Fv0 destroy;
    __Fv0 update;
    __Fv1f32 fixedUpdate;
    __Fv0 draw;
    __Fv1pi8 onInternalError;
    __Fv3i32i32u16 onAxisEvent;
    __Fv2i32i32 onGamepadEvent;
    __Fv2u8u32 onCharEvent;
    __Fv3i32i32b onKeyEvent;
    __Fv2i32b onMouseButtonEvent;
    __Fv2i32i32 onViewportSizeEvent;
    __Fv1pi8 onDropFileEvent;
    __Fv1b onExitEvent;
};

typedef i32 GamepadAxis;
typedef i32 KeyCode;
typedef i32 KeyModifier;
typedef i32 MouseButton;
typedef __Fv0 GameInit_t;
typedef __Fv0 GameDestroy_t;
typedef __Fv0 GameUpdate_t;
typedef __Fv1f32 GameFixedUpdate_t;
typedef __Fv0 GameDraw_t;
typedef __Fv1pi8 OnInternalErrorEvent_t;
typedef __Fv3i32i32u16 OnAxisEvent_t;
typedef __Fv2i32i32 OnGamepadEvent_t;
typedef __Fv2u8u32 OnCharEvent_t;
typedef __Fv3i32i32b OnKeyEvent_t;
typedef __Fv2i32b OnMouseButtonEvent_t;
typedef __Fv2i32i32 OnViewportSizeEvent_t;
typedef __Fv1pi8 OnDropFileEvent_t;
typedef __Fv1b OnExitEvent_t;


__declspec(dllimport) void rainfall__Application_Run_2x47296x59397(struct LaunchParams params, struct ApplicationCallbacks callbacks);