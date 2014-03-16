cmake_minimum_required(VERSION 2.8)

# Make sure the user is not executing this script directly
if(NOT InOpenJK)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InOpenJK)

set(SPVanillaRendererIncludeDirectories ${SPDir})
set(SPVanillaRendererIncludeDirectories ${SPVanillaRendererIncludeDirectories} "${SPDir}/rd-vanilla" ${SharedDir})

# Modular Renderer crap (let's hope this works..)

# Defines
set(SPVanillaRendererDefines ${SharedDefines} "RENDERER" "_JK2EXE")

# Files

# JPEG
file(GLOB_RECURSE J_SRC "${OpenJKLibDir}/jpeg-8c/*.c" "${OpenJKLibDir}/jpeg-8c/*.h")
source_group("jpeg-8c" FILES ${J_SRC})
set(SPVanillaRendererFiles ${SPVanillaRendererFiles} ${J_SRC})

# GHOUL 2
set(SPVanillaRendererG2Files
	"${SPDir}/ghoul2/G2.h"
	"${SPDir}/ghoul2/ghoul2_gore.h"
	"${SPDir}/rd-vanilla/G2_API.cpp"
	"${SPDir}/rd-vanilla/G2_bolts.cpp"
	"${SPDir}/rd-vanilla/G2_bones.cpp"
	"${SPDir}/rd-vanilla/G2_misc.cpp"
	"${SPDir}/rd-vanilla/G2_surfaces.cpp"
	)
source_group("ghoul2" FILES ${SPVanillaRendererG2Files})
set(SPVanillaRendererFiles ${SPVanillaRendererFiles} ${SPVanillaRendererG2Files})

if(UseInternalPNG)
	set(SPVanillaRendererLibPngFiles
		"${OpenJKLibDir}/libpng/png.c"
		"${OpenJKLibDir}/libpng/pngerror.c"
		"${OpenJKLibDir}/libpng/pngget.c"
		"${OpenJKLibDir}/libpng/pngmem.c"
		"${OpenJKLibDir}/libpng/pngpread.c"
		"${OpenJKLibDir}/libpng/pngread.c"
		"${OpenJKLibDir}/libpng/pngrio.c"
		"${OpenJKLibDir}/libpng/pngrtran.c"
		"${OpenJKLibDir}/libpng/pngrutil.c"
		"${OpenJKLibDir}/libpng/pngset.c"
		"${OpenJKLibDir}/libpng/pngtrans.c"
		"${OpenJKLibDir}/libpng/pngwio.c"
		"${OpenJKLibDir}/libpng/pngwrite.c"
		"${OpenJKLibDir}/libpng/pngwtran.c"
		"${OpenJKLibDir}/libpng/pngwutil.c")
	source_group("libpng" FILES ${SPVanillaRendererLibPngFiles})
	set(SPVanillaRendererIncludeDirectories ${SPVanillaRendererIncludeDirectories} "${OpenJKLibDir}/libpng")
	set(SPVanillaRendererFiles ${SPVanillaRendererFiles} ${SPVanillaRendererLibPngFiles})
else(UseInternalPNG)
	find_package(PNG REQUIRED)
	set(SPVanillaRendererIncludeDirectories ${SPVanillaRendererIncludeDirectories} ${PNG_INCLUDE_DIR})
	set(SPVanillaRendererLibraries ${SPVanillaRendererLibraries} ${PNG_LIBRARIES})
endif(UseInternalPNG)

if(UseInternalZlib)
	# zlib.lib is included for windows
	find_library(ZlibLibrary NAMES zlib PATHS ${OpenJKLibDir})
	if(NOT ZlibLibrary)
		message(FATAL_ERROR "UseInternalZlib enabled, but lib/zlib.lib not found!")
	endif(NOT ZlibLibrary)
	set(SPVanillaRendererIncludeDirectories ${SPVanillaRendererIncludeDirectories} "${OpenJKLibDir}/zlib")
	set(SPVanillaRendererLibraries ${SPVanillaRendererLibraries} ${ZlibLibrary})
else(UseInternalZlib)
	find_package(ZLIB REQUIRED)
	set(SPVanillaRendererIncludeDirectories ${SPVanillaRendererIncludeDirectories} ${ZLIB_INCLUDE_DIR})
	set(SPVanillaRendererLibraries ${SPVanillaRendererLibraries} ${ZLIB_LIBRARY})
endif(UseInternalZlib)

