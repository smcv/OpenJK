cmake_minimum_required(VERSION 2.8.8)

# Make sure the user is not executing this script directly
if(NOT InOpenJK)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InOpenJK)

set(WindowLibDefines ${SharedDefines})
set(WindowLibIncludeDirectories ${SharedDir})

set(WindowLibForwardFiles
	"${SharedDir}/forward/cl_public.h"
	)
source_group("Forward Definitions" FILES ${WindowLibForwardFiles})
set(WindowLibFiles ${WindowLibFiles} ${WindowLibForwardFiles})

set(WindowLibWindowFiles
	"${SharedDir}/window/window_public.h"
	)
source_group("Window" FILES ${WindowLibWindowFiles})
set(WindowLibFiles ${WindowLibFiles} ${WindowLibWindowFiles})

add_library(${WindowLib} STATIC ${WindowLibFiles})
# nothing to install, it's a static library
set_target_properties(${WindowLib} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "${WindowLibDefines};${ReleaseDefines}")
set_target_properties(${WindowLib} PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL "${WindowLibDefines};${ReleaseDefines}")
set_target_properties(${WindowLib} PROPERTIES COMPILE_DEFINITIONS_RELEASE "${WindowLibDefines};${ReleaseDefines}")
set_target_properties(${WindowLib} PROPERTIES COMPILE_DEFINITIONS_DEBUG "${WindowLibDefines};${DebugDefines}")
set_target_properties(${WindowLib} PROPERTIES INCLUDE_DIRECTORIES "${WindowLibIncludeDirectories}")
set_target_properties(${WindowLib} PROPERTIES PROJECT_LABEL "Window Code Library")
target_link_libraries(${WindowLib} ${WindowLibLibraries})
