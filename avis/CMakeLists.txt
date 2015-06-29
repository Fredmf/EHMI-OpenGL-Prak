# avis
add_executable(avis
    main.cpp
    AudioFFT.h
    AudioFFT.cpp
)
target_link_libraries(avis
    common
    ${RPi_LIBS}
    ${GL_LIBS}
    ${AU_LIBS}
)
file(
COPY
cube.obj
TextureFragmentShader.frag
TransformVertexShader.vert
uvtemplate.bmp
nin.wav
around_the_world-atc.wav
DESTINATION ${CMAKE_BINARY_DIR}/avis
)
