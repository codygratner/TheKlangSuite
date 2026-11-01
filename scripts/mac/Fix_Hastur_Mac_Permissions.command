#!/bin/bash
# ==============================================================================
# Hastur HS-01 - Gatekeeper Permission & Quarantine Stripper
# ==============================================================================

clear
echo ""
echo "========================================================================"
echo "    Hastur HS-01 (The Bass Synth in Yellow) - Permission Stripper"
echo "========================================================================"
echo ""
echo "This script removes macOS quarantine attributes from installed plugins"
echo "so your DAWs (Logic, Ableton, FL Studio, Reaper) scan them without warnings."
echo ""

echo "[1/3] Stripping quarantine attributes from VST3 & AU plugins..."
xattr -rd com.apple.quarantine "$HOME/Library/Audio/Plug-Ins/VST3/Hastur"* 2>/dev/null || true
xattr -rd com.apple.quarantine "/Library/Audio/Plug-Ins/VST3/Hastur"* 2>/dev/null || true
xattr -rd com.apple.quarantine "$HOME/Library/Audio/Plug-Ins/Components/Hastur"* 2>/dev/null || true
xattr -rd com.apple.quarantine "/Library/Audio/Plug-Ins/Components/Hastur"* 2>/dev/null || true

echo "[2/3] Stripping quarantine attributes from Standalone Applications..."
xattr -rd com.apple.quarantine "$HOME/Applications/Hastur"* 2>/dev/null || true
xattr -rd com.apple.quarantine "/Applications/Hastur"* 2>/dev/null || true

echo "[3/3] Restarting AudioComponentRegistrar & CoreAudio daemon..."
killall -9 AudioComponentRegistrar 2>/dev/null || true
killall -9 coreaudiod 2>/dev/null || true

echo ""
echo "========================================================================"
echo "  SUCCESS: All permissions unlocked!"
echo "  Launch your DAW and rescan plugins to start producing."
echo "========================================================================"
echo ""
read -p "Press [Enter] to exit..."
exit 0
