cmake_minimum_required(VERSION 3.16)
project(3DEngineProject C)

# 1. Point to the SDL3 folder on C:
set(SDL3_DIR "C:/SDL3/x86_64-w64-mingw32/lib/cmake/SDL3")
set(SDL3_image_DIR "C:/SDL3/x86_64-w64-mingw32/lib/cmake/SDL3_image")
set(SDL3_ttf_DIR "C:/SDL3/x86_64-w64-mingw32/lib/cmake/SDL3_ttf")
set(SDL3_mixer_DIR "C:/SDL3/x86_64-w64-mingw32/lib/cmake/SDL3_mixer")

# 2. Find the SDL3 package
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(SDL3_mixer REQUIRED)

# 3. Define your executable (The app name is 'MyGame')
add_executable(3DEngine main.c)

# 4. Link SDL3 to your code
target_link_libraries(3DEngine PRIVATE 
    SDL3::SDL3
    SDL3_image::SDL3_image 
    SDL3_ttf::SDL3_ttf 
    SDL3_mixer::SDL3_mixer)

# 5. Automatically copy the SDL3.dll to your build folder so it can run
add_custom_command(TARGET 3DEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "C:/SDL3/x86_64-w64-mingw32/bin/SDL3.dll"
    "C:/SDL3/x86_64-w64-mingw32/bin/SDL3_image.dll"
    "C:/SDL3/x86_64-w64-mingw32/bin/SDL3_ttf.dll"
    "C:/SDL3/x86_64-w64-mingw32/bin/SDL3_mixer.dll"
    $<TARGET_FILE_DIR:3DEngine>)

add_custom_command(TARGET 3DEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/Raziel"
    "$<TARGET_FILE_DIR:3DEngine>/Raziel")

# This will copy every PNG, WAV, MP3, and TTF file to the folder where the EXE is
file(GLOB ASSETS_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.wav"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.mp3"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.ttf"
)

add_custom_command(TARGET 3DEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${ASSETS_FILES}
    $<TARGET_FILE_DIR:3DEngine>)