# Misc files
set(SPVanillaRendererGameFiles
	"${SPDir}/game/genericparser2.h"
	"${SPDir}/game/genericparser2.cpp"
	)
source_group("game" FILES ${SPVanillaRendererGameFiles})
set(SPVanillaRendererFiles ${SPVanillaRendererFiles} ${SPVanillaRendererGameFiles})

# Misc files
set(SPVanillaRendererCommonFiles
	"${SPDir}/qcommon/matcomp.cpp"
	"${SPDir}/qcommon/q_math.cpp"
	"${SPDir}/qcommon/q_shared.cpp"
	"${SPDir}/qcommon/q_shared.h"
	"${SharedDir}/qcommon/q_platform.h"
	"${SPDir}/qcommon/cm_landscape.h"
	)
source_group("common" FILES ${SPVanillaRendererCommonFiles})
set(SPVanillaRendererFiles ${SPVanillaRendererFiles} ${SPVanillaRendererCommonFiles})

# main files
set(SPVanillaRendererMainFiles
	"${SPDir}/rd-vanilla/glext.h"
	"${SPDir}/rd-vanilla/qgl.h"
	"${SPDir}/rd-vanilla/tr_arioche.cpp"
	"${SPDir}/rd-vanilla/tr_backend.cpp"
	"${SPDir}/rd-vanilla/tr_bsp.cpp"
	"${SPDir}/rd-vanilla/tr_cmds.cpp"
	"${SPDir}/rd-vanilla/tr_curve.cpp"
	"${SPDir}/rd-vanilla/tr_draw.cpp"
	"${SPDir}/rd-vanilla/tr_flares.cpp"
	"${SPDir}/rd-vanilla/tr_ghoul2.cpp"
	"${SPDir}/rd-vanilla/tr_image.cpp"
	"${SPDir}/rd-vanilla/tr_init.cpp"
	"${SPDir}/rd-vanilla/tr_landscape.h"
	"${SPDir}/rd-vanilla/tr_light.cpp"
	"${SPDir}/rd-vanilla/tr_local.h"
	"${SPDir}/rd-vanilla/tr_main.cpp"
	"${SPDir}/rd-vanilla/tr_marks.cpp"
	"${SPDir}/rd-vanilla/tr_mesh.cpp"
	"${SPDir}/rd-vanilla/tr_model.cpp"
	"${SPDir}/rd-vanilla/tr_quicksprite.cpp"
	"${SPDir}/rd-vanilla/tr_quicksprite.h"
	"${SPDir}/rd-vanilla/tr_scene.cpp"
	"${SPDir}/rd-vanilla/tr_shade.cpp"
	"${SPDir}/rd-vanilla/tr_shade_calc.cpp"
	"${SPDir}/rd-vanilla/tr_shader.cpp"
	"${SPDir}/rd-vanilla/tr_shadows.cpp"
	"${SPDir}/rd-vanilla/tr_sky.cpp"
	"${SPDir}/rd-vanilla/tr_stl.cpp"
	"${SPDir}/rd-vanilla/tr_stl.h"
	"${SPDir}/rd-vanilla/tr_subs.cpp"
	"${SPDir}/rd-vanilla/tr_surface.cpp"
	"${SPDir}/rd-vanilla/tr_surfacesprites.cpp"
	"${SPDir}/rd-vanilla/tr_terrain.cpp"
	"${SPDir}/rd-vanilla/tr_world.cpp"
	"${SPDir}/rd-vanilla/tr_WorldEffects.cpp"
	"${SPDir}/rd-vanilla/tr_WorldEffects.h"
	)
source_group("renderer" FILES ${SPVanillaRendererMainFiles})
set(SPVanillaRendererFiles ${SPVanillaRendererFiles} ${SPVanillaRendererMainFiles})

set(SPVanillaRendererRdCommonFiles
	"${SPDir}/rd-common/mdx_format.h"
	"${SPDir}/rd-common/tr_common.h"
	"${SPDir}/rd-common/tr_font.cpp"
	"${SPDir}/rd-common/tr_font.h"
	"${SPDir}/rd-common/tr_image_load.cpp"
	"${SPDir}/rd-common/tr_image_manipulation.cpp"
	"${SPDir}/rd-common/tr_image_jpg.cpp"
	"${SPDir}/rd-common/tr_image_tga.cpp"
	"${SPDir}/rd-common/tr_image_png.cpp"
	"${SPDir}/rd-common/tr_noise.cpp"
	"${SPDir}/rd-common/tr_public.h"
	"${SPDir}/rd-common/tr_types.h")
