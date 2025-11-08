#include "SDL.h"
#include "SDL_image.h"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_hints.h>

#include <mutex>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <unordered_map>
#include <cstring>

using std::string;

namespace {

SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
SDL_Texture* gTexture = nullptr;
SDL_Surface* gPrimarySurface = nullptr;
SDL_VideoInfo gVideoInfo = {};
bool gVideoInfoDirty = true;
SDL_version gLinkedVersion = {0, 0, 0};
string gWindowTitle = "Advanced Strategic Command";
string gWindowIconLabel;
bool gIsFullscreen = false;
Uint32 gWindowID = 0;
bool gUnicodeEnabled = false;
bool gTriedDummyVideoDriver = false;

std::mutex gVideoMutex;
std::mutex gTimerMutex;
std::mutex gEventFilterMutex;
SDL_TimerID gSetTimerID = 0;
struct SetTimerContext {
   SDL_TimerCallbackSimple callback = nullptr;
};
SetTimerContext gSetTimerContext;
SDL_EventFilter gLegacyEventFilter = nullptr;

struct TimerBridgeData {
   Uint32 (SDLCALL *callback)(Uint32, void*);
   void* userdata;
};

std::unordered_map<SDL_TimerID, TimerBridgeData*> gTimerCallbacks;

void teardownVideoState()
{
   if (gTexture) {
      SDL_DestroyTexture(gTexture);
      gTexture = nullptr;
   }
   if (gPrimarySurface) {
      SDL_DestroySurface(gPrimarySurface);
      gPrimarySurface = nullptr;
   }
   if (gRenderer) {
      SDL_DestroyRenderer(gRenderer);
      gRenderer = nullptr;
   }
   if (gWindow) {
      SDL_DestroyWindow(gWindow);
      gWindow = nullptr;
   }
   gWindowID = 0;
   gIsFullscreen = false;
   gVideoInfoDirty = true;
}

Uint32 choosePixelFormat(int bpp) {
   switch (bpp) {
      case 8:  return SDL_PIXELFORMAT_INDEX8;
      case 15: return SDL_PIXELFORMAT_XRGB1555;
      case 16: return SDL_PIXELFORMAT_RGB565;
      case 24: return SDL_PIXELFORMAT_RGB24;
      case 32: return SDL_PIXELFORMAT_ARGB8888;
      default: return SDL_PIXELFORMAT_ARGB8888;
   }
}

Uint32 translateWindowFlags(Uint32 oldFlags) {
   Uint32 windowFlags = 0;
   if (oldFlags & SDL_FULLSCREEN)
      windowFlags |= SDL_WINDOW_FULLSCREEN;
   if (oldFlags & SDL_RESIZABLE)
      windowFlags |= SDL_WINDOW_RESIZABLE;
   if (oldFlags & SDL_NOFRAME)
      windowFlags |= SDL_WINDOW_BORDERLESS;
   if (oldFlags & SDL_OPENGL)
      windowFlags |= SDL_WINDOW_OPENGL;
   return windowFlags;
}

SDL_Texture* recreateTexture(int width, int height) {
   if (!gRenderer)
      return nullptr;
   if (gTexture) {
      SDL_DestroyTexture(gTexture);
      gTexture = nullptr;
   }
   gTexture = SDL_CreateTexture(gRenderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                width,
                                height);
   return gTexture;
}

SDL_Surface* recreateSurface(int width, int height, int bpp) {
   Uint32 fmt = choosePixelFormat(bpp);
   if (gPrimarySurface) {
      SDL_DestroySurface(gPrimarySurface);
      gPrimarySurface = nullptr;
   }
   gPrimarySurface = SDL_CreateSurface(width, height, static_cast<SDL_PixelFormat>(fmt));
   gVideoInfoDirty = true;
   return gPrimarySurface;
}

void presentSurface(SDL_Surface* surface) {
   if (!surface || surface != gPrimarySurface || !gRenderer || !gTexture)
      return;

   SDL_Surface* upload = surface;
   SDL_Surface* converted = nullptr;
   if (surface->format != SDL_PIXELFORMAT_ARGB8888) {
      converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888);
      if (converted)
         upload = converted;
   }

   SDL_UpdateTexture(gTexture, nullptr, upload->pixels, upload->pitch);
   SDL_RenderClear(gRenderer);
   SDL_RenderTexture(gRenderer, gTexture, nullptr, nullptr);
   SDL_RenderPresent(gRenderer);

   if (converted)
      SDL_DestroySurface(converted);
}

void ensureVideoInfo(int width, int height) {
   if (!gVideoInfoDirty)
      return;

   SDL_zero(gVideoInfo);
   gVideoInfo.wm_available = 1;
   gVideoInfo.current_w = width;
   gVideoInfo.current_h = height;
   gVideoInfo.vfmt = SDLCompat_BuildSurfacePixelFormat(gPrimarySurface);
   gVideoInfoDirty = false;
}

bool eventMatchesWindow(const SDL3_Event& e)
{
   if (gWindowID == 0)
      return true;

   switch (e.type) {
#if defined(SDL_EVENT_WINDOW_CLOSE_REQUESTED)
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
#endif
      case SDL_EVENT_WINDOW_RESIZED:
      case SDL_EVENT_WINDOW_EXPOSED:
#if defined(SDL_EVENT_WINDOW_SHOWN)
      case SDL_EVENT_WINDOW_SHOWN:
#endif
#if defined(SDL_EVENT_WINDOW_HIDDEN)
      case SDL_EVENT_WINDOW_HIDDEN:
#endif
      case SDL_EVENT_WINDOW_MINIMIZED:
#if defined(SDL_EVENT_WINDOW_MAXIMIZED)
      case SDL_EVENT_WINDOW_MAXIMIZED:
#endif
      case SDL_EVENT_WINDOW_RESTORED:
      case SDL_EVENT_WINDOW_MOUSE_ENTER:
      case SDL_EVENT_WINDOW_MOUSE_LEAVE:
      case SDL_EVENT_WINDOW_FOCUS_GAINED:
      case SDL_EVENT_WINDOW_FOCUS_LOST:
         return e.window.windowID == gWindowID;

      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP:
         return e.key.windowID == gWindowID;

#if defined(SDL_EVENT_TEXT_INPUT)
      case SDL_EVENT_TEXT_INPUT:
      case SDL_EVENT_TEXT_EDITING:
         return e.text.windowID == gWindowID;
#endif

      case SDL_EVENT_MOUSE_MOTION:
         return e.motion.windowID == gWindowID;

      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      case SDL_EVENT_MOUSE_BUTTON_UP:
         return e.button.windowID == gWindowID;

#if defined(SDL_EVENT_MOUSE_WHEEL)
      case SDL_EVENT_MOUSE_WHEEL:
         return e.wheel.windowID == gWindowID;
#endif

#if defined(SDL_EVENT_DROP_FILE)
      case SDL_EVENT_DROP_FILE:
      case SDL_EVENT_DROP_TEXT:
      case SDL_EVENT_DROP_BEGIN:
      case SDL_EVENT_DROP_COMPLETE:
         return e.drop.windowID == gWindowID;
#endif

      case SDL_EVENT_USER:
         return e.user.windowID == gWindowID;

      default:
         return true;
   }
}

