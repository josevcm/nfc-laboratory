set(CMAKE_CXX_STANDARD 17)

set(PRIVATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/include)

add_library(gl-engine STATIC
        src/main/cpp/engine/Buffer.cpp
        src/main/cpp/engine/Colors.cpp
        src/main/cpp/engine/Device.cpp
        src/main/cpp/engine/Engine.cpp
        src/main/cpp/engine/Font.cpp
        src/main/cpp/engine/Matrix.cpp
        src/main/cpp/engine/Metrics.cpp
        src/main/cpp/engine/Model.cpp
        src/main/cpp/engine/Object.cpp
        src/main/cpp/engine/Renderer.cpp
        src/main/cpp/engine/Program.cpp
        src/main/cpp/engine/Texture.cpp
        src/main/cpp/engine/Vector.cpp
        src/main/cpp/engine/Viewer.cpp
        src/main/cpp/engine/Widget.cpp
        src/main/cpp/shader/AbstractShader.cpp
        src/main/cpp/shader/GeometryShader.cpp
        src/main/cpp/shader/FontShader.cpp
        src/main/cpp/shader/TextureShader.cpp
        src/main/cpp/shader/TypeFaceShader.cpp
        src/main/cpp/transform/AccelerateTransform.cpp
        src/main/cpp/transform/ResizeTransform.cpp
        src/main/cpp/transform/RotateTransform.cpp
        src/main/cpp/transform/TranslateTransform.cpp
        src/main/cpp/widget/AxisWidget.cpp
        src/main/cpp/widget/BoxWidget.cpp
        src/main/cpp/widget/GridWidget.cpp
        src/main/cpp/widget/QuadWidget.cpp
        src/main/cpp/widget/TextWidget.cpp
        src/main/cpp/widget/PanelWidget.cpp
        src/main/cpp/typeface/FreeType.cpp
        )

target_include_directories(gl-engine PRIVATE ${FT_INCLUDE_DIR})
target_include_directories(gl-engine PRIVATE ${GLEW_INCLUDE_DIR})
target_include_directories(gl-engine PRIVATE ${PRIVATE_SOURCE_DIR})
target_include_directories(gl-engine PUBLIC ${PUBLIC_INCLUDE_DIR} ${PUBLIC_INCLUDE_DIR}/opengl)

target_link_libraries(gl-engine rt-lang opengl32 ${GLEW_LIBRARY} ${FT_LIBRARY})
