/**
@file LimeSuiteConfig.h
@author Lime Microsystems
@brief Configuration constants and macros for lime suite library.
*/

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
  #define LIME_HELPER_DLL_IMPORT __declspec(dllimport)
  #define LIME_HELPER_DLL_EXPORT __declspec(dllexport)
  #define LIME_HELPER_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define LIME_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define LIME_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
    #define LIME_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define LIME_HELPER_DLL_IMPORT
    #define LIME_HELPER_DLL_EXPORT
    #define LIME_HELPER_DLL_LOCAL
  #endif
#endif

// Now we use the generic helper definitions above to define LIME_API and LIME_LOCAL.
// LIME_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// LIME_LOCAL is used for non-api symbols.
#ifdef LIME_DLL // defined if LIME is compiled as a DLL
  #ifdef LIME_DLL_EXPORTS // defined if we are building the LIME DLL (instead of using it)
    #define LIME_API LIME_HELPER_DLL_EXPORT
  #else
    #define LIME_API LIME_HELPER_DLL_IMPORT
  #endif // LIME_DLL_EXPORTS
  #define LIME_LOCAL LIME_HELPER_DLL_LOCAL
#else // LIME_DLL is not defined: this means LIME is a static lib.
  #define LIME_API
  #define LIME_LOCAL
#endif // LIME_DLL