Uint16 keycodeToUnicode(SDL_Keycode key)
{
   if (key >= 0 && key <= 0xFFFF)
      return static_cast<Uint16>(key);
   return 0;
}

bool fillActiveEvent(SDL_Event& dst, Uint8 gain, Uint8 state)
{
   dst.type = SDL_ACTIVEEVENT;
   dst.active.type = SDL_ACTIVEEVENT;
   dst.active.gain = gain;
   dst.active.state = state;
   return true;
}

bool convertFromSDL3Event(const SDL3_Event& src, SDL_Event& dst)
{
   if (!eventMatchesWindow(src))
      return false;

   switch (src.type) {
#if defined(SDL_EVENT_WINDOW_CLOSE_REQUESTED)
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
#endif
      case SDL_EVENT_QUIT:
         dst.type = SDL_QUIT;
         dst.quit.type = SDL_QUIT;
         return true;

      case SDL_EVENT_WINDOW_RESIZED:
         dst.type = SDL_VIDEORESIZE;
         dst.resize.type = SDL_VIDEORESIZE;
         dst.resize.w = static_cast<int>(src.window.data1);
         dst.resize.h = static_cast<int>(src.window.data2);
         return true;

      case SDL_EVENT_WINDOW_EXPOSED:
#if defined(SDL_EVENT_WINDOW_SHOWN)
      case SDL_EVENT_WINDOW_SHOWN:
#endif
         dst.type = SDL_VIDEOEXPOSE;
         dst.expose.type = SDL_VIDEOEXPOSE;
         return true;

      case SDL_EVENT_WINDOW_FOCUS_GAINED:
         return fillActiveEvent(dst, 1, SDL_APPINPUTFOCUS);

      case SDL_EVENT_WINDOW_FOCUS_LOST:
         return fillActiveEvent(dst, 0, SDL_APPINPUTFOCUS);

      case SDL_EVENT_WINDOW_MOUSE_ENTER:
         return fillActiveEvent(dst, 1, SDL_APPMOUSEFOCUS);

      case SDL_EVENT_WINDOW_MOUSE_LEAVE:
         return fillActiveEvent(dst, 0, SDL_APPMOUSEFOCUS);

      case SDL_EVENT_WINDOW_RESTORED:
         return fillActiveEvent(dst, 1, SDL_APPACTIVE);

      case SDL_EVENT_WINDOW_MINIMIZED:
#if defined(SDL_EVENT_WINDOW_HIDDEN)
      case SDL_EVENT_WINDOW_HIDDEN:
#endif
         return fillActiveEvent(dst, 0, SDL_APPACTIVE);

      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP: {
         bool pressed = (src.type == SDL_EVENT_KEY_DOWN);
         dst.type = pressed ? SDL_KEYDOWN : SDL_KEYUP;
         dst.key.type = dst.type;
         dst.key.which = static_cast<Uint8>(src.key.which);
         dst.key.state = pressed ? SDL_PRESSED : SDL_RELEASED;
         dst.key.keysym.scancode = static_cast<Uint8>(src.key.scancode);
         dst.key.keysym.sym = static_cast<SDLKey>(src.key.key);
         dst.key.keysym.mod = static_cast<SDLMod>(src.key.mod);
         dst.key.keysym.unicode = gUnicodeEnabled ? keycodeToUnicode(src.key.key) : 0;
         return true;
      }

      case SDL_EVENT_MOUSE_MOTION:
         dst.type = SDL_MOUSEMOTION;
         dst.motion.type = SDL_MOUSEMOTION;
         dst.motion.which = static_cast<Uint8>(src.motion.which);
         dst.motion.state = static_cast<Uint8>(src.motion.state);
         dst.motion.x = static_cast<Uint16>(std::lround(src.motion.x));
         dst.motion.y = static_cast<Uint16>(std::lround(src.motion.y));
         dst.motion.xrel = static_cast<Sint16>(std::lround(src.motion.xrel));
         dst.motion.yrel = static_cast<Sint16>(std::lround(src.motion.yrel));
         return true;

      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      case SDL_EVENT_MOUSE_BUTTON_UP: {
         bool pressed = (src.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
         dst.type = pressed ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
         dst.button.type = dst.type;
         dst.button.which = static_cast<Uint8>(src.button.which);
         dst.button.button = src.button.button;
         dst.button.state = pressed ? SDL_PRESSED : SDL_RELEASED;
         dst.button.x = static_cast<Uint16>(std::lround(src.button.x));
         dst.button.y = static_cast<Uint16>(std::lround(src.button.y));
         return true;
      }

      case SDL_EVENT_JOYSTICK_AXIS_MOTION:
         dst.type = SDL_JOYAXISMOTION;
         dst.jaxis.type = SDL_JOYAXISMOTION;
         dst.jaxis.which = static_cast<Uint8>(src.jaxis.which);
         dst.jaxis.axis = src.jaxis.axis;
         dst.jaxis.value = src.jaxis.value;
         return true;

      case SDL_EVENT_JOYSTICK_BALL_MOTION:
         dst.type = SDL_JOYBALLMOTION;
         dst.jball.type = SDL_JOYBALLMOTION;
         dst.jball.which = static_cast<Uint8>(src.jball.which);
         dst.jball.ball = src.jball.ball;
         dst.jball.xrel = src.jball.xrel;
         dst.jball.yrel = src.jball.yrel;
         return true;

      case SDL_EVENT_JOYSTICK_HAT_MOTION:
         dst.type = SDL_JOYHATMOTION;
         dst.jhat.type = SDL_JOYHATMOTION;
         dst.jhat.which = static_cast<Uint8>(src.jhat.which);
         dst.jhat.hat = src.jhat.hat;
         dst.jhat.value = src.jhat.value;
         return true;

      case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
      case SDL_EVENT_JOYSTICK_BUTTON_UP: {
         bool pressed = (src.type == SDL_EVENT_JOYSTICK_BUTTON_DOWN);
         dst.type = pressed ? SDL_JOYBUTTONDOWN : SDL_JOYBUTTONUP;
         dst.jbutton.type = dst.type;
         dst.jbutton.which = static_cast<Uint8>(src.jbutton.which);
         dst.jbutton.button = src.jbutton.button;
         dst.jbutton.state = pressed ? SDL_PRESSED : SDL_RELEASED;
         return true;
      }

      case SDL_EVENT_USER:
         dst.type = SDL_USEREVENT;
         dst.user.type = SDL_USEREVENT;
         dst.user.code = src.user.code;
         dst.user.data1 = src.user.data1;
         dst.user.data2 = src.user.data2;
         return true;

#if defined(SDL_EVENT_SYSWM)
      case SDL_EVENT_SYSWM:
         dst.type = SDL_SYSWMEVENT;
         dst.syswm.type = SDL_SYSWMEVENT;
         dst.syswm.msg = src.syswm.msg;
         return true;
#endif

      default:
         break;
   }
   return false;
}

bool convertToSDL3Event(const SDL_Event& src, SDL3_Event& dst)
{
   SDL_zero(dst);
   switch (src.type) {
      case SDL_QUIT:
         dst.type = SDL_EVENT_QUIT;
         return true;

      case SDL_KEYDOWN:
      case SDL_KEYUP: {
         bool pressed = (src.type == SDL_KEYDOWN);
         dst.type = pressed ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
         dst.key.windowID = gWindowID;
         dst.key.which = 0;
         dst.key.scancode = static_cast<SDL_Scancode>(src.key.keysym.scancode);
         dst.key.key = static_cast<SDL_Keycode>(src.key.keysym.sym);
         dst.key.mod = static_cast<SDL_Keymod>(src.key.keysym.mod);
         dst.key.raw = static_cast<Uint16>(src.key.keysym.scancode);
         dst.key.down = pressed ? SDL_TRUE : SDL_FALSE;
         dst.key.repeat = 0;
         return true;
      }

      case SDL_MOUSEMOTION:
         dst.type = SDL_EVENT_MOUSE_MOTION;
         dst.motion.windowID = gWindowID;
         dst.motion.which = static_cast<SDL_MouseID>(src.motion.which);
         dst.motion.state = static_cast<SDL_MouseButtonFlags>(src.motion.state);
         dst.motion.x = static_cast<float>(src.motion.x);
         dst.motion.y = static_cast<float>(src.motion.y);
         dst.motion.xrel = static_cast<float>(src.motion.xrel);
         dst.motion.yrel = static_cast<float>(src.motion.yrel);
         return true;

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP: {
         bool pressed = (src.type == SDL_MOUSEBUTTONDOWN);
         dst.type = pressed ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
         dst.button.windowID = gWindowID;
         dst.button.which = static_cast<SDL_MouseID>(src.button.which);
         dst.button.button = src.button.button;
         dst.button.down = pressed ? SDL_TRUE : SDL_FALSE;
         dst.button.clicks = 1;
         dst.button.x = static_cast<float>(src.button.x);
         dst.button.y = static_cast<float>(src.button.y);
         return true;
      }

      case SDL_VIDEORESIZE:
         dst.type = SDL_EVENT_WINDOW_RESIZED;
         dst.window.windowID = gWindowID;
         dst.window.data1 = src.resize.w;
         dst.window.data2 = src.resize.h;
         return true;

      case SDL_VIDEOEXPOSE:
         dst.type = SDL_EVENT_WINDOW_EXPOSED;
         dst.window.windowID = gWindowID;
         return true;

      case SDL_USEREVENT:
         dst.type = SDL_EVENT_USER;
         dst.user.windowID = gWindowID;
         dst.user.code = src.user.code;
         dst.user.data1 = src.user.data1;
         dst.user.data2 = src.user.data2;
         return true;

      default:
         break;
   }
   return false;
}

static bool SDLCALL legacyEventFilterThunk(void*, SDL3_Event* raw)
{
   SDL_EventFilter filter;
   {
      std::lock_guard<std::mutex> guard(gEventFilterMutex);
      filter = gLegacyEventFilter;
   }
   if (!filter)
      return true;

   SDL_Event legacy;
   if (!convertFromSDL3Event(*raw, legacy))
      return true;

   int res = filter(&legacy);
   if (res) {
      SDL3_Event updated;
      if (convertToSDL3Event(legacy, updated))
         *raw = updated;
   }
   return res != 0;
}

} // namespace

