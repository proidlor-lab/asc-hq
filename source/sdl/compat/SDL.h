#ifndef ASC_SDL_COMPAT_SDL_H
#define ASC_SDL_COMPAT_SDL_H

/*
   Compatibility header that bridges SDL 1.2 style APIs used by ASC to SDL3.
   This header must be found before the system SDL headers, therefore
   configure adds -Isource/sdl/compat to CPPFLAGS.
*/

#define SDL_Event SDL3_Event
#define SDL_UserEvent SDL3_UserEvent
#define SDL_SysWMEvent SDL3_SysWMEvent
#define SDL_QuitEvent SDL3_QuitEvent
#define SDL_KeyboardEvent SDL3_KeyboardEvent
#define SDL_MouseMotionEvent SDL3_MouseMotionEvent
#define SDL_MouseButtonEvent SDL3_MouseButtonEvent
#define SDL_JoyAxisEvent SDL3_JoyAxisEvent
#define SDL_JoyBallEvent SDL3_JoyBallEvent
#define SDL_JoyHatEvent SDL3_JoyHatEvent
#define SDL_JoyButtonEvent SDL3_JoyButtonEvent
#define SDL_EventFilter SDL3_EventFilter

#include <SDL3/SDL.h>

#undef SDL_Event
#undef SDL_UserEvent
#undef SDL_SysWMEvent
#undef SDL_QuitEvent
#undef SDL_KeyboardEvent
#undef SDL_MouseMotionEvent
#undef SDL_MouseButtonEvent
#undef SDL_JoyAxisEvent
#undef SDL_JoyBallEvent
#undef SDL_JoyHatEvent
#undef SDL_JoyButtonEvent
#undef SDL_EventFilter

/* Provide stable aliases to the SDL3 runtime entry points before we
   redefine the legacy names further below. */
static inline bool SDL3_PollEvent(SDL3_Event* event)
{
   return SDL_PollEvent(event);
}

static inline bool SDL3_WaitEvent(SDL3_Event* event)
{
   return SDL_WaitEvent(event);
}

static inline bool SDL3_PushEvent(SDL3_Event* event)
{
   return SDL_PushEvent(event);
}

static inline void SDL3_SetEventFilter(SDL3_EventFilter filter, void* userdata)
{
   SDL_SetEventFilter(filter, userdata);
}

static inline bool SDL3_GetEventFilter(SDL3_EventFilter* filter, void** userdata)
{
   return SDL_GetEventFilter(filter, userdata);
}

/* SDL3 changed SDL_Init/SDL_InitSubSystem to return bool instead of int.
   Provide stable aliases before redefining them below. */
static inline bool SDL3_Init(SDL_InitFlags flags)
{
   return SDL_Init(flags);
}

static inline bool SDL3_InitSubSystem(SDL_InitFlags flags)
{
   return SDL_InitSubSystem(flags);
}

#ifndef DECLSPEC
#define DECLSPEC SDL_DECLSPEC
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* SDL 1.2 surface flag compatibility */
#ifndef SDL_SWSURFACE
#define SDL_SWSURFACE 0x00000000u
#endif
#ifndef SDL_HWSURFACE
#define SDL_HWSURFACE 0x00000001u
#endif
#ifndef SDL_ASYNCBLIT
#define SDL_ASYNCBLIT 0x00000004u
#endif
#ifndef SDL_SRCCOLORKEY
#define SDL_SRCCOLORKEY 0x00001000u
#endif
#ifndef SDL_RLEACCEL
#define SDL_RLEACCEL 0x00004000u
#endif
#ifndef SDL_SRCALPHA
#define SDL_SRCALPHA 0x00010000u
#endif
#ifndef SDL_OPENGL
#define SDL_OPENGL 0x00000002u
#endif
#ifndef SDL_FULLSCREEN
#define SDL_FULLSCREEN 0x80000000u
#endif
#ifndef SDL_RESIZABLE
#define SDL_RESIZABLE 0x00000010u
#endif
#ifndef SDL_NOFRAME
#define SDL_NOFRAME 0x00000020u
#endif
#ifndef SDL_DOUBLEBUF
#define SDL_DOUBLEBUF 0x40000000u
#endif
#ifndef SDL_ANYFORMAT
#define SDL_ANYFORMAT 0x10000000u
#endif
#ifndef SDL_HWPALETTE
#define SDL_HWPALETTE 0x20000000u
#endif

#ifndef SDL_LOGPAL
#define SDL_LOGPAL 0x00000001u
#endif
#ifndef SDL_PHYSPAL
#define SDL_PHYSPAL 0x00000002u
#endif

#ifdef SDL_FreeSurface
#undef SDL_FreeSurface
#endif
#define SDL_FreeSurface SDL_DestroySurface

#ifdef SDL_FillRect
#undef SDL_FillRect
#endif
#define SDL_FillRect SDL_FillSurfaceRect

#ifdef SDL_ConvertSurfaceFormat
#undef SDL_ConvertSurfaceFormat
#endif
#define SDL_ConvertSurfaceFormat SDL_ConvertSurface

#ifdef SDL_TRUE
#undef SDL_TRUE
#endif
#define SDL_TRUE 1

#ifdef SDL_FALSE
#undef SDL_FALSE
#endif
#define SDL_FALSE 0

#ifdef SDL_VERSION
#undef SDL_VERSION
#endif
#define SDL_VERSION(X)                 \
   do {                                \
      (X)->major = SDL_MAJOR_VERSION;  \
      (X)->minor = SDL_MINOR_VERSION;  \
      (X)->patch = SDL_MICRO_VERSION;  \
   } while (0)

