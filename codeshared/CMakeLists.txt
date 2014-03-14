cmake_minimum_required(VERSION 2.8.8)

# Make sure the user is not executing this script directly
if(NOT InOpenJK)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InOpenJK)

# Add CommonLib if requested

if(BuildCommonLib)
	set(CommonLibDefines ${SharedDefines})
	set(CommonLibIncludeDirectories ${SharedDir})
	
	# Files
	
	set(CommonLibForwardFiles
		"${SharedDir}/forward/cl_public.h"
		)
	source_group("Forward Declarations" FILES ${CommonLibForwardFiles})
	set(CommonLibFiles ${CommonLibFiles} ${CommonLibForwardFiles})
	
	
	if(WIN32)
		set(Platform "windows")
	else(WIN32)
		set(Platform "unix")
	endif(WIN32)
	
	set(CommonLibSysFiles
		"${SharedDir}/sys/sys_public.h"
		"${SharedDir}/sys/sys_local.h"
		"${SharedDir}/sys/sys_main.cpp"
		"${SharedDir}/sys/sys_paths.cpp"
		"${SharedDir}/sys/sys_events.cpp"
		"${SharedDir}/sys/sys_snapvector.cpp"
		"${SharedDir}/sys/sys_common.cpp"
		"${SharedDir}/sys/${Platform}/${Platform}_local.h"
		"${SharedDir}/sys/${Platform}/sys_dynlib.cpp"
		"${SharedDir}/sys/${Platform}/sys_filesystem.cpp"
		"${SharedDir}/sys/${Platform}/sys_time.cpp"
		"${SharedDir}/sys/${Platform}/sys_os.cpp"
		)
	source_group("System" FILES ${CommonLibSysFiles})
	set(CommonLibFiles ${CommonLibFiles} ${CommonLibSysFiles})
	
	set(CommonLibCommonFiles
		"${SharedDir}/qcommon/q_platform.h"
		"${SharedDir}/qcommon/q_sharedtypes.h"
		"${SharedDir}/qcommon/qcommon_shared.h"
		)
	source_group("Common" FILES ${CommonLibCommonFiles})
	set(CommonLibFiles ${CommonLibFiles} ${CommonLibCommonFiles})
	
	
	set(CommonLibGhoul2Files
		"${SharedDir}/ghoul2/g2_public.h"
		)
	source_group("Ghoul 2" FILES ${CommonLibGhoul2Files})
	set(CommonLibFiles ${CommonLibFiles} ${CommonLibGhoul2Files})
	
	# Create library
	
	add_library(${CommonLib} STATIC ${CommonLibFiles})
	# nothing to install, it's a static library
	set_target_properties(${CommonLib} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "${CommonLibDefines};${ReleaseDefines}")
	set_target_properties(${CommonLib} PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL "${CommonLibDefines};${ReleaseDefines}")
	set_target_properties(${CommonLib} PROPERTIES COMPILE_DEFINITIONS_RELEASE "${CommonLibDefines};${ReleaseDefines}")
	set_target_properties(${CommonLib} PROPERTIES COMPILE_DEFINITIONS_DEBUG "${CommonLibDefines};${DebugDefines}")
	set_target_properties(${CommonLib} PROPERTIES INCLUDE_DIRECTORIES "${CommonLibIncludeDirectories}")
	set_target_properties(${CommonLib} PROPERTIES PROJECT_LABEL "Common Code Library")

endif(BuildCommonLib)

# Add WindowLib if requested

if(BuildWindowLib)
	add_subdirectory("window")
endif(BuildWindowLib)

add_subdirectory("console/client")
add_subdirectory("console/dedicated")