/* SDL3 changed SDL_Init/SDL_InitSubSystem to return bool (true=success, false=failure)
   instead of int (0=success, -1=failure). These wrappers maintain SDL 1.2/2.0 semantics. */
int SDLCompat_Init(Uint32 flags)
{
   bool result = SDL3_Init(static_cast<SDL_InitFlags>(flags));
   return result ? 0 : -1;
}

int SDLCompat_InitSubSystem(Uint32 flags)
{
   bool result = SDL3_InitSubSystem(static_cast<SDL_InitFlags>(flags));
   return result ? 0 : -1;
}

int SDLCompat_PollEvent(SDL_Event* event)
{
   SDL3_Event raw;
   while (SDL3_PollEvent(&raw)) {
      SDL_Event converted;
      if (convertFromSDL3Event(raw, converted)) {
         if (event)
            *event = converted;
         return 1;
      }
   }
   return 0;
}

int SDLCompat_WaitEvent(SDL_Event* event)
{
   SDL3_Event raw;
   while (SDL3_WaitEvent(&raw)) {
      SDL_Event converted;
      if (convertFromSDL3Event(raw, converted)) {
         if (event)
            *event = converted;
         return 1;
      }
   }
   return 0;
}

int SDLCompat_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, Uint32 mask)
{
   if (numevents <= 0)
      return 0;

   if (action == SDL_ADDEVENT) {
      if (!events)
         return 0;
      int added = 0;
      for (int i = 0; i < numevents; ++i) {
         SDL3_Event raw;
         if (convertToSDL3Event(events[i], raw)) {
            if (SDL3_PushEvent(&raw) == 0)
               ++added;
         }
      }
      return added;
   }

   const bool peekOnly = (action == SDL_PEEKEVENT);
   std::vector<std::pair<SDL3_Event,bool>> pulled;
   pulled.reserve(static_cast<size_t>(numevents) * 2);
   int matched = 0;

   auto matchesMask = [mask](Uint32 type) {
      if (mask == 0xFFFFFFFFu)
         return true;
      if (mask == 0)
         return false;
      if (type >= 32)
         return true;
      return (mask & (1u << type)) != 0;
   };

   SDL3_Event raw;
   while (matched < numevents && SDL3_PollEvent(&raw)) {
      SDL_Event converted;
      bool convertible = convertFromSDL3Event(raw, converted);
      if (!convertible)
         continue;

      pulled.emplace_back(raw, false);

      if (!matchesMask(converted.type))
         continue;

      if (events)
         events[matched] = converted;
      ++matched;

      if (!peekOnly)
         pulled.back().second = true; // consume event
   }

   // Requeue events in reverse order to preserve original sequence.
   for (auto it = pulled.rbegin(); it != pulled.rend(); ++it) {
      if (peekOnly || !it->second)
         SDL3_PushEvent(&it->first);
   }

   return matched;
}

int SDLCompat_PushEvent(SDL_Event* event)
{
   if (!event)
      return -1;
   SDL3_Event raw;
   if (!convertToSDL3Event(*event, raw))
      return -1;
   return SDL3_PushEvent(&raw);
}