#define SDL_COMPAT_VERSION_VALUE SDL_VERSIONNUM(SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION)

#ifdef SDL_COMPILEDVERSION
#undef SDL_COMPILEDVERSION
#endif
#define SDL_COMPILEDVERSION SDL_COMPAT_VERSION_VALUE

#ifdef SDL_VERSION_ATLEAST
#undef SDL_VERSION_ATLEAST
#endif
#define SDL_VERSION_ATLEAST(X, Y, Z) \
   (SDL_COMPAT_VERSION_VALUE >= SDL_VERSIONNUM((X), (Y), (Z)))

#ifdef SDL_PIXELFORMAT_RGB444
#undef SDL_PIXELFORMAT_RGB444
#endif
#define SDL_PIXELFORMAT_RGB444 SDL_PIXELFORMAT_XRGB4444

#ifdef SDL_PIXELFORMAT_RGB555
#undef SDL_PIXELFORMAT_RGB555
#endif
#define SDL_PIXELFORMAT_RGB555 SDL_PIXELFORMAT_XRGB1555

#ifdef SDL_PIXELFORMAT_RGB888
#undef SDL_PIXELFORMAT_RGB888
#endif
#define SDL_PIXELFORMAT_RGB888 SDL_PIXELFORMAT_XRGB8888

#ifdef SDL_PIXELFORMAT_BGR444
#undef SDL_PIXELFORMAT_BGR444
#endif
#define SDL_PIXELFORMAT_BGR444 SDL_PIXELFORMAT_XBGR4444

#ifdef SDL_PIXELFORMAT_BGR555
#undef SDL_PIXELFORMAT_BGR555
#endif
#define SDL_PIXELFORMAT_BGR555 SDL_PIXELFORMAT_XBGR1555

#ifdef SDL_PIXELFORMAT_BGR888
#undef SDL_PIXELFORMAT_BGR888
#endif
#define SDL_PIXELFORMAT_BGR888 SDL_PIXELFORMAT_XBGR8888

#ifdef SDL_SwapLE16
#undef SDL_SwapLE16
#endif
#define SDL_SwapLE16 SDL_Swap16LE

#ifdef SDL_SwapBE16
#undef SDL_SwapBE16
#endif
#define SDL_SwapBE16 SDL_Swap16BE

#ifdef SDL_SwapLE32
#undef SDL_SwapLE32
#endif
#define SDL_SwapLE32 SDL_Swap32LE

#ifdef SDL_SwapBE32
#undef SDL_SwapBE32
#endif
#define SDL_SwapBE32 SDL_Swap32BE

#ifdef SDL_SwapLE64
#undef SDL_SwapLE64
#endif
#define SDL_SwapLE64 SDL_Swap64LE

#ifdef SDL_SwapBE64
#undef SDL_SwapBE64
#endif
#define SDL_SwapBE64 SDL_Swap64BE

#ifdef RW_SEEK_SET
#undef RW_SEEK_SET
#endif
#define RW_SEEK_SET SDL_IO_SEEK_SET

#ifdef RW_SEEK_CUR
#undef RW_SEEK_CUR
#endif
#define RW_SEEK_CUR SDL_IO_SEEK_CUR

#ifdef RW_SEEK_END
#undef RW_SEEK_END
#endif
#define RW_SEEK_END SDL_IO_SEEK_END

#ifdef SDL_bool
#undef SDL_bool
#endif
typedef int SDL_bool;

#ifdef SDL_SetClipRect
#undef SDL_SetClipRect
#endif
#define SDL_SetClipRect SDL_SetSurfaceClipRect

#ifdef SDL_GetClipRect
#undef SDL_GetClipRect
#endif
#define SDL_GetClipRect SDL_GetSurfaceClipRect

#ifdef SDL_BUTTON
#undef SDL_BUTTON
#endif
#define SDL_BUTTON(X) SDL_BUTTON_MASK(X)

#ifdef SDL_LoadBMP_RW
#undef SDL_LoadBMP_RW
#endif
#define SDL_LoadBMP_RW SDL_LoadBMP_IO

#ifdef SDL_mutex
#undef SDL_mutex
#endif
typedef SDL_Mutex SDL_mutex;

#ifndef SDL_APPACTIVE
#define SDL_APPACTIVE 0x01
#define SDL_APPINPUTFOCUS 0x02
#define SDL_APPMOUSEFOCUS 0x04
#endif

#define SDL_RELEASED 0
#define SDL_PRESSED 1

/* Audio format compatibility for SDL 1.2 */
/* Undefine SDL3's renamed/poisoned versions first */
#ifdef AUDIO_U8
#undef AUDIO_U8
#endif
#define AUDIO_U8 SDL_AUDIO_U8

#ifdef AUDIO_S8
#undef AUDIO_S8
#endif
#define AUDIO_S8 SDL_AUDIO_S8

#ifdef AUDIO_U16LSB
#undef AUDIO_U16LSB
#endif
#define AUDIO_U16LSB SDL_AUDIO_S16LE /* SDL3 removed unsigned 16-bit */

#ifdef AUDIO_S16LSB
#undef AUDIO_S16LSB
#endif
#define AUDIO_S16LSB SDL_AUDIO_S16LE

#ifdef AUDIO_U16MSB
#undef AUDIO_U16MSB
#endif
#define AUDIO_U16MSB SDL_AUDIO_S16BE /* SDL3 removed unsigned 16-bit */

