cmake_minimum_required(VERSION 3.22)

# macOS deployment target and architecture definitions MUST precede project()
if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version" FORCE)
    if (NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Universal binary architectures" FORCE)
    endif()
endif()

# 1. Project definition
project(TheKlangSuite VERSION 0.4.0 LANGUAGES C CXX)

# Feature branch build tag (e.g., "-tooltips"). Stripped upon merge back to master.
set(TKF_FEATURE_TAG "" CACHE STRING "Prerelease feature tag for development builds")
add_compile_definitions(TKF_FEATURE_TAG="${TKF_FEATURE_TAG}")

# Global JUCE configuration: disable unnecessary WebBrowser and cURL dependencies across all targets
add_compile_definitions(
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

option(TK_STRICT_WARNINGS "Treat warnings as errors (for CI)" OFF)
option(TK_USE_ASAN "Enable Address/Thread Sanitizer" OFF)

# Enforce modern C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Add the cloned JUCE repository
add_subdirectory(JUCE)

# 2b. Add Sovereign Shared TheKlangDSP engine
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../TheKlangDSP/CMakeLists.txt")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../TheKlangDSP" TheKlangDSP EXCLUDE_FROM_ALL)
endif()

# CI Toolchain Lockdown & Sanitizer Integration (applied to Klang Suite targets)
if (TK_STRICT_WARNINGS)
    if (MSVC)
        add_compile_options(/WX)
    else()
        add_compile_options(-Werror)
    endif()
endif()

if (TK_USE_ASAN)
    if (NOT MSVC)
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
    else()
        add_compile_options(/fsanitize=address)
    endif()
endif()

# Configure plugin formats per platform
set(TKF_FORMATS VST3 Standalone)
if (APPLE)
    list(APPEND TKF_FORMATS AU)
endif()

# Glob all JSON files in the controls, themes, and text directories
file(GLOB CONTROL_JSON_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/controls/*.json")
file(GLOB THEME_JSON_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/themes/*.json")
file(GLOB TEXT_JSON_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/text/*.json")

juce_add_binary_data(TkfAssets
    HEADER_NAME "TkfAssets.h"
    NAMESPACE TkfAssets
    SOURCES
    ${CONTROL_JSON_FILES}
    ${THEME_JSON_FILES}
    ${TEXT_JSON_FILES}
)

# 3. Configure the plugin target
juce_add_plugin(TheKlangFarmer
    COMPANY_NAME "R'lyeh Sound"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    # 4-character codes (required by VST/AU specs; must have at least 1 uppercase)
    PLUGIN_MANUFACTURER_CODE Rlyh
    PLUGIN_CODE Tkf1
    BUNDLE_ID com.rlyehsound.theklangfarmer
    
    FORMATS ${TKF_FORMATS}
    PRODUCT_NAME "The Klang Farmer"
    VST3_CAN_REPLACE_VST2 FALSE
)

# 4. Add source files
target_sources(TheKlangFarmer PRIVATE
    source/FastMath.h
    source/DSPBlock.h
    source/ModularBlocks.h
    source/ModulationEngine.h
    source/ParameterManager.h
    source/ParameterManager.cpp
    source/UIComponents.h
    source/KlangCoreProcessor.h
    source/KlangCoreProcessor.cpp
    source/KlangCoreEditor.h
    source/KlangCoreEditor.cpp
    source/UIComponents.cpp
    source/FarmerProcessor.h
    source/FarmerProcessor.cpp
    source/FarmerEditor.h
    source/FarmerEditor.cpp
    source/VersionChecker.h
    source/VersionChecker.cpp
    source/SettingsModal.h
    source/SettingsModal.cpp
    source/DevLogger.h
)

target_include_directories(TheKlangFarmer PRIVATE
    source
)

target_compile_definitions(TheKlangFarmer PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

set_target_properties(TheKlangFarmer PROPERTIES VST3_AUTO_MANIFEST FALSE)

set(TKF_JUCE_MODULES
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

# 5. Link required JUCE modules for The Klang Farmer
target_link_libraries(TheKlangFarmer PRIVATE
    TheKlangDSP
    ${TKF_JUCE_MODULES}
    TkfAssets
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# --- 6. The Klang Planter Plugin Target ---
juce_add_plugin(TheKlangPlanter
    COMPANY_NAME "R'lyeh Sound"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    PLUGIN_MANUFACTURER_CODE Rlyh
    PLUGIN_CODE Tkp1
    BUNDLE_ID com.rlyehsound.theklangplanter
    
    FORMATS ${TKF_FORMATS}
    PRODUCT_NAME "The Klang Planter"
    VST3_CAN_REPLACE_VST2 FALSE
)

target_sources(TheKlangPlanter PRIVATE
    source/FastMath.h
    source/DSPBlock.h
    source/ModularBlocks.h
    source/PlanterEngine.h
    source/ParameterManager.h
    source/ParameterManager.cpp
    source/UIComponents.h
    source/KlangCoreProcessor.h
    source/KlangCoreProcessor.cpp
    source/KlangCoreEditor.h
    source/KlangCoreEditor.cpp
    source/UIComponents.cpp
    source/PlanterProcessor.h
    source/PlanterProcessor.cpp
    source/PlanterEditor.h
    source/PlanterEditor.cpp
    source/VersionChecker.h
    source/VersionChecker.cpp
    source/SettingsModal.h
    source/SettingsModal.cpp
    source/DevLogger.h
)

target_include_directories(TheKlangPlanter PRIVATE source)

target_compile_definitions(TheKlangPlanter PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

set_target_properties(TheKlangPlanter PROPERTIES VST3_AUTO_MANIFEST FALSE)

target_link_libraries(TheKlangPlanter PRIVATE
    TheKlangDSP
    ${TKF_JUCE_MODULES}
    TkfAssets
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Cross-platform Linux static stdlib linking
if (UNIX AND NOT APPLE)
    target_link_options(TheKlangFarmer PRIVATE -static-libstdc++ -static-libgcc)
    target_link_options(TheKlangPlanter PRIVATE -static-libstdc++ -static-libgcc)
endif()

# 7. Test suite executable
add_executable(dsp_tests
    test/dsp_tests.cpp
    source/FastMath.h
    source/DSPBlock.h
    source/ModularBlocks.h
    source/ModulationEngine.h
    source/PlanterEngine.h
)
target_include_directories(dsp_tests PRIVATE source)
target_link_libraries(dsp_tests PRIVATE TheKlangDSP TkfAssets juce::juce_audio_processors)

# 8. FastMath & DSP performance benchmark
add_executable(benchmark_dsp
    test/benchmark_dsp.cpp
    source/FastMath.h
)
target_include_directories(benchmark_dsp PRIVATE source)

# 9. Screenshot snapshot generator
add_executable(capture_screenshot
    test/capture_screenshot.cpp
)
target_link_libraries(capture_screenshot PRIVATE
    TheKlangFarmer
    juce::juce_gui_basics
    juce::juce_graphics
)
target_include_directories(capture_screenshot PRIVATE source)

add_executable(capture_planter_screenshot
    test/capture_planter_screenshot.cpp
)
target_link_libraries(capture_planter_screenshot PRIVATE
    TheKlangPlanter
    juce::juce_gui_basics
    juce::juce_graphics
)
target_include_directories(capture_planter_screenshot PRIVATE source)




execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE TKF_GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND git rev-list --count HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE TKF_GIT_COMMIT_COUNT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# 10. Standalone JSON Data & Theme Editor
juce_add_gui_app(TheKlangEditor
    PRODUCT_NAME "The Klang Editor"
    VERSION ${PROJECT_VERSION}
)
juce_generate_juce_header(TheKlangEditor)
target_sources(TheKlangEditor PRIVATE
    tools/editor/Main.cpp
    tools/editor/MainComponent.cpp
    # Shared sources
    source/UIComponents.cpp
    source/ParameterManager.cpp
    source/DevLogger.h
)
target_link_libraries(TheKlangEditor PRIVATE
    ${TKF_JUCE_MODULES}
    TkfAssets
)
target_include_directories(TheKlangEditor PRIVATE source tools/editor)
target_compile_definitions(TheKlangEditor PRIVATE
    TKF_GIT_HASH="${TKF_GIT_HASH}"
    TKF_GIT_COMMIT_COUNT="${TKF_GIT_COMMIT_COUNT}"
)

if (UNIX AND NOT APPLE)
    target_link_options(TheKlangEditor PRIVATE -static-libstdc++ -static-libgcc)
endif()





# ==============================================================================
# 11. Artifact Gathering (Copy to current_build/)
# ==============================================================================
set(ARTIFACT_DIR "${CMAKE_SOURCE_DIR}/current_build")

# The Klang Farmer
add_custom_command(TARGET TheKlangFarmer_Standalone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/Standalone"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:TheKlangFarmer_Standalone>" "${ARTIFACT_DIR}/Standalone/"
)
add_custom_command(TARGET TheKlangFarmer_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/The Klang Farmer.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/TheKlangFarmer_artefacts/$<CONFIG>/VST3/The Klang Farmer.vst3" "${ARTIFACT_DIR}/VST3/The Klang Farmer.vst3"
)

# The Klang Planter
add_custom_command(TARGET TheKlangPlanter_Standalone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/Standalone"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:TheKlangPlanter_Standalone>" "${ARTIFACT_DIR}/Standalone/"
)
add_custom_command(TARGET TheKlangPlanter_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/The Klang Planter.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/TheKlangPlanter_artefacts/$<CONFIG>/VST3/The Klang Planter.vst3" "${ARTIFACT_DIR}/VST3/The Klang Planter.vst3"
)

# The Klang Editor
add_custom_command(TARGET TheKlangEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/Editor"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:TheKlangEditor>" "${ARTIFACT_DIR}/Editor/"
)


# 11. Automated GUI Test Harness
add_executable(gui_tests
    test/gui_tests.cpp
    test/ChaosMonkeySuite.h
    source/PlanterProcessor.cpp
    source/PlanterEditor.cpp
    tools/editor/MainComponent.cpp
)
target_link_libraries(gui_tests PRIVATE
    TheKlangFarmer
    juce::juce_gui_basics
    juce::juce_graphics
)
target_include_directories(gui_tests PRIVATE source tools/editor)
target_compile_definitions(gui_tests PRIVATE TKF_GUI_TESTS=1 JUCE_MODAL_LOOPS_PERMITTED=1)

# ==============================================================================
# 12. Automated C++ Linting
# ==============================================================================
find_program(CLANG_FORMAT_BIN clang-format)
if(CLANG_FORMAT_BIN)
    file(GLOB_RECURSE FORMAT_SOURCES 
        "source/*.cpp" "source/*.h"
        "tools/editor/*.cpp" "tools/editor/*.h"
        "test/*.cpp"
    )
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_BIN} -i ${FORMAT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-format on all source files..."
    )
endif()


