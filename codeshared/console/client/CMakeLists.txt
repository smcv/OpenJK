cmake_minimum_required(VERSION 2.8.8)

# Make sure the user is not executing this script directly
if(NOT InOpenJK)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InOpenJK)

# Add ClientConsoleLib if requested

if(BuildClientConsoleLib)
	set(ClientConsoleLibDefines ${SharedDefines})
	set(ClientConsoleLibIncludeDirectories ${SharedDir})
	
	# Files
	
	set(ClientConsoleLibForwardFiles
		"${SharedDir}/forward/cl_public.h"
		)
	source_group("Forward Declarations" FILES ${ClientConsoleLibForwardFiles})
	set(ClientConsoleLibFiles ${ClientConsoleLibFiles} ${ClientConsoleLibForwardFiles})
	
	
	if(WIN32)
		set(Platform "windows")
	else(WIN32)
		set(Platform "unix")
	endif(WIN32)
	
	set(ClientConsoleLibSysFiles
		"${SharedDir}/console/con_public.h"
		"${SharedDir}/console/client/${Platform}/con_client.cpp"
		)
	source_group("Console" FILES ${ClientConsoleLibSysFiles})
	set(ClientConsoleLibFiles ${ClientConsoleLibFiles} ${ClientConsoleLibSysFiles})
	
	set(ClientConsoleLibCommonFiles
		"${SharedDir}/qcommon/q_platform.h"
		"${SharedDir}/qcommon/q_sharedtypes.h"
		)
	source_group("Common" FILES ${ClientConsoleLibCommonFiles})
	set(ClientConsoleLibFiles ${ClientConsoleLibFiles} ${ClientConsoleLibCommonFiles})
	
	# Create library
	
	add_library(${ClientConsoleLib} STATIC ${ClientConsoleLibFiles})
	# nothing to install, it's a static library
	set_target_properties(${ClientConsoleLib} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "${ClientConsoleLibDefines};${ReleaseDefines}")
	set_target_properties(${ClientConsoleLib} PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL "${ClientConsoleLibDefines};${ReleaseDefines}")
	set_target_properties(${ClientConsoleLib} PROPERTIES COMPILE_DEFINITIONS_RELEASE "${ClientConsoleLibDefines};${ReleaseDefines}")
	set_target_properties(${ClientConsoleLib} PROPERTIES COMPILE_DEFINITIONS_DEBUG "${ClientConsoleLibDefines};${DebugDefines}")
	set_target_properties(${ClientConsoleLib} PROPERTIES INCLUDE_DIRECTORIES "${ClientConsoleLibIncludeDirectories}")
	set_target_properties(${ClientConsoleLib} PROPERTIES PROJECT_LABEL "Client Console Library")

endif(BuildClientConsoleLib)