#ifdef AUDIO_S16MSB
#undef AUDIO_S16MSB
#endif
#define AUDIO_S16MSB SDL_AUDIO_S16BE

#ifdef AUDIO_U16
#undef AUDIO_U16
#endif
#define AUDIO_U16 SDL_AUDIO_S16LE /* SDL3 removed unsigned 16-bit */

#ifdef AUDIO_S16
#undef AUDIO_S16
#endif
#define AUDIO_S16 SDL_AUDIO_S16LE

#ifdef AUDIO_S32LSB
#undef AUDIO_S32LSB
#endif
#define AUDIO_S32LSB SDL_AUDIO_S32LE

#ifdef AUDIO_S32MSB
#undef AUDIO_S32MSB
#endif
#define AUDIO_S32MSB SDL_AUDIO_S32BE

#ifdef AUDIO_S32
#undef AUDIO_S32
#endif
#define AUDIO_S32 SDL_AUDIO_S32LE

#ifdef AUDIO_F32LSB
#undef AUDIO_F32LSB
#endif
#define AUDIO_F32LSB SDL_AUDIO_F32LE

#ifdef AUDIO_F32MSB
#undef AUDIO_F32MSB
#endif
#define AUDIO_F32MSB SDL_AUDIO_F32BE

#ifdef AUDIO_F32
#undef AUDIO_F32
#endif
#define AUDIO_F32 SDL_AUDIO_F32LE

/* System-endian formats */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#ifdef AUDIO_U16SYS
#undef AUDIO_U16SYS
#endif
#define AUDIO_U16SYS SDL_AUDIO_S16LE /* SDL3 removed unsigned 16-bit */

#ifdef AUDIO_S16SYS
#undef AUDIO_S16SYS
#endif
#define AUDIO_S16SYS SDL_AUDIO_S16LE

#ifdef AUDIO_S32SYS
#undef AUDIO_S32SYS
#endif
#define AUDIO_S32SYS SDL_AUDIO_S32LE

#ifdef AUDIO_F32SYS
#undef AUDIO_F32SYS
#endif
#define AUDIO_F32SYS SDL_AUDIO_F32LE
#else
#ifdef AUDIO_U16SYS
#undef AUDIO_U16SYS
#endif
#define AUDIO_U16SYS SDL_AUDIO_S16BE /* SDL3 removed unsigned 16-bit */

#ifdef AUDIO_S16SYS
#undef AUDIO_S16SYS
#endif
#define AUDIO_S16SYS SDL_AUDIO_S16BE

#ifdef AUDIO_S32SYS
#undef AUDIO_S32SYS
#endif
#define AUDIO_S32SYS SDL_AUDIO_S32BE

#ifdef AUDIO_F32SYS
#undef AUDIO_F32SYS
#endif
#define AUDIO_F32SYS SDL_AUDIO_F32BE
#endif

#ifdef KMOD_NUM
#undef KMOD_NUM
#endif
#define KMOD_NUM SDL_KMOD_NUM

#ifdef KMOD_CAPS
#undef KMOD_CAPS
#endif
#define KMOD_CAPS SDL_KMOD_CAPS

#ifdef KMOD_SCROLL
#undef KMOD_SCROLL
#endif
#define KMOD_SCROLL SDL_KMOD_SCROLL

#ifdef KMOD_CTRL
#undef KMOD_CTRL
#endif
#define KMOD_CTRL SDL_KMOD_CTRL

#ifdef KMOD_ALT
#undef KMOD_ALT
#endif
#define KMOD_ALT SDL_KMOD_ALT

#ifdef KMOD_SHIFT
#undef KMOD_SHIFT
#endif
#define KMOD_SHIFT SDL_KMOD_SHIFT

#ifdef KMOD_MODE
#undef KMOD_MODE
#endif
#define KMOD_MODE SDL_KMOD_MODE

#ifdef SDLK_a
#undef SDLK_a
#endif
#define SDLK_a SDLK_A
#ifdef SDLK_b
#undef SDLK_b
#endif
#define SDLK_b SDLK_B
#ifdef SDLK_c
#undef SDLK_c
#endif
#define SDLK_c SDLK_C
#ifdef SDLK_d
#undef SDLK_d
#endif
#define SDLK_d SDLK_D
#ifdef SDLK_e
#undef SDLK_e
#endif
#define SDLK_e SDLK_E
#ifdef SDLK_f
#undef SDLK_f
#endif
#define SDLK_f SDLK_F
#ifdef SDLK_g
#undef SDLK_g
#endif
#define SDLK_g SDLK_G
#ifdef SDLK_h
#undef SDLK_h
#endif
#define SDLK_h SDLK_H
#ifdef SDLK_i
#undef SDLK_i
#endif
#define SDLK_i SDLK_I
#ifdef SDLK_j
#undef SDLK_j
#endif
#define SDLK_j SDLK_J
#ifdef SDLK_k
#undef SDLK_k
#endif
#define SDLK_k SDLK_K
#ifdef SDLK_l
#undef SDLK_l
#endif
#define SDLK_l SDLK_L
#ifdef SDLK_m
#undef SDLK_m
#endif
#define SDLK_m SDLK_M
#ifdef SDLK_n
#undef SDLK_n
#endif
#define SDLK_n SDLK_N
#ifdef SDLK_o
#undef SDLK_o
#endif
#define SDLK_o SDLK_O
#ifdef SDLK_p
#undef SDLK_p
#endif
#define SDLK_p SDLK_P
#ifdef SDLK_q
#undef SDLK_q
#endif
#define SDLK_q SDLK_Q
#ifdef SDLK_r
#undef SDLK_r
#endif
#define SDLK_r SDLK_R
#ifdef SDLK_s
#undef SDLK_s
#endif
#define SDLK_s SDLK_S
#ifdef SDLK_t
#undef SDLK_t
#endif
#define SDLK_t SDLK_T
#ifdef SDLK_u
#undef SDLK_u
#endif
#define SDLK_u SDLK_U
#ifdef SDLK_v
#undef SDLK_v
#endif
#define SDLK_v SDLK_V
#ifdef SDLK_w
#undef SDLK_w
#endif
#define SDLK_w SDLK_W
#ifdef SDLK_x
#undef SDLK_x
#endif
#define SDLK_x SDLK_X
#ifdef SDLK_y
#undef SDLK_y
#endif
#define SDLK_y SDLK_Y
#ifdef SDLK_z
#undef SDLK_z
#endif
#define SDLK_z SDLK_Z