source_group("rd-common" FILES ${SPVanillaRendererRdCommonFiles})
set(SPVanillaRendererFiles ${SPVanillaRendererFiles} ${SPVanillaRendererRdCommonFiles})

set(SPVanillaRendererSdlFiles
	"${SPDir}/sdl/sdl_glimp.cpp")
source_group("sdl" FILES ${SPVanillaRendererSdlFiles})
set(SPVanillaRendererFiles ${SPVanillaRendererFiles} ${SPVanillaRendererSdlFiles})

find_package(OpenGL REQUIRED)
set(SPVanillaRendererIncludeDirectories ${SPVanillaRendererIncludeDirectories} ${OPENGL_INCLUDE_DIR})
set(SPVanillaRendererLibraries ${SPVanillaRendererLibraries} ${OPENGL_LIBRARIES})

if(UseInternalSDL)
	find_library(LibSDL2 NAMES SDL2 PATHS ${OpenJKLibDir})
	if(NOT LibSDL2)
		message(FATAL_ERROR "UseInternalSDL enabled, but lib/SDL2.lib not found!")
	endif(NOT LibSDL2)
	set(SPVanillaRendererIncludeDirectories ${SPVanillaRendererIncludeDirectories} "${OpenJKLibDir}/sdl2")
	set(SPVanillaRendererLibraries ${SPVanillaRendererLibraries} ${LibSDL2})
else(UseInternalSDL)
	set(SDL2_BUILDING_LIBRARY ON) # don't want main() in Renderer
	find_package(SDL2 REQUIRED)
	set(SPVanillaRendererIncludeDirectories ${SPVanillaRendererIncludeDirectories} ${SDL2_INCLUDE_DIR})
	set(SPVanillaRendererLibraries ${SPVanillaRendererLibraries} ${SDL2_LIBRARY})
endif(UseInternalSDL)

set(SPVanillaRendererIncludeDirectories ${SPVanillaRendererIncludeDirectories} ${OpenJKLibDir})

function(add_sp_renderer_project ProjectName Label)
	add_library(${ProjectName} SHARED ${SPVanillaRendererFiles})
	if(NOT WIN32)
		# remove "lib" prefix for .so/.dylib files
		set_target_properties(${ProjectName} PROPERTIES PREFIX "")
	endif(NOT WIN32)

	if(WIN32)
		install(TARGETS ${ProjectName} RUNTIME DESTINATION ".")
	else(WIN32)
		if(MakeApplicationBundles AND BuildSPEngine)
			install(TARGETS ${ProjectName} LIBRARY DESTINATION "${SPEngine}.app/Contents/MacOS/")
		else(MakeApplicationBundles AND BuildSPEngine)
			install(TARGETS ${ProjectName} LIBRARY DESTINATION ".")
		endif(MakeApplicationBundles AND BuildSPEngine)
	endif(WIN32)

	set_target_properties(${ProjectName} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "${SPVanillaRendererDefines};${ReleaseDefines}")
	set_target_properties(${ProjectName} PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL "${SPVanillaRendererDefines};${ReleaseDefines}")
	set_target_properties(${ProjectName} PROPERTIES COMPILE_DEFINITIONS_RELEASE "${SPVanillaRendererDefines};${ReleaseDefines}")
	set_target_properties(${ProjectName} PROPERTIES COMPILE_DEFINITIONS_DEBUG "${SPVanillaRendererDefines};${DebugDefines}")
	set_target_properties(${ProjectName} PROPERTIES INCLUDE_DIRECTORIES "${SPVanillaRendererIncludeDirectories}")
	set_target_properties(${ProjectName} PROPERTIES PROJECT_LABEL ${Label})

	target_link_libraries(${ProjectName} ${SPVanillaRendererLibraries})
endfunction(add_sp_renderer_project)

if(BuildSPVanillaRenderer)
	add_sp_renderer_project(${SPVanillaRenderer} "SP Vanilla Renderer")
endif(BuildSPVanillaRenderer)
if(BuildJK2SPVanillaRenderer)
	set(SPVanillaRendererDefines ${SPVanillaRendererDefines} "JK2_MODE")
	add_sp_renderer_project(${JK2SPVanillaRenderer} "JK2 SP Vanilla Renderer")
endif(BuildJK2SPVanillaRenderer)
