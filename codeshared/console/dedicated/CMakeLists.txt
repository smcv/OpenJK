cmake_minimum_required(VERSION 2.8.8)

# Make sure the user is not executing this script directly
if(NOT InOpenJK)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InOpenJK)

# Add DedicatedConsoleLib if requested

if(BuildDedicatedConsoleLib)
	set(DedicatedConsoleLibDefines ${SharedDefines})
	set(DedicatedConsoleLibIncludeDirectories ${SharedDir})
	
	# Files
	
	set(DedicatedConsoleLibForwardFiles
		"${SharedDir}/forward/cl_public.h"
		)
	source_group("Forward Declarations" FILES ${DedicatedConsoleLibForwardFiles})
	set(DedicatedConsoleLibFiles ${DedicatedConsoleLibFiles} ${DedicatedConsoleLibForwardFiles})
	
	
	if(WIN32)
		set(Platform "windows")
	else(WIN32)
		set(Platform "unix")
	endif(WIN32)
	
	set(DedicatedConsoleLibSysFiles
		"${SharedDir}/console/con_public.h"
		"${SharedDir}/console/dedicated/${Platform}/con_dedicated.cpp"
		)
	source_group("Console" FILES ${DedicatedConsoleLibSysFiles})
	set(DedicatedConsoleLibFiles ${DedicatedConsoleLibFiles} ${DedicatedConsoleLibSysFiles})
	
	set(DedicatedConsoleLibCommonFiles
		"${SharedDir}/qcommon/q_platform.h"
		"${SharedDir}/qcommon/q_sharedtypes.h"
		)
	source_group("Common" FILES ${DedicatedConsoleLibCommonFiles})
	set(DedicatedConsoleLibFiles ${DedicatedConsoleLibFiles} ${DedicatedConsoleLibCommonFiles})
	
	# Create library
	
	add_library(${DedicatedConsoleLib} STATIC ${DedicatedConsoleLibFiles})
	# nothing to install, it's a static library
	set_target_properties(${DedicatedConsoleLib} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "${DedicatedConsoleLibDefines};${ReleaseDefines}")
	set_target_properties(${DedicatedConsoleLib} PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL "${DedicatedConsoleLibDefines};${ReleaseDefines}")
	set_target_properties(${DedicatedConsoleLib} PROPERTIES COMPILE_DEFINITIONS_RELEASE "${DedicatedConsoleLibDefines};${ReleaseDefines}")
	set_target_properties(${DedicatedConsoleLib} PROPERTIES COMPILE_DEFINITIONS_DEBUG "${DedicatedConsoleLibDefines};${DebugDefines}")
	set_target_properties(${DedicatedConsoleLib} PROPERTIES INCLUDE_DIRECTORIES "${DedicatedConsoleLibIncludeDirectories}")
	set_target_properties(${DedicatedConsoleLib} PROPERTIES PROJECT_LABEL "Dedicated Server Console Library")

endif(BuildDedicatedConsoleLib)