static Uint32 mapLegacyTypeToSDL3(Uint8 type)
{
   switch (type) {
      case SDL_KEYDOWN:
         return SDL_EVENT_KEY_DOWN;
      case SDL_KEYUP:
         return SDL_EVENT_KEY_UP;
      case SDL_ACTIVEEVENT:
         return SDL_EVENT_WINDOW_FOCUS_GAINED;
      case SDL_MOUSEMOTION:
         return SDL_EVENT_MOUSE_MOTION;
      case SDL_MOUSEBUTTONDOWN:
         return SDL_EVENT_MOUSE_BUTTON_DOWN;
      case SDL_MOUSEBUTTONUP:
         return SDL_EVENT_MOUSE_BUTTON_UP;
      case SDL_VIDEORESIZE:
         return SDL_EVENT_WINDOW_RESIZED;
      case SDL_VIDEOEXPOSE:
         return SDL_EVENT_WINDOW_EXPOSED;
      case SDL_QUIT:
         return SDL_EVENT_QUIT;
      case SDL_USEREVENT:
         return SDL_EVENT_USER;
#if defined(SDL_EVENT_SYSWM)
      case SDL_SYSWMEVENT:
         return SDL_EVENT_SYSWM;
#endif
      default:
         return static_cast<Uint32>(type);
   }
}

Uint8 SDLCompat_EventState(Uint8 type, int state)
{
   Uint32 native = mapLegacyTypeToSDL3(type);
   if (state == SDL_QUERY)
      return SDL_EventEnabled(native) ? SDL_ENABLE : SDL_IGNORE;

   bool enable = (state != SDL_DISABLE && state != SDL_IGNORE);
   SDL_SetEventEnabled(native, enable);
   return enable ? SDL_ENABLE : SDL_IGNORE;
}

void SDLCompat_SetEventFilter(SDL_EventFilter filter)
{
   {
      std::lock_guard<std::mutex> guard(gEventFilterMutex);
      gLegacyEventFilter = filter;
   }

   if (filter)
      SDL3_SetEventFilter(legacyEventFilterThunk, nullptr);
   else
      SDL3_SetEventFilter(nullptr, nullptr);
}

SDL_EventFilter SDLCompat_GetEventFilter(void)
{
   std::lock_guard<std::mutex> guard(gEventFilterMutex);
   return gLegacyEventFilter;
}

SDL_CompatPixelFormat SDLCompat_BuildPixelFormat(SDL_PixelFormat format)
{
   SDL_CompatPixelFormat out = {};
   const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(format);
   if (details) {
      out.BitsPerPixel = details->bits_per_pixel;
      out.BytesPerPixel = details->bytes_per_pixel;
      auto clampLoss = [](Uint8 bits) -> Uint8 {
         return (bits >= 8) ? 0 : static_cast<Uint8>(8 - bits);
      };
      out.Rloss = clampLoss(details->Rbits);
      out.Gloss = clampLoss(details->Gbits);
      out.Bloss = clampLoss(details->Bbits);
      out.Aloss = clampLoss(details->Abits);
      out.Rshift = details->Rshift;
      out.Gshift = details->Gshift;
      out.Bshift = details->Bshift;
      out.Ashift = details->Ashift;
      out.Rmask = details->Rmask;
      out.Gmask = details->Gmask;
      out.Bmask = details->Bmask;
      out.Amask = details->Amask;
   }
   out.palette = nullptr;
   out.colorkey = 0;
   out.alpha = 255;
   return out;
}

SDL_CompatPixelFormat SDLCompat_BuildSurfacePixelFormat(const SDL_Surface* surface)
{
   SDL_PixelFormat format = surface ? surface->format : SDL_PIXELFORMAT_UNKNOWN;
   SDL_CompatPixelFormat out = SDLCompat_BuildPixelFormat(format);
   if (surface) {
      SDL_Surface* mutableSurface = const_cast<SDL_Surface*>(surface);
      out.palette = SDL_GetSurfacePalette(mutableSurface);
      Uint32 key = 0;
      if (SDL_GetSurfaceColorKey(mutableSurface, &key))
         out.colorkey = key;
      else
         out.colorkey = 0;
      Uint8 alpha = 255;
      if (SDL_GetSurfaceAlphaMod(mutableSurface, &alpha))
         out.alpha = alpha;
      else
         out.alpha = 255;
   }
   return out;
}

Uint8 SDLCompat_GetMouseState(int* x, int* y)
{
   float fx = 0.0f;
   float fy = 0.0f;
#if defined(SDL_GetMouseState)
#define SDL_COMPAT_RESTORE_GETMOUSESTATE 1
#undef SDL_GetMouseState
#endif
   Uint32 buttons = SDL_GetMouseState(&fx, &fy);
#if defined(SDL_COMPAT_RESTORE_GETMOUSESTATE)
#undef SDL_COMPAT_RESTORE_GETMOUSESTATE
#define SDL_GetMouseState SDLCompat_GetMouseState
#endif
   if (x)
      *x = static_cast<int>(fx);
   if (y)
      *y = static_cast<int>(fy);
   return static_cast<Uint8>(buttons);
}

Uint8 SDLCompat_GetRelativeMouseState(int* x, int* y)
{
   float fx = 0.0f;
   float fy = 0.0f;
#if defined(SDL_GetRelativeMouseState)
#define SDL_COMPAT_RESTORE_GETRELATIVEMOUSESTATE 1
#undef SDL_GetRelativeMouseState
#endif
   Uint32 buttons = SDL_GetRelativeMouseState(&fx, &fy);
#if defined(SDL_COMPAT_RESTORE_GETRELATIVEMOUSESTATE)
#undef SDL_COMPAT_RESTORE_GETRELATIVEMOUSESTATE
#define SDL_GetRelativeMouseState SDLCompat_GetRelativeMouseState
#endif
   if (x)
      *x = static_cast<int>(fx);
   if (y)
      *y = static_cast<int>(fy);
   return static_cast<Uint8>(buttons);
}

int SDLCompat_JoystickEventState(int state)
{
   if (state == SDL_QUERY)
      return SDL_JoystickEventsEnabled() ? SDL_ENABLE : SDL_DISABLE;
   if (state == SDL_ENABLE)
      SDL_SetJoystickEventsEnabled(SDL_TRUE);
   else if (state == SDL_DISABLE)
      SDL_SetJoystickEventsEnabled(SDL_FALSE);
   return SDL_JoystickEventsEnabled() ? SDL_ENABLE : SDL_DISABLE;
}

