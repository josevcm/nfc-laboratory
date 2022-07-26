set(oglGraph_src_files
	dlgMarkers.cpp
	GLFont.cpp
	OpenGLGraph.cpp
	glew/glew.c
)
include_directories(glew)
include_directories(glew/GL)

#ignore warnings about narrowing: static font header
if(CMAKE_COMPILER_IS_GNUCXX)
	add_definitions(-Wno-narrowing)
endif()

ADD_DEFINITIONS(-DGLEW_STATIC)
add_library(oglGraph STATIC ${oglGraph_src_files})

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
if(NOT TARGET OpenGL::GL)
    message(FATAL_ERROR "OpenGL/GL not found!")
endif(NOT TARGET OpenGL::GL)
if(NOT TARGET OpenGL::GLU)
    message(FATAL_ERROR "OpenGL/GLU not found!")
endif(NOT TARGET OpenGL::GLU)
message(STATUS "OPENGL_INCLUDE_DIR: ${OPENGL_INCLUDE_DIR}")
message(STATUS "OPENGL_LIBRARIES: ${OPENGL_LIBRARIES}")

target_include_directories(oglGraph PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OPENGL_INCLUDE_DIR}
)

if (APPLE)
	set(GL_LIBS "-framework OpenGL" "-framework GLUT")
elseif (UNIX)
	set(GL_LIBS GL ${OPENGL_LIBRARIES})
endif()

if(UNIX)
	target_link_libraries(oglGraph ${GL_LIBS} ${wxWidgets_LIBRARIES})
endif()
