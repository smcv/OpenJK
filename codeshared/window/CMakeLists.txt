cmake_minimum_required(VERSION 2.8.8)

# Make sure the user is not executing this script directly
if(NOT InOpenJK)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InOpenJK)

set(WindowLibDefines ${SharedDefines})
set(WindowLibIncludeDirectories ${SharedDir})

# Add SDL

if(UseInternalSDL)
	set(WindowLibIncludeDirectories ${WindowLibIncludeDirectories} "${OpenJKLibDir}/sdl2")
else(UseInternalSDL)
	find_package(SDL2 REQUIRED)
	set(WindowLibIncludeDirectories ${WindowLibIncludeDirectories} ${SDL2_INCLUDE_DIR})
endif(UseInternalSDL)

# set(SPEngineSysFiles
	# "${SPDir}/sys/sys_main.cpp"
	# "${SPDir}/sys/sys_unix.cpp"
	# "${SPDir}/sys/sys_local.h"
	# "${SPDir}/sys/sys_loadlib.h"
	# "${SPDir}/sdl/sdl_qgl.h"
	# "${SPDir}/sdl/sdl_snd.cpp"
	# "${SPDir}/sdl/sdl_input.cpp"
	# )
# source_group("sys" FILES ${SPEngineSysFiles})
# set(SPEngineFiles ${SPEngineFiles} ${SPEngineSysFiles})

# Add Files

set(WindowLibForwardFiles
	"${SharedDir}/forward/cl_public.h"
	)
source_group("Forward Declarations" FILES ${WindowLibForwardFiles})
set(WindowLibFiles ${WindowLibFiles} ${WindowLibForwardFiles})

set(WindowLibWindowFiles
	"${SharedDir}/window/window_public.h"
	"${SharedDir}/window/window_local.h"
	"${SharedDir}/window/window_main.c"
	"${SharedDir}/window/window_init.cpp"
	"${SharedDir}/window/window_events.cpp"
	"${SharedDir}/window/window_sound.cpp"
	"${SharedDir}/window/window_clipboard.cpp"
	"${SharedDir}/window/window_videomode.cpp"
	)
source_group("Window" FILES ${WindowLibWindowFiles})
set(WindowLibFiles ${WindowLibFiles} ${WindowLibWindowFiles})

# Create project

add_library(${WindowLib} STATIC ${WindowLibFiles})
# nothing to install, it's a static library
set_target_properties(${WindowLib} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "${WindowLibDefines};${ReleaseDefines}")
set_target_properties(${WindowLib} PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL "${WindowLibDefines};${ReleaseDefines}")
set_target_properties(${WindowLib} PROPERTIES COMPILE_DEFINITIONS_RELEASE "${WindowLibDefines};${ReleaseDefines}")
set_target_properties(${WindowLib} PROPERTIES COMPILE_DEFINITIONS_DEBUG "${WindowLibDefines};${DebugDefines}")
set_target_properties(${WindowLib} PROPERTIES INCLUDE_DIRECTORIES "${WindowLibIncludeDirectories}")
set_target_properties(${WindowLib} PROPERTIES PROJECT_LABEL "Window Code Library")
