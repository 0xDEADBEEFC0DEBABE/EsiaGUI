# Esia - the Android platform layer (ESIA_PLATFORM_ANDROID): an activity's input -> the core's input, the soft keyboard,
# the clipboard, the density as the UI scale (include/esia/platform/android.hpp). Depends on the core only.
add_library(esia_platform_android STATIC android_platform.cpp)
target_link_libraries(esia_platform_android PUBLIC esia_core PRIVATE android log)
esia_target_defaults(esia_platform_android)
add_library(esia::platform_android ALIAS esia_platform_android)
