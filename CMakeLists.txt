CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)
PROJECT(KAREL)

IF(CROSSCOMP)
  link_directories(${KAREL_SOURCE_DIR}/ming-sdl/lib)
  set(CMAKE_EXE_LINKER_FLAGS " -lmingw32 -lSDLmain -lSDL -mwindows")
  set(SDL_LIBRARY "libSDL.dll.a")
  set(SDLTTF_LIBRARY "SDL_ttf.lib")
  include_directories("ming-sdl/include")
  include_directories("ming-sdl/include/SDL")
ENDIF(CROSSCOMP)

find_package(OpenGL) #and GLU
IF (OPENGL_FOUND)
  MESSAGE("OpenGL found")
ELSE(OPENGL_FOUND)
  MESSAGE(FATAL_ERROR "OpenGL not found")
ENDIF(OPENGL_FOUND)

IF(NOT CROSSCOMP)
find_package(SDL)
IF (SDL_FOUND)
  MESSAGE("SDL found")
ELSE(SDL_FOUND)
  MESSAGE(FATAL_ERROR "Do you have sdl? (on ubuntu 10.10 it's libsdl1.2-dev package)")
ENDIF(SDL_FOUND)



find_package(SDL_ttf)
IF (SDLTTF_FOUND)
  MESSAGE("SDL ttf found")
ELSE(SDLTTF_FOUND)
  MESSAGE(FATAL_ERROR "Do you have sdl ttf? (on ubuntu 10.10 it's libsdl-ttf2.0-dev package)")
ENDIF(SDLTTF_FOUND)
ENDIF(NOT CROSSCOMP)

# find_package(DevIL)
# IF (IL_FOUND)
#   MESSAGE("DevIL found")
# ELSE(IL_FOUND)
#   MESSAGE(FATAL_ERROR "DevIL not found")
# ENDIF(IL_FOUND)


set(CORELIBS ${OPENGL_LIBRARIES} ${SDL_LIBRARY} ${SDLTTF_LIBRARY})


ADD_EXECUTABLE(karel  main.cpp)
include_directories(objTester)
ADD_SUBDIRECTORY(objTester)

include_directories(stb_image)
ADD_SUBDIRECTORY(stb_image)

target_link_libraries(karel ${CORELIBS} objLoader stb_image)