SDL_Surface* SDLCompat_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);

   if (!SDL_WasInit(SDL_INIT_VIDEO)) {
      if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
         fprintf(stderr, "[ASC SDL3] SDL_InitSubSystem failed: %s\n", SDL_GetError());
         fflush(stderr);
         return nullptr;
      }
   }

   Uint32 windowFlags = translateWindowFlags(flags);
   bool attemptedDummyFallback = false;

   int availableDrivers = SDL_GetNumVideoDrivers();
   std::vector<std::string> driverNames;
   driverNames.reserve(static_cast<size_t>(availableDrivers));
   for (int i = 0; i < availableDrivers; ++i) {
      const char* name = SDL_GetVideoDriver(i);
      if (name)
         driverNames.emplace_back(name);
   }
   if (!driverNames.empty()) {
      fprintf(stderr, "[ASC SDL3] available SDL video drivers:");
      for (const auto& name : driverNames)
         fprintf(stderr, " %s", name.c_str());
      fprintf(stderr, "\n");
      fflush(stderr);
   } else {
      fprintf(stderr, "[ASC SDL3] no SDL video drivers reported via SDL_GetNumVideoDrivers\n");
      fflush(stderr);
   }

   const char* currentDriverCStr = SDL_GetCurrentVideoDriver();
   std::string activeDriver = currentDriverCStr ? currentDriverCStr : std::string();

   auto ensureWindow = [&](int w, int h, Uint32 sdlFlags, const std::string& driverName) -> bool {
      if (!gWindow) {
         gWindow = SDL_CreateWindow(gWindowTitle.c_str(), w, h, sdlFlags);
         if (!gWindow) {
            const char* err = SDL_GetError();
            fprintf(stderr, "[ASC SDL3] SDL_CreateWindow failed (driver='%s'): %s\n",
                    driverName.empty() ? "none" : driverName.c_str(),
                    (err && *err) ? err : "(no error reported)");
            fflush(stderr);
            return false;
         } else {
            fprintf(stderr, "[ASC SDL3] SDL_CreateWindow succeeded (driver='%s')\n",
                    driverName.empty() ? "none" : driverName.c_str());
            fflush(stderr);
         }
      } else {
         SDL_SetWindowSize(gWindow, w, h);
         bool wantFullscreen = (sdlFlags & SDL_WINDOW_FULLSCREEN) != 0;
         if (wantFullscreen != gIsFullscreen) {
            SDL_SetWindowFullscreen(gWindow, wantFullscreen);
            gIsFullscreen = wantFullscreen;
         }
      }
      return true;
   };

   bool windowReady = ensureWindow(width, height, windowFlags, activeDriver);

   if (!windowReady && !driverNames.empty()) {
      std::vector<std::string> reinitDrivers;
      reinitDrivers.reserve(driverNames.size());
      for (const auto& candidate : driverNames) {
         if (candidate == "dummy")
            continue;
         if (!activeDriver.empty() && candidate == activeDriver)
            continue;
         reinitDrivers.push_back(candidate);
      }

      for (const auto& driverCandidate : reinitDrivers) {
         SDL_QuitSubSystem(SDL_INIT_VIDEO);
         teardownVideoState();
         SDL_SetHint(SDL_HINT_VIDEO_DRIVER, driverCandidate.c_str());
         if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
            fprintf(stderr, "[ASC SDL3] SDL_InitSubSystem retry with driver '%s' failed: %s\n",
                    driverCandidate.c_str(), SDL_GetError());
            fflush(stderr);
            continue;
         }
         const char* retryDriver = SDL_GetCurrentVideoDriver();
         if (retryDriver && *retryDriver)
            activeDriver = retryDriver;
         else
            activeDriver = driverCandidate;

         windowReady = ensureWindow(width, height, windowFlags, activeDriver);
         if (windowReady)
            break;
      }
   }

   if (!windowReady) {
      const char* configuredDriver = SDL_getenv("SDL_VIDEODRIVER");
      attemptedDummyFallback = true;
      bool allowDummyFallback = SDL_getenv("ASC_FORCE_DUMMY_VIDEO") != nullptr;
      if (!activeDriver.empty() && activeDriver != "dummy" && !allowDummyFallback) {
         fprintf(stderr, "[ASC SDL3] skipping dummy fallback; video driver '%s' is available\n",
                 activeDriver.c_str());
         fflush(stderr);
      } else if (!gTriedDummyVideoDriver && (!configuredDriver || configuredDriver[0] == '\0' || allowDummyFallback)) {
         gTriedDummyVideoDriver = true;
         fprintf(stderr, "[ASC SDL3] attempting fallback to SDL dummy video driver\n");
         fflush(stderr);
         SDL_QuitSubSystem(SDL_INIT_VIDEO);
         if (SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy") != SDL_TRUE) {
            fprintf(stderr, "[ASC SDL3] SDL_SetHint(SDL_HINT_VIDEO_DRIVER,\"dummy\") failed\n");
            fflush(stderr);
         }
         if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
            fprintf(stderr, "[ASC SDL3] SDL_InitSubSystem retry failed: %s\n", SDL_GetError());
            fflush(stderr);
         } else if (!ensureWindow(width, height, windowFlags, "dummy")) {
            fprintf(stderr, "[ASC SDL3] SDL_CreateWindow still failed with dummy driver: %s\n", SDL_GetError());
            fflush(stderr);
         }
      } else {
         fprintf(stderr, "[ASC SDL3] no usable SDL video driver (set SDL_VIDEODRIVER or install a windowing backend)\n");
         fflush(stderr);
      }
   }

   if (!gWindow) {
      fprintf(stderr, "[ASC SDL3] unable to create window; aborting video mode setup\n");
      fflush(stderr);
      return nullptr;
   }

   if (gWindow) {
      gWindowID = SDL_GetWindowID(gWindow);

      if (!gRenderer) {
         gRenderer = SDL_CreateRenderer(gWindow, nullptr);
         if (!gRenderer) {
            fprintf(stderr, "[ASC SDL3] SDL_CreateRenderer (default) failed: %s\n", SDL_GetError());
            fflush(stderr);
            SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
            gRenderer = SDL_CreateRenderer(gWindow, nullptr);
         }

         if (!gRenderer) {
            fprintf(stderr, "[ASC SDL3] SDL_CreateRenderer (software hint) failed: %s\n", SDL_GetError());
            fflush(stderr);
            SDL_DestroyWindow(gWindow);
            gWindow = nullptr;
            gWindowID = 0;
         } else {
            fprintf(stderr, "[ASC SDL3] SDL_CreateRenderer succeeded\n");
            fflush(stderr);
            if (flags & SDL_DOUBLEBUF)
               SDL_SetRenderVSync(gRenderer, 1);
         }
      }

      if (gRenderer) {
         if (!recreateTexture(width, height)) {
            fprintf(stderr, "[ASC SDL3] recreateTexture failed: %s\n", SDL_GetError());
            fflush(stderr);
            SDL_DestroyRenderer(gRenderer);
            gRenderer = nullptr;
            SDL_DestroyWindow(gWindow);
            gWindow = nullptr;
            gWindowID = 0;
         }
      }
   }

   if (!gWindow || !gRenderer) {
      teardownVideoState();
      fprintf(stderr, "[ASC SDL3] video initialization incomplete; returning failure\n");
      fflush(stderr);
      return nullptr;
   }

   const char* finalDriver = SDL_GetCurrentVideoDriver();
   fprintf(stderr, "[ASC SDL3] final SDL video driver: %s\n", finalDriver ? finalDriver : "(null)");
   fflush(stderr);
   if (!finalDriver || std::strcmp(finalDriver, "dummy") == 0 || std::strcmp(finalDriver, "offscreen") == 0) {
      fprintf(stderr, "[ASC SDL3] SDL video driver '%s' is headless; set SDL_VIDEODRIVER to a windowing backend\n",
              finalDriver ? finalDriver : "(null)");
      fflush(stderr);
      teardownVideoState();
      return nullptr;
   }

   SDL_Surface* surface = recreateSurface(width, height, bpp);
   if (!surface) {
      fprintf(stderr, "[ASC SDL3] recreateSurface failed: %s\n", SDL_GetError());
      fflush(stderr);
      return nullptr;
   }

   ensureVideoInfo(width, height);
   gIsFullscreen = (windowFlags & SDL_WINDOW_FULLSCREEN) != 0;
   return surface;
}

