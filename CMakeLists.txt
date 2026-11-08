cmake_minimum_required(VERSION 3.22)

# macOS deployment target and architecture definitions MUST precede project()
if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version" FORCE)
    if (NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Universal binary architectures" FORCE)
    endif()
endif()

# 1. Project definition
project(TheKlangSuite VERSION 0.7.0 LANGUAGES C CXX)

# Feature branch build tag (e.g., "-tooltips"). Stripped upon merge back to master.
set(TKS_FEATURE_TAG "" CACHE STRING "Prerelease feature tag for development builds")
add_compile_definitions(TKS_FEATURE_TAG="${TKS_FEATURE_TAG}")

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
set(TKS_FORMATS VST3 Standalone)
if (APPLE)
    list(APPEND TKS_FORMATS AU)
endif()

# Shared JUCE Modules for Modern Fleet
set(TKS_JUCE_MODULES
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

# ==============================================================================
# 3. Formless Spawn Plugin Target (Tsathogguan Percussion Workstation)
# ==============================================================================
juce_add_plugin(FormlessSpawn
    COMPANY_NAME "Cracked Chimes"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    PLUGIN_MANUFACTURER_CODE CkCh
    PLUGIN_CODE Tfsp
    BUNDLE_ID com.crackedchimes.formlessspawn
    VERSION "1.3.0"
    
    FORMATS ${TKS_FORMATS}
    PRODUCT_NAME "Formless Spawn"
    VST3_CAN_REPLACE_VST2 FALSE
)

target_sources(FormlessSpawn PRIVATE
    source/FastMath.h
    source/FormlessProcessor.h
    source/FormlessProcessor.cpp
    source/FormlessEditor.h
    source/FormlessEditor.cpp
    source/formless_spawn/FormlessControlInfo.h
    source/dsp/drums/FormlessSpawnVoice.h
    source/PresetManager.cpp
)

target_include_directories(FormlessSpawn PRIVATE source)

target_compile_definitions(FormlessSpawn PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

set_target_properties(FormlessSpawn PROPERTIES VST3_AUTO_MANIFEST FALSE)

if (MSVC)
    target_compile_options(FormlessSpawn PRIVATE /utf-8)
endif()

target_link_libraries(FormlessSpawn PRIVATE
    TheKlangDSP
    ${TKS_JUCE_MODULES}
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Formless Spawn Headless UI Fast-Preview Target
add_executable(FormlessSpawn_Preview
    source/FormlessSpawn_Preview_main.cpp
    source/FormlessProcessor.cpp
    source/FormlessEditor.cpp
    source/PresetManager.cpp
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
    ${TKS_JUCE_MODULES}
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# ==============================================================================
# 4. Hastur Plugin Target (The Bass Synth in Yellow - Testbed Edition)
# ==============================================================================
juce_add_plugin(Hastur
    COMPANY_NAME "Cracked Chimes"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    PLUGIN_MANUFACTURER_CODE CkCh
    PLUGIN_CODE Hst1
    BUNDLE_ID com.crackedchimes.hastur
    VERSION "1.3.0"
    
    FORMATS ${TKS_FORMATS}
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
    source/PresetManager.cpp
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
    ${TKS_JUCE_MODULES}
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

if (UNIX AND NOT APPLE)
    target_link_options(Hastur PRIVATE -static-libstdc++ -static-libgcc)
endif()

# Hastur Headless UI Fast-Preview Target
add_executable(Hastur_Preview
    source/hastur/ui_preview_main.cpp
    source/hastur/HasturProcessor.cpp
    source/hastur/HasturEditor.cpp
    source/PresetManager.cpp
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
    ${TKS_JUCE_MODULES}
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# ==============================================================================
# 5. Nyarlat Plugin Target (16-Voice Metamorphic Z-Domain Synthesizer)
# ==============================================================================
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
    
    FORMATS ${TKS_FORMATS}
    PRODUCT_NAME "Nyarlat"
    VST3_CAN_REPLACE_VST2 FALSE
)

target_sources(Nyarlat PRIVATE
    source/FastMath.h
    source/nyarlat/NyarlatRomMetadata.h
    source/nyarlat/NyarlatRom.h
    source/nyarlat/NyarlatAdditiveOscillator.h
    source/nyarlat/NyarlatAnalogFilter.h
    source/nyarlat/NyarlatOscillator.h
    source/nyarlat/NyarlatChaosMatrix.h
    source/nyarlat/AbyssReverb.h
    source/nyarlat/NyarlatVoice.h
    source/nyarlat/NyarlatProcessor.h
    source/nyarlat/NyarlatProcessor.cpp
    source/nyarlat/NyarlatZPlaneRom.cpp
    source/nyarlat/NyarlatEditor.h
    source/nyarlat/NyarlatEditor.cpp
    source/PresetManager.cpp
)

target_include_directories(Nyarlat PRIVATE source)

target_compile_definitions(Nyarlat PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

if (MSVC)
    target_compile_options(Nyarlat PRIVATE /utf-8 /fp:fast /arch:AVX2)
endif()

set_target_properties(Nyarlat PROPERTIES VST3_AUTO_MANIFEST FALSE)

target_link_libraries(Nyarlat PRIVATE
    TheKlangDSP
    ${TKS_JUCE_MODULES}
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

if (UNIX AND NOT APPLE)
    target_link_options(Nyarlat PRIVATE -static-libstdc++ -static-libgcc)
endif()

# Nyarlat Headless UI Fast-Preview Target
add_executable(Nyarlat_Preview
    source/nyarlat/ui_preview_main.cpp
    source/nyarlat/NyarlatProcessor.cpp
    source/nyarlat/NyarlatZPlaneRom.cpp
    source/nyarlat/NyarlatEditor.cpp
    source/PresetManager.cpp
)

target_include_directories(Nyarlat_Preview PRIVATE source)

target_compile_definitions(Nyarlat_Preview PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    NYARLAT_HEADLESS_PREVIEW=1
)

if (MSVC)
    target_compile_options(Nyarlat_Preview PRIVATE /utf-8 /fp:fast /arch:AVX2)
    set_target_properties(Nyarlat_Preview PROPERTIES LINK_FLAGS "/STACK:8388608")
endif()

target_link_libraries(Nyarlat_Preview PRIVATE
    TheKlangDSP
    ${TKS_JUCE_MODULES}
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# ==============================================================================
# 6. Shoggoth Plugin Target (Clean-Room Additive Synthesizer)
# ==============================================================================
juce_add_plugin(Shoggoth
    COMPANY_NAME "Cracked Chimes"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    PLUGIN_MANUFACTURER_CODE CkCh
    PLUGIN_CODE Shgt
    BUNDLE_ID com.crackedchimes.shoggoth
    VERSION "0.1.0"
    
    FORMATS ${TKS_FORMATS}
    PRODUCT_NAME "Shoggoth"
    VST3_CAN_REPLACE_VST2 FALSE
)

target_sources(Shoggoth PRIVATE
    source/FastMath.h
    source/PresetManager.h
    source/PresetManager.cpp
    source/shoggoth/shoggoth_layout.h
    source/shoggoth/ShoggothParameterIds.h
    source/shoggoth/ShoggothParameterLayout.h
    source/shoggoth/ShoggothControlInfo.h
    source/shoggoth/ShoggothLaws.h
    source/shoggoth/ShoggothOscillator.h
    source/shoggoth/ShoggothEnvelopes.h
    source/shoggoth/ShoggothSysExParser.h
    source/shoggoth/ShoggothAudioResynthesizer.h
    source/shoggoth/ShoggothVoice.h
    source/shoggoth/ShoggothProcessor.h
    source/shoggoth/ShoggothProcessor.cpp
    source/shoggoth/ShoggothSpectrumCanvas.h
    source/shoggoth/ShoggothEnvelopeDisplay.h
    source/shoggoth/ShoggothEditor.h
    source/shoggoth/ShoggothEditor.cpp
)

target_include_directories(Shoggoth PRIVATE source)

target_compile_definitions(Shoggoth PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

if (MSVC)
    target_compile_options(Shoggoth PRIVATE /utf-8 /fp:fast /arch:AVX2)
else()
    target_compile_options(Shoggoth PRIVATE -ffast-math -mavx2)
endif()

set_target_properties(Shoggoth PROPERTIES VST3_AUTO_MANIFEST FALSE)

target_link_libraries(Shoggoth PRIVATE
    TheKlangDSP
    ${TKS_JUCE_MODULES}
    PUBLIC
    juce::juce_recommended_config_flags
)

# Shoggoth Headless UI Fast-Preview Target
add_executable(Shoggoth_Preview
    source/shoggoth/ui_preview_main.cpp
    source/PresetManager.cpp
    source/shoggoth/ShoggothProcessor.cpp
    source/shoggoth/ShoggothEditor.cpp
)

target_include_directories(Shoggoth_Preview PRIVATE source)

target_compile_definitions(Shoggoth_Preview PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    SHOGGOTH_HEADLESS_PREVIEW=1
)

if (MSVC)
    target_compile_options(Shoggoth_Preview PRIVATE /utf-8 /fp:fast /arch:AVX2)
    set_target_properties(Shoggoth_Preview PROPERTIES LINK_FLAGS "/STACK:8388608")
endif()

target_link_libraries(Shoggoth_Preview PRIVATE
    TheKlangDSP
    ${TKS_JUCE_MODULES}
    PUBLIC
    juce::juce_recommended_config_flags
)

# ==============================================================================
# 7. DigiStrike FS2 Sovereign Vintage PCM Workstation
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
    VERSION "1.2.0"
    
    FORMATS ${TKS_FORMATS}
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
    source/PresetManager.cpp
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
    ${TKS_JUCE_MODULES}
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

add_executable(DigiStrikeFS2_Preview
    source/digistrike_fs2/DigiStrikeFS2_Preview_main.cpp
    source/digistrike_fs2/DigiStrikeFS2Processor.cpp
    source/digistrike_fs2/DigiStrikeFS2Editor.cpp
    source/PresetManager.cpp
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
    ${TKS_JUCE_MODULES}
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# ==============================================================================
# 8. Test Suites & Benchmarks
# ==============================================================================
add_executable(dsp_tests
    test/dsp_tests.cpp
    source/FastMath.h
    source/PresetManager.h
    source/PresetManager.cpp
)
target_include_directories(dsp_tests PRIVATE source)
target_link_libraries(dsp_tests PRIVATE TheKlangDSP juce::juce_audio_processors)
option(TKS_BUILD_EXPERIMENTAL_TESTS "Include experimental synth suites (Nyarlat) in dsp_tests" OFF)
if(NOT TKS_BUILD_EXPERIMENTAL_TESTS)
    target_compile_definitions(dsp_tests PRIVATE TKS_EXCLUDE_EXPERIMENTAL_SYNTHS=1)
endif()
if(MSVC)
    set_target_properties(dsp_tests PROPERTIES LINK_FLAGS "/STACK:8388608")
endif()

# Standalone Embedded Concurrency & HAL Test Suite
add_executable(test_embedded_concurrency
    test/test_embedded_concurrency.cpp
)
target_include_directories(test_embedded_concurrency PRIVATE source)

# Standalone Multi-Machine Engines (MME) Test Suite
add_executable(test_mme
    test/test_mme.cpp
)
target_link_libraries(test_mme PRIVATE TheKlangDSP)

# FastMath & DSP performance benchmark
add_executable(benchmark_dsp
    test/benchmark_dsp.cpp
    source/FastMath.h
)
target_include_directories(benchmark_dsp PRIVATE source)
target_link_libraries(benchmark_dsp PRIVATE TheKlangDSP juce::juce_audio_processors)

# Automated Headless Plugin-Boundary Harness
add_executable(headless_plugin_runner
    test/headless_plugin_runner.cpp
    test/support/HeadlessX11.cpp
    source/hastur/HasturProcessor.cpp
    source/hastur/HasturEditor.cpp
)
target_link_libraries(headless_plugin_runner PRIVATE
    TheKlangDSP
    ${TKS_JUCE_MODULES}
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_graphics
)
target_include_directories(headless_plugin_runner PRIVATE source test)
target_compile_definitions(headless_plugin_runner PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
if(MSVC)
    target_compile_options(headless_plugin_runner PRIVATE /utf-8)
endif()

# ==============================================================================
# 9. Artifact Gathering (Copy to current_build/)
# ==============================================================================
set(ARTIFACT_DIR "${CMAKE_SOURCE_DIR}/current_build")

# Formless Spawn
add_custom_command(TARGET FormlessSpawn_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/Formless Spawn.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/FormlessSpawn_artefacts/$<CONFIG>/VST3/Formless Spawn.vst3" "${ARTIFACT_DIR}/VST3/Formless Spawn.vst3"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/FormlessSpawn_artefacts/$<CONFIG>/VST3/Formless Spawn.vst3/Contents/Resources/presets/formless_spawn"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets/presets/formless_spawn" "${CMAKE_CURRENT_BINARY_DIR}/FormlessSpawn_artefacts/$<CONFIG>/VST3/Formless Spawn.vst3/Contents/Resources/presets/formless_spawn"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/Formless Spawn.vst3/Contents/Resources/presets/formless_spawn"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets/presets/formless_spawn" "${ARTIFACT_DIR}/VST3/Formless Spawn.vst3/Contents/Resources/presets/formless_spawn"
)

# Hastur
add_custom_command(TARGET Hastur_Standalone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/Standalone"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:Hastur_Standalone>" "${ARTIFACT_DIR}/Standalone/"
)
add_custom_command(TARGET Hastur_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/Hastur.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/Hastur_artefacts/$<CONFIG>/VST3/Hastur.vst3" "${ARTIFACT_DIR}/VST3/Hastur.vst3"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/Hastur_artefacts/$<CONFIG>/VST3/Hastur.vst3/Contents/Resources/presets/hastur"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets/presets/hastur" "${CMAKE_CURRENT_BINARY_DIR}/Hastur_artefacts/$<CONFIG>/VST3/Hastur.vst3/Contents/Resources/presets/hastur"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/Hastur.vst3/Contents/Resources/presets/hastur"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets/presets/hastur" "${ARTIFACT_DIR}/VST3/Hastur.vst3/Contents/Resources/presets/hastur"
)

# DigiStrike FS2
add_custom_command(TARGET DigiStrikeFS2_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/DigiStrike FS2.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/DigiStrikeFS2_artefacts/$<CONFIG>/VST3/DigiStrike FS2.vst3" "${ARTIFACT_DIR}/VST3/DigiStrike FS2.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/digistrike_fs2_rom.bin" "${CMAKE_CURRENT_BINARY_DIR}/DigiStrikeFS2_artefacts/$<CONFIG>/VST3/DigiStrike FS2.vst3/Contents/Resources/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/digistrike_fs2_rom.bin" "${ARTIFACT_DIR}/VST3/DigiStrike FS2.vst3/Contents/Resources/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/DigiStrikeFS2_artefacts/$<CONFIG>/VST3/DigiStrike FS2.vst3/Contents/Resources/presets/digistrike_fs2"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets/presets/digistrike_fs2" "${CMAKE_CURRENT_BINARY_DIR}/DigiStrikeFS2_artefacts/$<CONFIG>/VST3/DigiStrike FS2.vst3/Contents/Resources/presets/digistrike_fs2"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/DigiStrike FS2.vst3/Contents/Resources/presets/digistrike_fs2"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets/presets/digistrike_fs2" "${ARTIFACT_DIR}/VST3/DigiStrike FS2.vst3/Contents/Resources/presets/digistrike_fs2"
)

# Nyarlat
add_custom_command(TARGET Nyarlat_Standalone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/Standalone"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:Nyarlat_Standalone>" "${ARTIFACT_DIR}/Standalone/"
)
add_custom_command(TARGET Nyarlat_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/Nyarlat.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/Nyarlat_artefacts/$<CONFIG>/VST3/Nyarlat.vst3" "${ARTIFACT_DIR}/VST3/Nyarlat.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/nyarlat_akwf_rom.bin" "${CMAKE_CURRENT_BINARY_DIR}/Nyarlat_artefacts/$<CONFIG>/VST3/Nyarlat.vst3/Contents/Resources/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/nyarlat_akwf_rom.bin" "${ARTIFACT_DIR}/VST3/Nyarlat.vst3/Contents/Resources/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/Nyarlat_artefacts/$<CONFIG>/VST3/Nyarlat.vst3/Contents/Resources/presets/nyarlat"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets/presets/nyarlat" "${CMAKE_CURRENT_BINARY_DIR}/Nyarlat_artefacts/$<CONFIG>/VST3/Nyarlat.vst3/Contents/Resources/presets/nyarlat"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/Nyarlat.vst3/Contents/Resources/presets/nyarlat"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets/presets/nyarlat" "${ARTIFACT_DIR}/VST3/Nyarlat.vst3/Contents/Resources/presets/nyarlat"
)

# Shoggoth
add_custom_command(TARGET Shoggoth_Standalone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/Standalone"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:Shoggoth_Standalone>" "${ARTIFACT_DIR}/Standalone/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/Standalone/assets/presets/shoggoth"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets/presets/shoggoth" "${ARTIFACT_DIR}/Standalone/assets/presets/shoggoth"
)
add_custom_command(TARGET Shoggoth_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/Shoggoth.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/Shoggoth_artefacts/$<CONFIG>/VST3/Shoggoth.vst3" "${ARTIFACT_DIR}/VST3/Shoggoth.vst3"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARTIFACT_DIR}/VST3/Shoggoth.vst3/Contents/Resources/presets/shoggoth"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets/presets/shoggoth" "${ARTIFACT_DIR}/VST3/Shoggoth.vst3/Contents/Resources/presets/shoggoth"
)
