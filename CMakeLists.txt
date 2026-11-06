cmake_minimum_required(VERSION 3.22)

# macOS deployment target and architecture definitions MUST precede project()
if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version" FORCE)
    if (NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Universal binary architectures" FORCE)
    endif()
endif()

# 1. Project definition
project(TheKlangSuite VERSION 0.5.0 LANGUAGES C CXX)

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

# 2b. Add Sovereign Shared TheKlangDSP engine (with self-contained CI fallback)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../TheKlangDSP/CMakeLists.txt")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../TheKlangDSP" TheKlangDSP EXCLUDE_FROM_ALL)
else()
    add_library(TheKlangDSP INTERFACE)
    target_include_directories(TheKlangDSP INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/source>
    )
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
    COMPANY_NAME "Cracked Chimes"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    # 4-character codes (required by VST/AU specs; must have at least 1 uppercase)
    PLUGIN_MANUFACTURER_CODE CkCh
    PLUGIN_CODE Tkf1
    BUNDLE_ID com.crackedchimes.theklangfarmer
    
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
    source/StateMigrator.h
    source/PresetManager.h
    source/PresetManager.cpp
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
    COMPANY_NAME "Cracked Chimes"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    PLUGIN_MANUFACTURER_CODE CkCh
    PLUGIN_CODE Tkp1
    BUNDLE_ID com.crackedchimes.theklangplanter
    
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
# --- 6b. The Shard Plugin Target (Sovereign Pantheon) ---
juce_add_plugin(TheShard
    COMPANY_NAME "Cracked Chimes"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    PLUGIN_MANUFACTURER_CODE CkCh
    PLUGIN_CODE Tsh1
    BUNDLE_ID com.crackedchimes.theshard
    
    FORMATS ${TKF_FORMATS}
    PRODUCT_NAME "The Shard"
    VST3_CAN_REPLACE_VST2 FALSE
)

target_sources(TheShard PRIVATE
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
    source/ShardEditor.h
    source/ShardEditor.cpp
    source/VersionChecker.h
    source/VersionChecker.cpp
    source/SettingsModal.h
    source/SettingsModal.cpp
    source/DevLogger.h
)

target_include_directories(TheShard PRIVATE source)

target_compile_definitions(TheShard PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    TK_IS_THE_SHARD=1
)

set_target_properties(TheShard PROPERTIES VST3_AUTO_MANIFEST FALSE)

target_link_libraries(TheShard PRIVATE
    TheKlangDSP
    ${TKF_JUCE_MODULES}
    TkfAssets
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# --- 6c. Formless Spawn Plugin Target (Tsathogguan Percussion Workstation) ---
juce_add_plugin(FormlessSpawn
    COMPANY_NAME "Cracked Chimes"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    PLUGIN_MANUFACTURER_CODE CkCh
    PLUGIN_CODE Tfsp
    BUNDLE_ID com.crackedchimes.formlessspawn
    VERSION "1.0.1"
    
    FORMATS ${TKF_FORMATS}
    PRODUCT_NAME "Formless Spawn"
    VST3_CAN_REPLACE_VST2 FALSE
)

target_sources(FormlessSpawn PRIVATE
    source/FastMath.h
    source/UIComponents.h
    source/UIComponents.cpp
    source/FormlessProcessor.h
    source/FormlessProcessor.cpp
    source/FormlessEditor.h
    source/FormlessEditor.cpp
    source/formless_spawn/FormlessControlInfo.h
    source/dsp/drums/FormlessSpawnVoice.h
)

target_include_directories(FormlessSpawn PRIVATE source)

target_compile_definitions(FormlessSpawn PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

set_target_properties(FormlessSpawn PROPERTIES VST3_AUTO_MANIFEST FALSE)

target_link_libraries(FormlessSpawn PRIVATE
    TheKlangDSP
    ${TKF_JUCE_MODULES}
    TkfAssets
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)


# --- 6e. Hastur Plugin Target (The Bass Synth in Yellow - Testbed Edition) ---
juce_add_plugin(Hastur
    COMPANY_NAME "Cracked Chimes"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    PLUGIN_MANUFACTURER_CODE CkCh
    PLUGIN_CODE Hst1
    BUNDLE_ID com.crackedchimes.hastur
    VERSION "1.0.6"
    
    FORMATS ${TKF_FORMATS}
    PRODUCT_NAME "Hastur"
    VST3_CAN_REPLACE_VST2 FALSE
)

target_sources(Hastur PRIVATE
    source/FastMath.h
    source/hastur/VactrolLPG.h
    source/hastur/LadderFilter.h
    source/hastur/HasturDCO.h
    source/hastur/AlphaOscillator.h
    source/hastur/AllPassFilter4.h
    source/hastur/AnalogPLL.h
    source/hastur/ModMatrix.h
    source/hastur/DualSvfFilter.h
    source/hastur/PllSubOsc.h
    source/hastur/SampleHoldNoise.h
    source/hastur/CodyBassEnvelope.h
    source/hastur/VortexEnvelope.h
    source/hastur/HasturVoice.h
    source/hastur/HasturProcessor.h
    source/hastur/HasturProcessor.cpp
    source/hastur/HasturEditor.h
    source/hastur/HasturEditor.cpp
)

target_include_directories(Hastur PRIVATE source)

target_compile_definitions(Hastur PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

if (MSVC)
    target_compile_options(Hastur PRIVATE /utf-8)
endif()

set_target_properties(Hastur PROPERTIES VST3_AUTO_MANIFEST FALSE)

target_link_libraries(Hastur PRIVATE
    TheKlangDSP
    ${TKF_JUCE_MODULES}
    TkfAssets
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

if (UNIX AND NOT APPLE)
    target_link_options(Hastur PRIVATE -static-libstdc++ -static-libgcc)
endif()

# --- 6f. Nyarlat Plugin Target (16-Voice Metamorphic Z-Domain Synthesizer) ---
juce_add_plugin(Nyarlat
    COMPANY_NAME "Cracked Chimes"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    PLUGIN_MANUFACTURER_CODE CkCh
    PLUGIN_CODE Nyrt
    BUNDLE_ID com.crackedchimes.nyarlat
    VERSION "1.0.0"
    
    FORMATS ${TKF_FORMATS}
    PRODUCT_NAME "Nyarlat"
    VST3_CAN_REPLACE_VST2 FALSE
)

target_sources(Nyarlat PRIVATE
    source/FastMath.h
    source/nyarlat/NyarlatRomMetadata.h
    source/nyarlat/NyarlatRom.h
    source/nyarlat/NyarlatOscillator.h
    source/nyarlat/ZDomainFilter14.h
    source/nyarlat/NyarlatChaosMatrix.h
    source/nyarlat/AbyssReverb.h
    source/nyarlat/NyarlatVoice.h
    source/nyarlat/NyarlatProcessor.h
    source/nyarlat/NyarlatProcessor.cpp
    source/nyarlat/ZCubeVisualizer.h
    source/nyarlat/NyarlatEditor.h
    source/nyarlat/NyarlatEditor.cpp
)

target_include_directories(Nyarlat PRIVATE source)

target_compile_definitions(Nyarlat PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

if (MSVC)
    target_compile_options(Nyarlat PRIVATE /utf-8 /fp:fast)
endif()

set_target_properties(Nyarlat PROPERTIES VST3_AUTO_MANIFEST FALSE)

target_link_libraries(Nyarlat PRIVATE
    TheKlangDSP
    ${TKF_JUCE_MODULES}
    TkfAssets
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

if (UNIX AND NOT APPLE)
    target_link_options(Nyarlat PRIVATE -static-libstdc++ -static-libgcc)
endif()



# --- 6f. Hastur Headless UI Fast-Preview Target ---
add_executable(Hastur_Preview
    source/hastur/ui_preview_main.cpp
    source/hastur/HasturProcessor.cpp
    source/hastur/HasturEditor.cpp
)

target_include_directories(Hastur_Preview PRIVATE source)

target_compile_definitions(Hastur_Preview PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    HASTUR_HEADLESS_PREVIEW=1
)

if (MSVC)
    target_compile_options(Hastur_Preview PRIVATE /utf-8)
endif()

target_link_libraries(Hastur_Preview PRIVATE
    TheKlangDSP
    ${TKF_JUCE_MODULES}
    TkfAssets
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# --- 6f2. Nyarlat Headless UI Fast-Preview Target ---
add_executable(Nyarlat_Preview
    source/nyarlat/ui_preview_main.cpp
    source/nyarlat/NyarlatProcessor.cpp
    source/nyarlat/NyarlatEditor.cpp
)

target_include_directories(Nyarlat_Preview PRIVATE source)

target_compile_definitions(Nyarlat_Preview PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    NYARLAT_HEADLESS_PREVIEW=1
)

if (MSVC)
    target_compile_options(Nyarlat_Preview PRIVATE /utf-8 /fp:fast)
    set_target_properties(Nyarlat_Preview PROPERTIES LINK_FLAGS "/STACK:8388608")
endif()

target_link_libraries(Nyarlat_Preview PRIVATE
    TheKlangDSP
    ${TKF_JUCE_MODULES}
    TkfAssets
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# --- 6g. Formless Spawn Headless UI Fast-Preview Target ---
add_executable(FormlessSpawn_Preview
    source/FormlessSpawn_Preview_main.cpp
    source/ParameterManager.cpp
    source/UIComponents.cpp
    source/FormlessProcessor.cpp
    source/FormlessEditor.cpp
)

target_include_directories(FormlessSpawn_Preview PRIVATE source)

target_compile_definitions(FormlessSpawn_Preview PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

if (MSVC)
    target_compile_options(FormlessSpawn_Preview PRIVATE /utf-8)
endif()

target_link_libraries(FormlessSpawn_Preview PRIVATE
    TheKlangDSP
    ${TKF_JUCE_MODULES}
    TkfAssets
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)


if (UNIX AND NOT APPLE)
    target_link_options(TheShard PRIVATE -static-libstdc++ -static-libgcc)
endif()

# ==============================================================================
# DigiStrike FS2 Sovereign Vintage PCM Workstation
# ==============================================================================
juce_add_plugin(DigiStrikeFS2
    COMPANY_NAME "Cracked Chimes"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    PLUGIN_MANUFACTURER_CODE CkCh
    PLUGIN_CODE Dfs2
    BUNDLE_ID com.crackedchimes.digistrikefs2
    VERSION "1.0.0"
    
    FORMATS ${TKF_FORMATS}
    PRODUCT_NAME "DigiStrike FS2"
    VST3_CAN_REPLACE_VST2 FALSE
)

target_sources(DigiStrikeFS2 PRIVATE
    source/FastMath.h
    source/dsp/drums/DigiStrikeFS2Voice.h
    source/digistrike_fs2/DigiStrikeFS2Catalog.h
    source/digistrike_fs2/DigiStrikeFS2ControlInfo.h
    source/digistrike_fs2/digistrike_fs2ParameterIds.h
    source/digistrike_fs2/digistrike_fs2ParameterLayout.h
    source/digistrike_fs2/DigiStrikeFS2Processor.h
    source/digistrike_fs2/DigiStrikeFS2Processor.cpp
    source/digistrike_fs2/DigiStrikeFS2Editor.h
    source/digistrike_fs2/DigiStrikeFS2Editor.cpp
)

target_include_directories(DigiStrikeFS2 PRIVATE source source/digistrike_fs2)

target_compile_definitions(DigiStrikeFS2 PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

if (MSVC)
    target_compile_options(DigiStrikeFS2 PRIVATE /utf-8)
endif()

set_target_properties(DigiStrikeFS2 PROPERTIES VST3_AUTO_MANIFEST FALSE)

target_link_libraries(DigiStrikeFS2 PRIVATE
    TheKlangDSP
    ${TKF_JUCE_MODULES}
    TkfAssets
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

add_executable(DigiStrikeFS2_Preview
    source/digistrike_fs2/DigiStrikeFS2_Preview_main.cpp
    source/digistrike_fs2/DigiStrikeFS2Processor.cpp
    source/digistrike_fs2/DigiStrikeFS2Editor.cpp
)

target_include_directories(DigiStrikeFS2_Preview PRIVATE source source/digistrike_fs2)

target_compile_definitions(DigiStrikeFS2_Preview PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

if (MSVC)
    target_compile_options(DigiStrikeFS2_Preview PRIVATE /utf-8)
endif()

target_link_libraries(DigiStrikeFS2_Preview PRIVATE
    TheKlangDSP
    ${TKF_JUCE_MODULES}
    TkfAssets
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)


# 7. Test suite executable
add_executable(dsp_tests
    test/dsp_tests.cpp
    source/FastMath.h
    source/DSPBlock.h
    source/ModularBlocks.h
    source/ModulationEngine.h
    source/PlanterEngine.h
    source/StateMigrator.h
    source/PresetManager.h
    source/PresetManager.cpp
)
target_include_directories(dsp_tests PRIVATE source)
target_link_libraries(dsp_tests PRIVATE TheKlangDSP TkfAssets juce::juce_audio_processors)
if(MSVC)
    set_target_properties(dsp_tests PROPERTIES LINK_FLAGS "/STACK:8388608")
endif()

# 7.5 Standalone Embedded Concurrency & HAL Test Suite
add_executable(test_embedded_concurrency
    test/test_embedded_concurrency.cpp
)
target_include_directories(test_embedded_concurrency PRIVATE source)

# 7.6 Standalone Multi-Machine Engines (MME) & ElektronTrackWrapper Test Suite
add_executable(test_mme
    test/test_mme.cpp
)
# target_include_directories(test_mme PRIVATE source)
target_link_libraries(test_mme PRIVATE TheKlangDSP)

# 8. FastMath & DSP performance benchmark
add_executable(benchmark_dsp
    test/benchmark_dsp.cpp
    source/FastMath.h
)
target_include_directories(benchmark_dsp PRIVATE source)

# 9. Screenshot snapshot generator
add_executable(capture_screenshot
    test/capture_screenshot.cpp
    test/support/HeadlessX11.cpp
)
target_link_libraries(capture_screenshot PRIVATE
    TheKlangFarmer
    juce::juce_gui_basics
    juce::juce_graphics
)
target_include_directories(capture_screenshot PRIVATE source test)

add_executable(capture_planter_screenshot
    test/capture_planter_screenshot.cpp
    test/support/HeadlessX11.cpp
)
target_link_libraries(capture_planter_screenshot PRIVATE
    TheKlangPlanter
    juce::juce_gui_basics
    juce::juce_graphics
)
target_include_directories(capture_planter_screenshot PRIVATE source test)




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
# The Shard
add_custom_command(TARGET TheShard_Standalone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/Standalone"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:TheShard_Standalone>" "${ARTIFACT_DIR}/Standalone/"
)
add_custom_command(TARGET TheShard_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/The Shard.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/TheShard_artefacts/$<CONFIG>/VST3/The Shard.vst3" "${ARTIFACT_DIR}/VST3/The Shard.vst3"
)


# Hastur
add_custom_command(TARGET Hastur_Standalone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/Standalone"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:Hastur_Standalone>" "${ARTIFACT_DIR}/Standalone/"
)
add_custom_command(TARGET Hastur_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/Hastur.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/Hastur_artefacts/$<CONFIG>/VST3/Hastur.vst3" "${ARTIFACT_DIR}/VST3/Hastur.vst3"
)

# DigiStrike FS2
add_custom_command(TARGET DigiStrikeFS2_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/DigiStrike FS2.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/DigiStrikeFS2_artefacts/$<CONFIG>/VST3/DigiStrike FS2.vst3" "${ARTIFACT_DIR}/VST3/DigiStrike FS2.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/digistrike_fs2_rom.bin" "${CMAKE_CURRENT_BINARY_DIR}/DigiStrikeFS2_artefacts/$<CONFIG>/VST3/DigiStrike FS2.vst3/Contents/Resources/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/digistrike_fs2_rom.bin" "${ARTIFACT_DIR}/VST3/DigiStrike FS2.vst3/Contents/Resources/"
)

# Nyarlat
add_custom_command(TARGET Nyarlat_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/Nyarlat.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/Nyarlat_artefacts/$<CONFIG>/VST3/Nyarlat.vst3" "${ARTIFACT_DIR}/VST3/Nyarlat.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/nyarlat_akwf_rom.bin" "${CMAKE_CURRENT_BINARY_DIR}/Nyarlat_artefacts/$<CONFIG>/VST3/Nyarlat.vst3/Contents/Resources/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/nyarlat_akwf_rom.bin" "${ARTIFACT_DIR}/VST3/Nyarlat.vst3/Contents/Resources/"
)


# The Klang Editor
add_custom_command(TARGET TheKlangEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/Editor"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:TheKlangEditor>" "${ARTIFACT_DIR}/Editor/"
)


# 11b. Automated Headless Plugin-Boundary Harness
add_executable(headless_plugin_runner
    test/headless_plugin_runner.cpp
    test/support/HeadlessX11.cpp
    source/hastur/HasturProcessor.cpp
    source/hastur/HasturEditor.cpp
)
target_link_libraries(headless_plugin_runner PRIVATE
    TheKlangDSP
    TkfAssets
    ${TKF_JUCE_MODULES}
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_graphics
)
target_include_directories(headless_plugin_runner PRIVATE source test)
target_compile_definitions(headless_plugin_runner PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
if(MSVC)
    target_compile_options(headless_plugin_runner PRIVATE /utf-8)
endif()

# 11. Automated GUI Test Harness
add_executable(gui_tests
    test/gui_tests.cpp
    test/support/HeadlessX11.cpp
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
target_include_directories(gui_tests PRIVATE source tools/editor test)
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


