# CMake entry point
cmake_minimum_required (VERSION 2.8)
project ("EHMI OpenGL Prak")

SET(COMPILE_DEFINITIONS -Werror)

include_directories(
    /opt/vc/include
    /opt/vc/include/interface/vcos/pthreads
    /opt/vc/include/interface/vmcs_host/linux
#   ${CMAKE_SOURCE_DIR}/libs/glm/
    ${CMAKE_SOURCE_DIR}/libs/ilclient
    ${CMAKE_SOURCE_DIR}/common
)

link_directories(
    ${CMAKE_SOURCE_DIR}/libs/ilclient
    /opt/vc/lib
)


set(RPi_LIBS
# Raspberry Pi - Broadcom
    libbcm_host.so
    libvcos.so
)

set(AU_LIBS
    pulse
    pulse-simple
    sndfile
)

set(CAM_LIBS
# camera
    libmmal_core.so
    libmmal_util.so
    libmmal_vc_client.so
)

set(GL_LIBS
# openGLES
    GLESv2
    EGL
)

set(ILC_LIBS
# ilclient
    ilclient
    openmaxil
)


# common library
add_subdirectory(common)

# subdirectories
add_subdirectory(avis)
