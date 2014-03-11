cmake_minimum_required(VERSION 2.8.8)

# Make sure the user is not executing this script directly
if(NOT InOpenJK)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InOpenJK)

if(BuildCommonLib)
	set(CommonLibDefines ${SharedDefines})
	set(CommonLibIncludeDirectories ${SharedDir})
	
	set(CommonLibForwardFiles
		"${SharedDir}/forward/cl_public.h"
		)
	source_group("Forward Definitions" FILES ${CommonLibForwardFiles})
	set(CommonLibFiles ${CommonLibFiles} ${CommonLibForwardFiles})
	
	set(CommonLibSysFiles
		"${SharedDir}/sys/sys_public.h"
		)
	source_group("System" FILES ${CommonLibSysFiles})
	set(CommonLibFiles ${CommonLibFiles} ${CommonLibSysFiles})
	
	set(CommonLibGhoul2Files
		"${SharedDir}/ghoul2/g2_public.h"
		)
	source_group("Ghoul 2" FILES ${CommonLibGhoul2Files})
	set(CommonLibFiles ${CommonLibFiles} ${CommonLibGhoul2Files})
	
	add_library(${CommonLib} STATIC ${CommonLibFiles})
	# nothing to install, it's a static library
	set_target_properties(${CommonLib} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "${CommonLibDefines};${ReleaseDefines}")
	set_target_properties(${CommonLib} PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL "${CommonLibDefines};${ReleaseDefines}")
	set_target_properties(${CommonLib} PROPERTIES COMPILE_DEFINITIONS_RELEASE "${CommonLibDefines};${ReleaseDefines}")
	set_target_properties(${CommonLib} PROPERTIES COMPILE_DEFINITIONS_DEBUG "${CommonLibDefines};${DebugDefines}")
	set_target_properties(${CommonLib} PROPERTIES INCLUDE_DIRECTORIES "${CommonLibIncludeDirectories}")
	set_target_properties(${CommonLib} PROPERTIES PROJECT_LABEL "Common Code Library")
	target_link_libraries(${CommonLib} ${CommonLibLibraries})

endif(BuildCommonLib)

if(BuildWindowLib)
	add_subdirectory("window")
endif(BuildWindowLib)