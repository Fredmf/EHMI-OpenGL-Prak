# avis
add_executable(avis
    main.cpp
)
target_link_libraries(avis
    common
    ${RPi_LIBS}
    ${GL_LIBS}
)
file(
COPY
cube.obj
TextureFragmentShader.glsl
TransformVertexShader.glsl
uvtemplate.bmp
DESTINATION ${CMAKE_BINARY_DIR}/avis
)