SDL_Surface* SDLCompat_GetVideoSurface(void)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);
   return gPrimarySurface;
}

void SDLCompat_UpdateRect(SDL_Surface* surface, Sint32, Sint32, Sint32, Sint32)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);
   presentSurface(surface);
}

void SDLCompat_UpdateRects(SDL_Surface* surface, int, SDL_Rect*)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);
   presentSurface(surface);
}

int SDLCompat_Flip(SDL_Surface* surface)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);
   presentSurface(surface);
   return 0;
}

void SDLCompat_WM_SetCaption(const char* title, const char* icon)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);
   if (title)
      gWindowTitle = title;
   else
      gWindowTitle = "Advanced Strategic Command";

   if (icon)
      gWindowIconLabel = icon;
   else
      gWindowIconLabel.clear();

   if (gWindow)
      SDL_SetWindowTitle(gWindow, gWindowTitle.c_str());
}

int SDLCompat_EnableUNICODE(int enable)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);
   int previous = gUnicodeEnabled ? 1 : 0;
   if (enable >= 0)
      gUnicodeEnabled = (enable != 0);
   return previous;
}

int SDLCompat_EnableKeyRepeat(int, int)
{
   /* SDL3 does not expose configurable key repeat; report success. */
   return 0;
}

Uint8* SDLCompat_GetKeyState(int* numkeys)
{
   const bool* state = SDL_GetKeyboardState(numkeys);
   return const_cast<Uint8*>(reinterpret_cast<const Uint8*>(state));
}

Uint8 SDLCompat_IsKeyPressed(SDLKey key)
{
   SDL_Scancode sc = SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(key), nullptr);
   if (sc == SDL_SCANCODE_UNKNOWN)
      return 0;

   int num = 0;
   const bool* state = SDL_GetKeyboardState(&num);
   if (!state || sc < 0 || sc >= num)
      return 0;
   return state[sc] ? 1 : 0;
}

const SDL_VideoInfo* SDLCompat_GetVideoInfo(void)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);
   if (gPrimarySurface) {
      ensureVideoInfo(gPrimarySurface->w, gPrimarySurface->h);
   }
   return &gVideoInfo;
}

SDL_Rect** SDLCompat_ListModes(SDL_PixelFormat*, Uint32)
{
   static std::vector<SDL_Rect> storedModes;
   static std::vector<SDL_Rect*> modePointers;

   storedModes.clear();
   modePointers.clear();

   int displayCount = 0;
   SDL_DisplayID* displays = SDL_GetDisplays(&displayCount);
   if (!displays) {
      fprintf(stderr, "[ASC SDL3] SDL_GetDisplays failed: %s\n", SDL_GetError());
      return reinterpret_cast<SDL_Rect**>(-1);
   }

   for (int i = 0; i < displayCount; ++i) {
      int modeCount = 0;
      SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(displays[i], &modeCount);
      if (!modes)
         continue;

      for (int j = 0; j < modeCount; ++j) {
         SDL_DisplayMode* mode = modes[j];
         if (!mode)
            continue;
         if (mode->w <= 0 || mode->h <= 0)
            continue;
         SDL_Rect rect = {0, 0, mode->w, mode->h};
         storedModes.push_back(rect);
      }
      SDL_free(modes);
   }

   SDL_free(displays);

   if (storedModes.empty())
      return reinterpret_cast<SDL_Rect**>(-1);

   std::sort(storedModes.begin(), storedModes.end(), [](const SDL_Rect& a, const SDL_Rect& b) {
      if (a.w != b.w)
         return a.w > b.w;
      return a.h > b.h;
   });

   storedModes.erase(std::unique(storedModes.begin(), storedModes.end(),
                                 [](const SDL_Rect& a, const SDL_Rect& b) {
                                    return a.w == b.w && a.h == b.h;
                                 }),
                     storedModes.end());

   modePointers.reserve(storedModes.size() + 1);
   for (SDL_Rect& rect : storedModes)
      modePointers.push_back(&rect);
   modePointers.push_back(nullptr);
   return modePointers.data();
}

const char* SDLCompat_VideoDriverName(char* namebuf, int maxlen)
{
   const char* driver = SDL_GetCurrentVideoDriver();
   if (!driver)
      driver = "unknown";
   if (namebuf && maxlen > 0) {
      SDL_strlcpy(namebuf, driver, static_cast<size_t>(maxlen));
   }
   return driver;
}

const SDL_version* SDLCompat_Linked_Version(void)
{
   int linked = SDL_GetVersion();
   gLinkedVersion.major = static_cast<Uint8>(SDL_VERSIONNUM_MAJOR(linked));
   gLinkedVersion.minor = static_cast<Uint8>(SDL_VERSIONNUM_MINOR(linked));
   gLinkedVersion.patch = static_cast<Uint8>(SDL_VERSIONNUM_MICRO(linked));
   return &gLinkedVersion;
}

#pragma push_macro("SDL_AddTimer")
#undef SDL_AddTimer
#pragma push_macro("SDL_RemoveTimer")
#undef SDL_RemoveTimer

static Uint32 SDLCALL SDLCompat_TimerThunk(void* userdata, SDL_TimerID timerID, Uint32 interval)
{
   auto* data = static_cast<TimerBridgeData*>(userdata);
   if (!data || !data->callback)
      return 0;

   Uint32 next = data->callback(interval, data->userdata);
   if (next == 0) {
      std::lock_guard<std::mutex> guard(gTimerMutex);
      gTimerCallbacks.erase(timerID);
      delete data;
   }
   return next;
}