static inline Uint32 SDLCompat_InitEverythingMask(void)
{
   Uint32 mask = 0;
#ifdef SDL_INIT_TIMER
   mask |= SDL_INIT_TIMER;
#endif
#ifdef SDL_INIT_AUDIO
   mask |= SDL_INIT_AUDIO;
#endif
#ifdef SDL_INIT_VIDEO
   mask |= SDL_INIT_VIDEO;
#endif
#ifdef SDL_INIT_EVENTS
   mask |= SDL_INIT_EVENTS;
#endif
#ifdef SDL_INIT_JOYSTICK
   mask |= SDL_INIT_JOYSTICK;
#endif
#ifdef SDL_INIT_GAMEPAD
   mask |= SDL_INIT_GAMEPAD;
#endif
#ifdef SDL_INIT_HAPTIC
   mask |= SDL_INIT_HAPTIC;
#endif
#ifdef SDL_INIT_SENSOR
   mask |= SDL_INIT_SENSOR;
#endif
#ifdef SDL_INIT_CAMERA
   mask |= SDL_INIT_CAMERA;
#endif
   return mask;
}

#ifndef SDL_INIT_EVERYTHING
#define SDL_INIT_EVERYTHING SDLCompat_InitEverythingMask()
#endif

#ifndef SDL_DISABLE
#define SDL_DISABLE 0
#define SDL_ENABLE 1
#define SDL_QUERY -1
#endif

#define SDL_IGNORE SDL_DISABLE

#ifndef SDL_DEFAULT_REPEAT_DELAY
#define SDL_DEFAULT_REPEAT_DELAY 500
#endif
#ifndef SDL_DEFAULT_REPEAT_INTERVAL
#define SDL_DEFAULT_REPEAT_INTERVAL 30
#endif

#ifdef SDL_NOEVENT
#undef SDL_NOEVENT
#endif
#define SDL_NOEVENT          0
#ifdef SDL_ACTIVEEVENT
#undef SDL_ACTIVEEVENT
#endif
#define SDL_ACTIVEEVENT      1
#ifdef SDL_KEYDOWN
#undef SDL_KEYDOWN
#endif
#define SDL_KEYDOWN          2
#ifdef SDL_KEYUP
#undef SDL_KEYUP
#endif
#define SDL_KEYUP            3
#ifdef SDL_MOUSEMOTION
#undef SDL_MOUSEMOTION
#endif
#define SDL_MOUSEMOTION      4
#ifdef SDL_MOUSEBUTTONDOWN
#undef SDL_MOUSEBUTTONDOWN
#endif
#define SDL_MOUSEBUTTONDOWN  5
#ifdef SDL_MOUSEBUTTONUP
#undef SDL_MOUSEBUTTONUP
#endif
#define SDL_MOUSEBUTTONUP    6
#ifdef SDL_JOYAXISMOTION
#undef SDL_JOYAXISMOTION
#endif
#define SDL_JOYAXISMOTION    7
#ifdef SDL_JOYBALLMOTION
#undef SDL_JOYBALLMOTION
#endif
#define SDL_JOYBALLMOTION    8
#ifdef SDL_JOYHATMOTION
#undef SDL_JOYHATMOTION
#endif
#define SDL_JOYHATMOTION     9
#ifdef SDL_JOYBUTTONDOWN
#undef SDL_JOYBUTTONDOWN
#endif
#define SDL_JOYBUTTONDOWN   10
#ifdef SDL_JOYBUTTONUP
#undef SDL_JOYBUTTONUP
#endif
#define SDL_JOYBUTTONUP     11
#ifdef SDL_QUIT
#undef SDL_QUIT
#endif
#define SDL_QUIT            12
#ifdef SDL_SYSWMEVENT
#undef SDL_SYSWMEVENT
#endif
#define SDL_SYSWMEVENT      13
#ifdef SDL_EVENT_RESERVEDA
#undef SDL_EVENT_RESERVEDA
#endif
#define SDL_EVENT_RESERVEDA 14
#ifdef SDL_EVENT_RESERVEDB
#undef SDL_EVENT_RESERVEDB
#endif
#define SDL_EVENT_RESERVEDB 15
#ifdef SDL_VIDEORESIZE
#undef SDL_VIDEORESIZE
#endif
#define SDL_VIDEORESIZE     16
#ifdef SDL_VIDEOEXPOSE
#undef SDL_VIDEOEXPOSE
#endif
#define SDL_VIDEOEXPOSE     17
#ifdef SDL_EVENT_RESERVED2
#undef SDL_EVENT_RESERVED2
#endif
#define SDL_EVENT_RESERVED2 18
#ifdef SDL_EVENT_RESERVED3
#undef SDL_EVENT_RESERVED3
#endif
#define SDL_EVENT_RESERVED3 19
#ifdef SDL_EVENT_RESERVED4
#undef SDL_EVENT_RESERVED4
#endif
#define SDL_EVENT_RESERVED4 20
#ifdef SDL_EVENT_RESERVED5
#undef SDL_EVENT_RESERVED5
#endif
#define SDL_EVENT_RESERVED5 21
#ifdef SDL_EVENT_RESERVED6
#undef SDL_EVENT_RESERVED6
#endif
#define SDL_EVENT_RESERVED6 22
#ifdef SDL_EVENT_RESERVED7
#undef SDL_EVENT_RESERVED7
#endif
#define SDL_EVENT_RESERVED7 23
#ifdef SDL_USEREVENT
#undef SDL_USEREVENT
#endif
#define SDL_USEREVENT       24
#ifdef SDL_NUMEVENTS
#undef SDL_NUMEVENTS
#endif
#define SDL_NUMEVENTS       32

