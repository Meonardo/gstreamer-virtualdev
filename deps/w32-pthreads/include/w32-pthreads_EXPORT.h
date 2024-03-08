
#ifndef W32_PTHREADS_EXPORT_H
#define W32_PTHREADS_EXPORT_H

#ifdef W32_PTHREADS_STATIC_DEFINE
#  define W32_PTHREADS_EXPORT
#  define W32_PTHREADS_NO_EXPORT
#else
#  ifndef W32_PTHREADS_EXPORT
#    ifdef w32_pthreads_EXPORTS
        /* We are building this library */
#      define W32_PTHREADS_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define W32_PTHREADS_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef W32_PTHREADS_NO_EXPORT
#    define W32_PTHREADS_NO_EXPORT 
#  endif
#endif

#ifndef W32_PTHREADS_DEPRECATED
#  define W32_PTHREADS_DEPRECATED __declspec(deprecated)
#endif

#ifndef W32_PTHREADS_DEPRECATED_EXPORT
#  define W32_PTHREADS_DEPRECATED_EXPORT W32_PTHREADS_EXPORT W32_PTHREADS_DEPRECATED
#endif

#ifndef W32_PTHREADS_DEPRECATED_NO_EXPORT
#  define W32_PTHREADS_DEPRECATED_NO_EXPORT W32_PTHREADS_NO_EXPORT W32_PTHREADS_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef W32_PTHREADS_NO_DEPRECATED
#    define W32_PTHREADS_NO_DEPRECATED
#  endif
#endif

#endif /* W32_PTHREADS_EXPORT_H */