static Uint32 SDLCALL SDLCompat_SetTimerBridge(void* userdata, SDL_TimerID timerID, Uint32 interval)
{
   (void)timerID;
   auto* ctx = static_cast<SetTimerContext*>(userdata);
   if (!ctx || !ctx->callback)
      return 0;
   Uint32 next = ctx->callback(interval);
   if (next == 0) {
      std::lock_guard<std::mutex> guard(gTimerMutex);
      gSetTimerID = 0;
      ctx->callback = nullptr;
   }
   return next;
}

SDL_TimerID SDLCompat_AddTimer(Uint32 interval, Uint32 (SDLCALL *callback)(Uint32, void*), void* param)
{
   auto* data = new TimerBridgeData{callback, param};
   SDL_TimerID id = SDL_AddTimer(interval, SDLCompat_TimerThunk, data);
   if (!id) {
      delete data;
      return 0;
   }
   std::lock_guard<std::mutex> guard(gTimerMutex);
   gTimerCallbacks[id] = data;
   return id;
}

int SDLCompat_RemoveTimer(SDL_TimerID id)
{
   TimerBridgeData* data = nullptr;
   {
      std::lock_guard<std::mutex> guard(gTimerMutex);
      auto it = gTimerCallbacks.find(id);
      if (it != gTimerCallbacks.end()) {
         data = it->second;
         gTimerCallbacks.erase(it);
      }
   }
   bool removed = SDL_RemoveTimer(id);
   if (data)
      delete data;
   return removed ? 1 : 0;
}

int SDLCompat_SetTimer(Uint32 interval, SDL_TimerCallbackSimple callback)
{
   std::lock_guard<std::mutex> guard(gTimerMutex);

   if (gSetTimerID) {
      SDL_RemoveTimer(gSetTimerID);
      gSetTimerID = 0;
      gSetTimerContext.callback = nullptr;
   }

   if (interval == 0 || callback == nullptr)
      return 0;

   gSetTimerContext.callback = callback;
   SDL_TimerID id = SDL_AddTimer(interval, SDLCompat_SetTimerBridge, &gSetTimerContext);
   if (!id) {
      gSetTimerContext.callback = nullptr;
      return -1;
   }
   gSetTimerID = id;
   return 0;
}

#pragma pop_macro("SDL_RemoveTimer")
#pragma pop_macro("SDL_AddTimer")

namespace {

SDL_Surface* convertToDisplayFormat(SDL_Surface* surface, bool forceAlpha)
{
   if (!surface)
      return nullptr;

   SDL_PixelFormat targetFormat = SDL_PIXELFORMAT_UNKNOWN;
   if (gPrimarySurface)
      targetFormat = gPrimarySurface->format;

   if (targetFormat == SDL_PIXELFORMAT_UNKNOWN)
      targetFormat = forceAlpha ? SDL_PIXELFORMAT_ARGB8888 : surface->format;

   const SDL_PixelFormatDetails* targetDetails = SDL_GetPixelFormatDetails(targetFormat);
   if (forceAlpha && targetDetails && targetDetails->bytes_per_pixel < 4)
      targetFormat = SDL_PIXELFORMAT_ARGB8888;

   SDL_Surface* converted = SDL_ConvertSurface(surface, targetFormat);
   if (!converted) {
      converted = SDL_DuplicateSurface(surface);
      if (!converted)
         return nullptr;
   }

   if (forceAlpha)
      SDL_SetSurfaceBlendMode(converted, SDL_BLENDMODE_BLEND);
   else
      SDL_SetSurfaceBlendMode(converted, SDL_BLENDMODE_NONE);

   return converted;
}

} // namespace

SDL_Surface* SDLCompat_DisplayFormat(SDL_Surface* surface)
{
   return convertToDisplayFormat(surface, false);
}

SDL_Surface* SDLCompat_DisplayFormatAlpha(SDL_Surface* surface)
{
   return convertToDisplayFormat(surface, true);
}

SDL_Rect SDLCompat_GetClipRect(SDL_Surface* surface)
{
   SDL_Rect rect = {0, 0, 0, 0};
   if (!surface)
      return rect;
   SDL_GetSurfaceClipRect(surface, &rect);
   return rect;
}

SDL_Surface* SDLCompat_IMG_Load_RW(SDL_RWops* src, int freesrc)
{
   return IMG_Load_IO(src, freesrc != 0);
}

SDL_Surface* SDLCompat_IMG_LoadPNG_RW(SDL_RWops* src)
{
   return IMG_LoadPNG_IO(src);
}

SDL_Surface* SDLCompat_IMG_LoadPCX_RW(SDL_RWops* src)
{
   return IMG_LoadPCX_IO(src);
}

int SDLCompat_SetColorKey(SDL_Surface* surface, int flag, Uint32 key)
{
   if (!surface)
      return -1;
   return SDL_SetSurfaceColorKey(surface, (flag & SDL_SRCCOLORKEY) != 0, key);
}

int SDLCompat_SetAlpha(SDL_Surface* surface, Uint32 flag, Uint8 alpha)
{
   if (!surface)
      return -1;

   SDL_SetSurfaceRLE(surface, (flag & SDL_RLEACCEL) != 0);
   SDL_SetSurfaceAlphaMod(surface, alpha);
   SDL_SetSurfaceBlendMode(surface, (flag & SDL_SRCALPHA) ? SDL_BLENDMODE_BLEND
                                                         : SDL_BLENDMODE_NONE);
   return 0;
}

static SDL_PixelFormat masksToFormat(int depth, Uint32 rmask, Uint32 gmask, Uint32 bmask, Uint32 amask)
{
   if (depth == 8 && rmask == 0 && gmask == 0 && bmask == 0)
      return SDL_PIXELFORMAT_INDEX8;

   SDL_PixelFormat format = SDL_GetPixelFormatForMasks(depth, rmask, gmask, bmask, amask);
   if (format == SDL_PIXELFORMAT_UNKNOWN)
      format = static_cast<SDL_PixelFormat>(choosePixelFormat(depth));
   return format;
}

SDL_Surface* SDLCompat_CreateRGBSurface(Uint32 flags, int width, int height, int depth,
                                        Uint32 rmask, Uint32 gmask, Uint32 bmask, Uint32 amask)
{
   SDL_PixelFormat format = masksToFormat(depth, rmask, gmask, bmask, amask);
   SDL_Surface* surface = SDL_CreateSurface(width, height, static_cast<SDL_PixelFormat>(format));
   if (!surface)
      return nullptr;

   if (flags & SDL_SRCALPHA)
      SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);

   return surface;
}

SDL_Surface* SDLCompat_CreateRGBSurfaceFrom(void* pixels, int width, int height, int depth,
                                            int pitch, Uint32 rmask, Uint32 gmask,
                                            Uint32 bmask, Uint32 amask)
{
   SDL_PixelFormat format = masksToFormat(depth, rmask, gmask, bmask, amask);
   return SDL_CreateSurfaceFrom(width, height, static_cast<SDL_PixelFormat>(format), pixels, pitch);
}