#ifndef SDLK_KP0
#define SDLK_KP0 SDLK_KP_0
#endif
#ifndef SDLK_KP1
#define SDLK_KP1 SDLK_KP_1
#endif
#ifndef SDLK_KP2
#define SDLK_KP2 SDLK_KP_2
#endif
#ifndef SDLK_KP3
#define SDLK_KP3 SDLK_KP_3
#endif
#ifndef SDLK_KP4
#define SDLK_KP4 SDLK_KP_4
#endif
#ifndef SDLK_KP5
#define SDLK_KP5 SDLK_KP_5
#endif
#ifndef SDLK_KP6
#define SDLK_KP6 SDLK_KP_6
#endif
#ifndef SDLK_KP7
#define SDLK_KP7 SDLK_KP_7
#endif
#ifndef SDLK_KP8
#define SDLK_KP8 SDLK_KP_8
#endif
#ifndef SDLK_KP9
#define SDLK_KP9 SDLK_KP_9
#endif

#ifndef KMOD_CTRL
#define KMOD_CTRL SDL_KMOD_CTRL
#endif
#ifndef KMOD_SHIFT
#define KMOD_SHIFT SDL_KMOD_SHIFT
#endif
#ifndef KMOD_ALT
#define KMOD_ALT SDL_KMOD_ALT
#endif
#ifndef KMOD_GUI
#define KMOD_GUI SDL_KMOD_GUI
#endif
#ifndef KMOD_META
#define KMOD_META SDL_KMOD_GUI
#endif
#ifndef KMOD_LMETA
#define KMOD_LMETA SDL_KMOD_LGUI
#endif
#ifndef KMOD_RMETA
#define KMOD_RMETA SDL_KMOD_RGUI
#endif

typedef enum {
   SDL_GRAB_QUERY = -1,
   SDL_GRAB_OFF = 0,
   SDL_GRAB_ON = 1
} SDL_GrabMode;

/* Legacy SDL structures */
typedef struct SDL_CompatPixelFormat {
   Uint8 BitsPerPixel;
   Uint8 BytesPerPixel;
   Uint8 Rloss;
   Uint8 Gloss;
   Uint8 Bloss;
   Uint8 Aloss;
   Uint8 Rshift;
   Uint8 Gshift;
   Uint8 Bshift;
   Uint8 Ashift;
   Uint32 Rmask;
   Uint32 Gmask;
   Uint32 Bmask;
   Uint32 Amask;
   SDL_Palette* palette;
   Uint32 colorkey;
   Uint8 alpha;
} SDL_CompatPixelFormat;

typedef struct SDL_VideoInfo {
   Uint32 hw_available;
   Uint32 wm_available;
   Uint32 blit_hw;
   Uint32 blit_hw_CC;
   Uint32 blit_hw_A;
   Uint32 blit_sw;
   Uint32 blit_sw_CC;
   Uint32 blit_sw_A;
   Uint32 blit_fill;
   Uint32 video_mem;
   SDL_CompatPixelFormat vfmt;
   int current_w;
   int current_h;
} SDL_VideoInfo;

typedef struct SDL_version {
   Uint8 major;
   Uint8 minor;
   Uint8 patch;
} SDL_version;

typedef struct SDL_SysWMinfo {
   SDL_version version;
} SDL_SysWMinfo;

typedef SDL_Keycode SDLKey;
typedef SDL_Keymod SDLMod;

typedef struct SDL_keysym {
   Uint8 scancode;
   SDLKey sym;
   SDLMod mod;
   Uint16 unicode;
} SDL_keysym;

typedef struct SDL_KeyboardEvent {
   Uint32 type;
   Uint8 which;
   Uint8 state;
   SDL_keysym keysym;
} SDL_KeyboardEvent;

typedef struct SDL_MouseMotionEvent {
   Uint32 type;
   Uint8 which;
   Uint8 state;
   Uint16 x;
   Uint16 y;
   Sint16 xrel;
   Sint16 yrel;
} SDL_MouseMotionEvent;

typedef struct SDL_MouseButtonEvent {
   Uint32 type;
   Uint8 which;
   Uint8 button;
   Uint8 state;
   Uint16 x;
   Uint16 y;
} SDL_MouseButtonEvent;

