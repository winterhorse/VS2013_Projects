#ifndef __eglplatform_h_
#define __eglplatform_h_

/*
** Copyright (c) 2007-2009 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

/* Platform-specific types and definitions for egl.h
 * $Revision: 12306 $ on $Date: 2010-08-25 09:51:28 -0700 (Wed, 25 Aug 2010) $
 *
 * Adopters may modify khrplatform.h and this file to suit their platform.
 * You are encouraged to submit all modifications to the Khronos group so that
 * they can be included in future versions of this file.  Please submit changes
 * by sending them to the public Khronos Bugzilla (http://khronos.org/bugzilla)
 * by filing a bug against product "EGL" component "Registry".
 */

#include <KHR/khrplatform.h>

/* Macros used in EGL function prototype declarations.
 *
 * EGL functions should be prototyped as:
 *
 * EGLAPI return-type EGLAPIENTRY eglFunction(arguments);
 * typedef return-type (EXPAPIENTRYP PFNEGLFUNCTIONPROC) (arguments);
 *
 * KHRONOS_APICALL and KHRONOS_APIENTRY are defined in KHR/khrplatform.h
 */

#ifndef EGLAPI
#define EGLAPI KHRONOS_APICALL
#endif

#ifndef EGLAPIENTRY
#define EGLAPIENTRY  KHRONOS_APIENTRY
#endif
#define EGLAPIENTRYP EGLAPIENTRY*

/* The types NativeDisplayType, NativeWindowType, and NativePixmapType
 * are aliases of window-system-dependent types, such as X Display * or
 * Windows Device Context. They must be defined in platform-specific
 * code below. The EGL-prefixed versions of Native*Type are the same
 * types, renamed in EGL 1.3 so all types in the API start with "EGL".
 *
 * Khronos STRONGLY RECOMMENDS that you use the default definitions
 * provided below, since these changes affect both binary and source
 * portability of applications using EGL running on different EGL
 * implementations.
 */
#if defined(_WIN32) || defined(__VC32__) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__) /* Win32 and Windows CE */
#ifndef WIN32_LEAN_AND_MEAN
/* #define WIN32_LEAN_AND_MEAN 1 */
#endif
#include <windows.h>

typedef HDC     EGLNativeDisplayType;
typedef HWND    EGLNativeWindowType;
typedef HBITMAP EGLNativePixmapType;

#elif /* defined(__APPLE__) || */ defined(__WINSCW__) || defined(__SYMBIAN32__)  /* Symbian */

typedef int   EGLNativeDisplayType;
typedef void *EGLNativeWindowType;
typedef void *EGLNativePixmapType;

#elif defined(WL_EGL_PLATFORM) /* Wayland */

typedef struct wl_display *   EGLNativeDisplayType;
typedef struct wl_egl_window *EGLNativeWindowType;
typedef struct wl_egl_pixmap *EGLNativePixmapType;

#elif defined(__ANDROID__) || defined(ANDROID)
#  if ANDROID_SDK_VERSION >= 26
#    include <system/window.h>
#  else
#    include <android/native_window.h>
#  endif

struct egl_native_pixmap_t;

typedef struct ANativeWindow *      EGLNativeWindowType;
typedef struct egl_native_pixmap_t *EGLNativePixmapType;
typedef void *                      EGLNativeDisplayType;

#elif defined(MIR_EGL_PLATFORM) /* Mir */

#include <mir_toolkit/mir_client_library.h>
typedef MirEGLNativeDisplayType EGLNativeDisplayType;
typedef void                   *EGLNativePixmapType;
typedef MirEGLNativeWindowType  EGLNativeWindowType;

#elif defined(__QNXNTO__) /* QNX */

#include <screen/screen.h>
typedef int             EGLNativeDisplayType;
typedef screen_window_t EGLNativeWindowType;
typedef screen_pixmap_t EGLNativePixmapType;

#elif defined(__unix__) || defined(__APPLE__)

#if defined(EGL_API_DFB)

/* Vivante DFB */
typedef void * EGLNativeDisplayType;
typedef void * EGLNativeWindowType;
typedef void * EGLNativePixmapType;

#elif defined(EGL_API_FB)

/* Vivante FBDEV */
typedef void * EGLNativeDisplayType;
typedef void * EGLNativeWindowType;
typedef void * EGLNativePixmapType;

#elif defined(EGL_API_NULLWS)

typedef void * EGLNativeDisplayType;
typedef void * EGLNativeWindowType;
typedef void * EGLNativePixmapType;

#else

/* X11 (tentative) */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef Display *EGLNativeDisplayType;
typedef Window   EGLNativeWindowType;
typedef Pixmap   EGLNativePixmapType;

#endif

#elif defined(__GBM__) /* GBM */

typedef struct gbm_device *EGLNativeDisplayType;
typedef struct gbm_bo     *EGLNativePixmapType;
typedef void              *EGLNativeWindowType;

#else
#error "Platform not recognized"
#endif

/* EGL 1.2 types, renamed for consistency in EGL 1.3 */
typedef EGLNativeDisplayType NativeDisplayType;
typedef EGLNativePixmapType  NativePixmapType;
typedef EGLNativeWindowType  NativeWindowType;


/* Define EGLint. This must be a signed integral type large enough to contain
 * all legal attribute names and values passed into and out of EGL, whether
 * their type is boolean, bitmask, enumerant (symbolic constant), integer,
 * handle, or other.  While in general a 32-bit integer will suffice, if
 * handles are 64 bit types, then EGLint should be defined as a signed 64-bit
 * integer type.
 */
typedef khronos_int32_t EGLint;

#include <EGL/eglvivante.h>

#endif /* __eglplatform_h */