int SDLCompat_SetColors(SDL_Surface* surface, SDL_Color* colors, int firstcolor, int ncolors)
{
   if (!surface)
      return -1;

   SDL_Surface* mutableSurface = surface;
   SDL_Palette* palette = SDL_GetSurfacePalette(mutableSurface);
   if (!palette) {
      SDL_Palette* created = SDL_CreatePalette(256);
      if (!created)
         return -1;
      if (SDL_SetSurfacePalette(mutableSurface, created) != SDL_TRUE) {
         SDL_DestroyPalette(created);
         palette = SDL_GetSurfacePalette(mutableSurface);
         if (!palette)
            return -1;
      } else {
         palette = created;
      }
   }

   const int requiredColors = firstcolor + ncolors;
   if (requiredColors > palette->ncolors) {
      SDL_Palette* resized = SDL_CreatePalette(std::max(requiredColors, 256));
      if (!resized)
         return -1;
      if (SDL_SetSurfacePalette(mutableSurface, resized) != SDL_TRUE) {
         SDL_DestroyPalette(resized);
         palette = SDL_GetSurfacePalette(mutableSurface);
         if (!palette)
            return -1;
      } else {
         palette = resized;
      }
   }

   return SDL_SetPaletteColors(palette, colors, firstcolor, ncolors);
}

int SDLCompat_SetPalette(SDL_Surface* surface, int, SDL_Color* colors, int firstcolor, int ncolors)
{
   return SDLCompat_SetColors(surface, colors, firstcolor, ncolors);
}

int SDLCompat_VideoModeOK(int width, int height, int bpp, Uint32)
{
   (void)width;
   (void)height;
   return bpp;
}

int SDLCompat_WM_IconifyWindow(void)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);
   if (!gWindow)
      return 0;
   SDL_MinimizeWindow(gWindow);
   return 1;
}

int SDLCompat_WM_ToggleFullScreen(SDL_Surface*)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);
   if (!gWindow)
      return 0;

   bool newState = !gIsFullscreen;
   if (SDL_SetWindowFullscreen(gWindow, newState) == 0) {
      gIsFullscreen = newState;
      if (!newState && gPrimarySurface)
         SDL_SetWindowSize(gWindow, gPrimarySurface->w, gPrimarySurface->h);
      return 1;
   }
   return 0;
}

SDL_GrabMode SDLCompat_WM_GrabInput(SDL_GrabMode mode)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);
   if (!gWindow)
      return SDL_GRAB_OFF;

   if (mode == SDL_GRAB_QUERY) {
      bool grabbing = SDL_GetWindowMouseGrab(gWindow) || SDL_GetWindowKeyboardGrab(gWindow);
      return grabbing ? SDL_GRAB_ON : SDL_GRAB_OFF;
   }

   bool grab = (mode == SDL_GRAB_ON);
   SDL_SetWindowMouseGrab(gWindow, grab);
   SDL_SetWindowKeyboardGrab(gWindow, grab);
   return grab ? SDL_GRAB_ON : SDL_GRAB_OFF;
}

void SDLCompat_WM_GetCaption(char** title, char** icon)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);
   if (title)
      *title = gWindowTitle.empty() ? nullptr : const_cast<char*>(gWindowTitle.c_str());
   if (icon)
      *icon = gWindowIconLabel.empty() ? nullptr : const_cast<char*>(gWindowIconLabel.c_str());
}

void SDLCompat_WM_SetIcon(SDL_Surface* icon, Uint8*)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);
   if (gWindow && icon)
      SDL_SetWindowIcon(gWindow, icon);
}

void SDLCompat_WarpMouse(Uint16 x, Uint16 y)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);
   if (gWindow)
      SDL_WarpMouseInWindow(gWindow, x, y);
}

Uint8 SDLCompat_GetAppState(void)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);
   if (!gWindow)
      return 0;

   Uint32 flags = SDL_GetWindowFlags(gWindow);
   Uint8 state = 0;
   if (!(flags & SDL_WINDOW_MINIMIZED))
      state |= SDL_APPACTIVE;
   if (flags & SDL_WINDOW_INPUT_FOCUS)
      state |= SDL_APPINPUTFOCUS;
   if (flags & SDL_WINDOW_MOUSE_FOCUS)
      state |= SDL_APPMOUSEFOCUS;
   return state;
}

int SDLCompat_GetWMInfo(SDL_SysWMinfo* info)
{
   std::lock_guard<std::mutex> guard(gVideoMutex);
   (void)info;
   (void)gWindow;
   return 0;
}

SDL_RWops* SDLCompat_RWFromFile(const char* file, const char* mode)
{
   return SDL_IOFromFile(file, mode);
}

Sint64 SDLCompat_RWseek(SDL_RWops* context, Sint64 offset, int whence)
{
   SDL_IOWhence ioWhence = SDL_IO_SEEK_SET;
   if (whence == RW_SEEK_SET || whence == SEEK_SET) {
      ioWhence = SDL_IO_SEEK_SET;
   } else if (whence == RW_SEEK_CUR || whence == SEEK_CUR) {
      ioWhence = SDL_IO_SEEK_CUR;
   } else if (whence == RW_SEEK_END || whence == SEEK_END) {
      ioWhence = SDL_IO_SEEK_END;
   } else {
      SDL_SetError("Invalid seek whence %d", whence);
      return -1;
   }
   return SDL_SeekIO(context, offset, ioWhence);
}

size_t SDLCompat_RWread(SDL_RWops* context, void* ptr, size_t size, size_t maxnum)
{
   if (!context || !ptr || size == 0 || maxnum == 0)
      return 0;
   size_t total = size * maxnum;
   size_t bytes = SDL_ReadIO(context, ptr, total);
   if (bytes == 0)
      return 0;
   return bytes / size;
}

size_t SDLCompat_RWwrite(SDL_RWops* context, const void* ptr, size_t size, size_t num)
{
   if (!context || !ptr || size == 0 || num == 0)
      return 0;
   size_t total = size * num;
   size_t bytes = SDL_WriteIO(context, ptr, total);
   if (bytes == 0)
      return 0;
   return bytes / size;
}

Sint64 SDLCompat_RWtell(SDL_RWops* context)
{
   return SDL_TellIO(context);
}

Sint64 SDLCompat_RWsize(SDL_RWops* context)
{
   return SDL_GetIOSize(context);
}

int SDLCompat_RWclose(SDL_RWops* context)
{
   if (!context)
      return 0;
   return SDL_CloseIO(context) ? 0 : -1;
}

int SDLCompat_mutexP(SDL_Mutex* mutex)
{
   if (!mutex)
      return -1;
   SDL_LockMutex(mutex);
   return 0;
}

int SDLCompat_mutexV(SDL_Mutex* mutex)
{
   if (!mutex)
      return -1;
   SDL_UnlockMutex(mutex);
   return 0;
}