typedef struct SDL_JoyAxisEvent {
   Uint32 type;
   Uint8 which;
   Uint8 axis;
   Sint16 value;
} SDL_JoyAxisEvent;

typedef struct SDL_JoyBallEvent {
   Uint32 type;
   Uint8 which;
   Uint8 ball;
   Sint16 xrel;
   Sint16 yrel;
} SDL_JoyBallEvent;

typedef struct SDL_JoyHatEvent {
   Uint32 type;
   Uint8 which;
   Uint8 hat;
   Uint8 value;
} SDL_JoyHatEvent;

typedef struct SDL_JoyButtonEvent {
   Uint32 type;
   Uint8 which;
   Uint8 button;
   Uint8 state;
} SDL_JoyButtonEvent;

typedef struct SDL_ActiveEvent {
   Uint32 type;
   Uint8 gain;
   Uint8 state;
} SDL_ActiveEvent;

typedef struct SDL_ExposeEvent {
   Uint32 type;
} SDL_ExposeEvent;

typedef struct SDL_ResizeEvent {
   Uint32 type;
   int w;
   int h;
} SDL_ResizeEvent;

typedef struct SDL_QuitEvent {
   Uint32 type;
} SDL_QuitEvent;

typedef struct SDL_UserEvent {
   Uint32 type;
   int code;
   void* data1;
   void* data2;
} SDL_UserEvent;

typedef struct SDL_SysWMmsg {
   int placeholder;
} SDL_SysWMmsg;

typedef struct SDL_SysWMEvent {
   Uint32 type;
   SDL_SysWMmsg* msg;
} SDL_SysWMEvent;

typedef union SDL_Event {
   Uint32 type;
   SDL_ActiveEvent active;
   SDL_KeyboardEvent key;
   SDL_MouseMotionEvent motion;
   SDL_MouseButtonEvent button;
   SDL_JoyAxisEvent jaxis;
   SDL_JoyBallEvent jball;
   SDL_JoyHatEvent jhat;
   SDL_JoyButtonEvent jbutton;
   SDL_QuitEvent quit;
   SDL_UserEvent user;
   SDL_SysWMEvent syswm;
   SDL_ResizeEvent resize;
   SDL_ExposeEvent expose;
} SDL_Event;

typedef SDL_EventAction SDL_eventaction;

typedef int (*SDL_EventFilter)(const SDL_Event* event);
/* Compatibility function declarations */
SDL_Surface* SDLCompat_SetVideoMode(int width, int height, int bpp, Uint32 flags);
SDL_Surface* SDLCompat_GetVideoSurface(void);
void SDLCompat_UpdateRect(SDL_Surface* surface, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
void SDLCompat_UpdateRects(SDL_Surface* surface, int numrects, SDL_Rect* rects);
int SDLCompat_Flip(SDL_Surface* surface);
void SDLCompat_WM_SetCaption(const char* title, const char* icon);
int SDLCompat_EnableUNICODE(int enable);
int SDLCompat_EnableKeyRepeat(int delay, int interval);
Uint8* SDLCompat_GetKeyState(int* numkeys);
Uint8 SDLCompat_IsKeyPressed(SDLKey key);
const SDL_VideoInfo* SDLCompat_GetVideoInfo(void);
SDL_Rect** SDLCompat_ListModes(SDL_PixelFormat* format, Uint32 flags);
const char* SDLCompat_VideoDriverName(char* namebuf, int maxlen);
const SDL_version* SDLCompat_Linked_Version(void);
SDL_Surface* SDLCompat_DisplayFormat(SDL_Surface* surface);
SDL_Surface* SDLCompat_DisplayFormatAlpha(SDL_Surface* surface);
int SDLCompat_SetColorKey(SDL_Surface* surface, int flag, Uint32 key);
int SDLCompat_SetAlpha(SDL_Surface* surface, Uint32 flag, Uint8 alpha);
int SDLCompat_SetColors(SDL_Surface* surface, SDL_Color* colors, int firstcolor, int ncolors);
SDL_Rect SDLCompat_GetClipRect(SDL_Surface* surface);
SDL_Surface* SDLCompat_CreateRGBSurface(Uint32 flags, int width, int height, int depth,
                                        Uint32 rmask, Uint32 gmask, Uint32 bmask, Uint32 amask);
SDL_Surface* SDLCompat_CreateRGBSurfaceFrom(void* pixels, int width, int height, int depth,
                                            int pitch, Uint32 rmask, Uint32 gmask,
                                            Uint32 bmask, Uint32 amask);
int SDLCompat_SetPalette(SDL_Surface* surface, int flags, SDL_Color* colors, int firstcolor, int ncolors);
int SDLCompat_VideoModeOK(int width, int height, int bpp, Uint32 flags);
int SDLCompat_WM_IconifyWindow(void);
int SDLCompat_WM_ToggleFullScreen(SDL_Surface* surface);
SDL_GrabMode SDLCompat_WM_GrabInput(SDL_GrabMode mode);
void SDLCompat_WM_GetCaption(char** title, char** icon);
void SDLCompat_WM_SetIcon(SDL_Surface* icon, Uint8* mask);
void SDLCompat_WarpMouse(Uint16 x, Uint16 y);
Uint8 SDLCompat_GetAppState(void);
int SDLCompat_GetWMInfo(SDL_SysWMinfo* info);
typedef Uint32 (SDLCALL *SDL_TimerCallbackSimple)(Uint32 interval);
int SDLCompat_Init(Uint32 flags);
int SDLCompat_InitSubSystem(Uint32 flags);
SDL_TimerID SDLCompat_AddTimer(Uint32 interval, Uint32 (SDLCALL *callback)(Uint32 interval, void* param), void* param);
int SDLCompat_RemoveTimer(SDL_TimerID id);
int SDLCompat_SetTimer(Uint32 interval, SDL_TimerCallbackSimple callback);
int SDLCompat_PollEvent(SDL_Event* event);
int SDLCompat_WaitEvent(SDL_Event* event);
int SDLCompat_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, Uint32 mask);
int SDLCompat_PushEvent(SDL_Event* event);
Uint8 SDLCompat_EventState(Uint8 type, int state);
void SDLCompat_SetEventFilter(SDL_EventFilter filter);
SDL_EventFilter SDLCompat_GetEventFilter(void);
Uint8 SDLCompat_GetMouseState(int* x, int* y);
Uint8 SDLCompat_GetRelativeMouseState(int* x, int* y);
int SDLCompat_JoystickEventState(int state);

