cmake_minimum_required(VERSION 3.16)
project(TriVRyad LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(TriVRyad
    main.cpp
    Game.cpp
    Gem.cpp
)
target_compile_definitions(TriVRyad PRIVATE
    UNICODE
    _UNICODE
)
target_include_directories(TriVRyad PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(ResetSettings
    ResetSettings.cpp
)
target_compile_definitions(ResetSettings PRIVATE
    UNICODE
    _UNICODE
)

if (MSVC)
    target_link_options(TriVRyad PRIVATE "/SUBSYSTEM:WINDOWS")
    target_link_options(TriVRyad PRIVATE "/ENTRY:WinMainCRTStartup")
    
    target_link_options(ResetSettings PRIVATE "/SUBSYSTEM:WINDOWS")
    target_link_options(ResetSettings PRIVATE "/ENTRY:WinMainCRTStartup")
endif()

if (WIN32)
    target_link_libraries(TriVRyad
        PRIVATE
            gdiplus
            user32
            gdi32
            comdlg32
            ole32
            oleaut32
            uuid
    )
    
    target_link_libraries(ResetSettings
        PRIVATE
            user32
            advapi32
    )
endif()