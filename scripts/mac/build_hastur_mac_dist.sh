#!/bin/bash
# ==============================================================================
# Hastur HS-01 - macOS Distribution Packager
# Generates .pkg Installer, stylized .dmg, and portable .zip archive
# ==============================================================================
set -e

TAG="${1:-v0.1.0}"
BUILD_DIR="${2:-build}"
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/../.." && pwd )"

CLEAN_VER="${TAG#hastur-v}"
CLEAN_VER="${CLEAN_VER#hastur-}"
CLEAN_VER="${CLEAN_VER#v}"
CLEAN_TAG="v${CLEAN_VER}"

echo "========================================================================"
echo " Packaging Hastur macOS Distribution for Release: $TAG ($CLEAN_TAG)"
echo " Build directory: $BUILD_DIR"
echo "========================================================================"

# 1. Clean and prepare directories
rm -rf pkg_root pkg_scripts dmg_root macos_dist
mkdir -p pkg_root/Library/Audio/Plug-Ins/VST3
mkdir -p pkg_root/Library/Audio/Plug-Ins/Components
mkdir -p pkg_root/Applications
mkdir -p pkg_scripts
mkdir -p dmg_root
mkdir -p macos_dist

# 2. Locate built artifacts
find "$BUILD_DIR" -name "Hastur.vst3" -exec cp -R -P {} pkg_root/Library/Audio/Plug-Ins/VST3/ \;
find "$BUILD_DIR" -name "Hastur.component" -exec cp -R -P {} pkg_root/Library/Audio/Plug-Ins/Components/ \;
find "$BUILD_DIR" -name "Hastur.app" -exec cp -R -P {} pkg_root/Applications/ \;

# 3. Ad-hoc Codesign all binaries
echo "Codesigning binaries..."
find pkg_root -name "*.vst3" -o -name "*.component" -o -name "*.app" | while read -r item; do
    echo "  Ad-hoc signing: $item"
    codesign --force --deep -s - "$item" 2>/dev/null || true
done

# 4. Generate .pkg Installer with automated postinstall quarantine stripper
echo "Building .pkg Installer..."
cp "$SCRIPT_DIR/postinstall_hastur" pkg_scripts/postinstall
chmod +x pkg_scripts/postinstall

pkgbuild --root pkg_root \
         --identifier com.crackedchimes.hastur.pkg \
         --version "$CLEAN_VER" \
         --install-location "/" \
         --scripts pkg_scripts \
         "Hastur-component.pkg"

productbuild --package "Hastur-component.pkg" \
             "Hastur-${CLEAN_TAG}-macOS-Installer.pkg"

rm -f "Hastur-component.pkg"
echo "  SUCCESS: Hastur-${CLEAN_TAG}-macOS-Installer.pkg generated."

# 5. Generate stylized .dmg with symlinks
echo "Building portable .dmg..."
cp -R -P pkg_root/Library/Audio/Plug-Ins/VST3/* dmg_root/ 2>/dev/null || true
cp -R -P pkg_root/Library/Audio/Plug-Ins/Components/* dmg_root/ 2>/dev/null || true
cp -R -P pkg_root/Applications/* dmg_root/ 2>/dev/null || true

ln -s "/Library/Audio/Plug-Ins/VST3" dmg_root/"Drag to VST3 Folder"
ln -s "/Library/Audio/Plug-Ins/Components" dmg_root/"Drag to AU Components Folder"
ln -s "/Applications" dmg_root/"Drag to Applications"

cp "$SCRIPT_DIR/Fix_Hastur_Mac_Permissions.command" dmg_root/
chmod +x dmg_root/Fix_Hastur_Mac_Permissions.command

hdiutil create -volname "Hastur" \
               -srcfolder dmg_root \
               -ov \
               -format UDZO \
               "Hastur-${CLEAN_TAG}-macOS.dmg"

echo "  SUCCESS: Hastur-${CLEAN_TAG}-macOS.dmg generated."

# 6. Generate portable .zip archive
echo "Building portable .zip..."
cp -R dmg_root/* macos_dist/
cd macos_dist
zip -r -y "../Hastur-${CLEAN_TAG}-macOS.zip" *
cd ..
echo "  SUCCESS: Hastur-${CLEAN_TAG}-macOS.zip generated."

# Clean temporary folders
rm -rf pkg_root pkg_scripts dmg_root macos_dist

echo "========================================================================"
echo " macOS Release Packages Ready:"
ls -lh Hastur-${CLEAN_TAG}-macOS*
echo "========================================================================"