SDL_CompatPixelFormat SDLCompat_BuildPixelFormat(SDL_PixelFormat format);
SDL_CompatPixelFormat SDLCompat_BuildSurfacePixelFormat(const SDL_Surface* surface);

static inline SDL_CompatPixelFormat SDLCompat_GetSurfaceFormat(const SDL_Surface* surface)
{
   return SDLCompat_BuildSurfacePixelFormat(surface);
}

static inline SDL_CompatPixelFormat SDLCompat_GetPixelFormat(SDL_PixelFormat format)
{
   return SDLCompat_BuildPixelFormat(format);
}

/* SDL_rwops compatibility layer */
#ifdef SDL_RWops
#undef SDL_RWops
#endif
typedef SDL_IOStream SDL_RWops;

SDL_RWops* SDLCompat_RWFromFile(const char* file, const char* mode);
Sint64 SDLCompat_RWseek(SDL_RWops* context, Sint64 offset, int whence);
size_t SDLCompat_RWread(SDL_RWops* context, void* ptr, size_t size, size_t maxnum);
size_t SDLCompat_RWwrite(SDL_RWops* context, const void* ptr, size_t size, size_t num);
Sint64 SDLCompat_RWtell(SDL_RWops* context);
Sint64 SDLCompat_RWsize(SDL_RWops* context);
int SDLCompat_RWclose(SDL_RWops* context);

#ifdef SDL_RWFromFile
#undef SDL_RWFromFile
#endif
#ifdef SDL_RWread
#undef SDL_RWread
#endif
#ifdef SDL_RWwrite
#undef SDL_RWwrite
#endif
#ifdef SDL_RWseek
#undef SDL_RWseek
#endif
#ifdef SDL_RWtell
#undef SDL_RWtell
#endif
#ifdef SDL_RWsize
#undef SDL_RWsize
#endif
#ifdef SDL_RWclose
#undef SDL_RWclose
#endif

#define SDL_RWFromFile SDLCompat_RWFromFile
#define SDL_RWread SDLCompat_RWread
#define SDL_RWwrite SDLCompat_RWwrite
#define SDL_RWseek SDLCompat_RWseek
#define SDL_RWtell SDLCompat_RWtell
#define SDL_RWsize SDLCompat_RWsize
#define SDL_RWclose SDLCompat_RWclose

int SDLCompat_mutexP(SDL_Mutex* mutex);
int SDLCompat_mutexV(SDL_Mutex* mutex);

#ifdef SDL_mutexP
#undef SDL_mutexP
#endif
#ifdef SDL_mutexV
#undef SDL_mutexV
#endif
#define SDL_mutexP SDLCompat_mutexP
#define SDL_mutexV SDLCompat_mutexV


#ifdef __cplusplus
}
#endif

