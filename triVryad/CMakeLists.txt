cmake_minimum_required(VERSION 3.16)

cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(TriVRyad LANGUAGES CXX)

if (MSVC)
    foreach(flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif()
    endforeach()
endif()

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

set_property(TARGET TriVRyad PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET ResetSettings PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

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
    
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_link_options(TriVRyad PRIVATE "-static-libgcc" "-static-libstdc++" "-static")
        target_link_options(ResetSettings PRIVATE "-static-libgcc" "-static-libstdc++" "-static")
    endif()
endif()
