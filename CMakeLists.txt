CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)
PROJECT(karel)

find_package(OpenGL) #and GLU
IF (OPENGL_FOUND)
  MESSAGE("OpenGL found")
ELSE(OPENGL_FOUND)
  MESSAGE(FATAL_ERROR "OpenGL not found")
ENDIF(OPENGL_FOUND)


find_package(SDL)
IF (SDL_FOUND)
  MESSAGE("SDL found")
ELSE(SDL_FOUND)
  MESSAGE(FATAL_ERROR "Do you have sdl? (on ubuntu 10.10 it's libsdl1.2.dev package)")
ENDIF(SDL_FOUND)


# find_package(DevIL)
# IF (IL_FOUND)
#   MESSAGE("DevIL found")
# ELSE(IL_FOUND)
#   MESSAGE(FATAL_ERROR "DevIL not found")
# ENDIF(IL_FOUND)



set(CORELIBS ${OPENGL_LIBRARIES} ${SDL_LIBRARY})

ADD_EXECUTABLE(karel  main.cpp)
include_directories(objTester)
ADD_SUBDIRECTORY(objTester)

include_directories(stb_image)
ADD_SUBDIRECTORY(stb_image)
target_link_libraries(karel ${CORELIBS} objLoader stb_image)