/* Macro remapping */
#ifdef SDL_SetColorKey
#undef SDL_SetColorKey
#endif
#ifdef SDL_SetAlpha
#undef SDL_SetAlpha
#endif
#ifdef SDL_CreateRGBSurface
#undef SDL_CreateRGBSurface
#endif
#ifdef SDL_CreateRGBSurfaceFrom
#undef SDL_CreateRGBSurfaceFrom
#endif
#ifdef SDL_SetColors
#undef SDL_SetColors
#endif
#ifdef SDL_SetPalette
#undef SDL_SetPalette
#endif
#ifdef SDL_SetVideoMode
#undef SDL_SetVideoMode
#endif
#ifdef SDL_GetVideoSurface
#undef SDL_GetVideoSurface
#endif
#ifdef SDL_UpdateRect
#undef SDL_UpdateRect
#endif
#ifdef SDL_UpdateRects
#undef SDL_UpdateRects
#endif
#ifdef SDL_Flip
#undef SDL_Flip
#endif
#ifdef SDL_WM_SetCaption
#undef SDL_WM_SetCaption
#endif
#ifdef SDL_EnableUNICODE
#undef SDL_EnableUNICODE
#endif
#ifdef SDL_EnableKeyRepeat
#undef SDL_EnableKeyRepeat
#endif
#ifdef SDL_GetKeyState
#undef SDL_GetKeyState
#endif
#ifdef SDL_GetVideoInfo
#undef SDL_GetVideoInfo
#endif
#ifdef SDL_ListModes
#undef SDL_ListModes
#endif
#ifdef SDL_VideoDriverName
#undef SDL_VideoDriverName
#endif
#ifdef SDL_Linked_Version
#undef SDL_Linked_Version
#endif
#ifdef SDL_DisplayFormat
#undef SDL_DisplayFormat
#endif
#ifdef SDL_DisplayFormatAlpha
#undef SDL_DisplayFormatAlpha
#endif
#ifdef SDL_VideoModeOK
#undef SDL_VideoModeOK
#endif
#ifdef SDL_WM_IconifyWindow
#undef SDL_WM_IconifyWindow
#endif
#ifdef SDL_WM_ToggleFullScreen
#undef SDL_WM_ToggleFullScreen
#endif
#ifdef SDL_WM_GrabInput
#undef SDL_WM_GrabInput
#endif
#ifdef SDL_WM_GetCaption
#undef SDL_WM_GetCaption
#endif
#ifdef SDL_WM_SetIcon
#undef SDL_WM_SetIcon
#endif
#ifdef SDL_WarpMouse
#undef SDL_WarpMouse
#endif
#ifdef SDL_GetAppState
#undef SDL_GetAppState
#endif
#ifdef SDL_GetWMInfo
#undef SDL_GetWMInfo
#endif
#ifdef SDL_AddTimer
#undef SDL_AddTimer
#endif
#ifdef SDL_RemoveTimer
#undef SDL_RemoveTimer
#endif
#ifdef SDL_SetTimer
#undef SDL_SetTimer
#endif
#ifdef SDL_PollEvent
#undef SDL_PollEvent
#endif
#ifdef SDL_WaitEvent
#undef SDL_WaitEvent
#endif
#ifdef SDL_PeepEvents
#undef SDL_PeepEvents
#endif
#ifdef SDL_PushEvent
#undef SDL_PushEvent
#endif
#ifdef SDL_EventState
#undef SDL_EventState
#endif
#ifdef SDL_SetEventFilter
#undef SDL_SetEventFilter
#endif
#ifdef SDL_GetEventFilter
#undef SDL_GetEventFilter
#endif
#ifdef SDL_GetMouseState
#undef SDL_GetMouseState
#endif
#ifdef SDL_GetRelativeMouseState
#undef SDL_GetRelativeMouseState
#endif
#ifdef SDL_JoystickEventState
#undef SDL_JoystickEventState
#endif
#define SDL_SetVideoMode SDLCompat_SetVideoMode
#define SDL_GetVideoSurface SDLCompat_GetVideoSurface
#define SDL_UpdateRect SDLCompat_UpdateRect
#define SDL_UpdateRects SDLCompat_UpdateRects
#define SDL_Flip SDLCompat_Flip
#define SDL_WM_SetCaption SDLCompat_WM_SetCaption
#define SDL_EnableUNICODE SDLCompat_EnableUNICODE
#define SDL_EnableKeyRepeat SDLCompat_EnableKeyRepeat
#define SDL_GetKeyState SDLCompat_GetKeyState
#define SDL_GetVideoInfo SDLCompat_GetVideoInfo
#define SDL_ListModes SDLCompat_ListModes
#define SDL_VideoDriverName SDLCompat_VideoDriverName
#define SDL_Linked_Version SDLCompat_Linked_Version
#define SDL_DisplayFormat SDLCompat_DisplayFormat
#define SDL_DisplayFormatAlpha SDLCompat_DisplayFormatAlpha
#define SDL_SetColorKey SDLCompat_SetColorKey
#define SDL_SetAlpha SDLCompat_SetAlpha
#define SDL_CreateRGBSurface SDLCompat_CreateRGBSurface
#define SDL_CreateRGBSurfaceFrom SDLCompat_CreateRGBSurfaceFrom
#define SDL_SetColors SDLCompat_SetColors
#define SDL_SetPalette SDLCompat_SetPalette
#define SDL_VideoModeOK SDLCompat_VideoModeOK
#define SDL_WM_IconifyWindow SDLCompat_WM_IconifyWindow
#define SDL_WM_ToggleFullScreen SDLCompat_WM_ToggleFullScreen
#define SDL_WM_GrabInput SDLCompat_WM_GrabInput
#define SDL_WM_GetCaption SDLCompat_WM_GetCaption
#define SDL_WM_SetIcon SDLCompat_WM_SetIcon
#define SDL_WarpMouse SDLCompat_WarpMouse
#define SDL_GetAppState SDLCompat_GetAppState
#define SDL_GetWMInfo SDLCompat_GetWMInfo
#define SDL_Init SDLCompat_Init
#define SDL_InitSubSystem SDLCompat_InitSubSystem
#define SDL_AddTimer SDLCompat_AddTimer
#define SDL_RemoveTimer SDLCompat_RemoveTimer
#define SDL_SetTimer SDLCompat_SetTimer
#define SDL_PollEvent SDLCompat_PollEvent
#define SDL_WaitEvent SDLCompat_WaitEvent
#define SDL_PeepEvents SDLCompat_PeepEvents
#define SDL_PushEvent SDLCompat_PushEvent
#define SDL_EventState SDLCompat_EventState
#define SDL_SetEventFilter SDLCompat_SetEventFilter
#define SDL_GetEventFilter SDLCompat_GetEventFilter
#define SDL_GetMouseState SDLCompat_GetMouseState
#define SDL_GetRelativeMouseState SDLCompat_GetRelativeMouseState
#define SDL_JoystickEventState SDLCompat_JoystickEventState

#endif /* ASC_SDL_COMPAT_SDL_H */
