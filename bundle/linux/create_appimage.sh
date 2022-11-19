#!/bin/bash

set -x
set -e

# Run this script once Kemai is built. Install and AppImage will be handle here.
# It just need path to Qt and to build directory.


# Read args
qt_path=${qt_path:-}
build_path=${build_path:-}

while [ $# -gt 0 ]; do
	if [[ $1 == *"--"* ]]; then
		param="${1/--/}"
		declare $param="$2"
	fi
	shift
done

# Cleanup
rm $build_path/*.AppImage* || true

# Run install to AppDir and add linux files
DESTDIR=AppDir cmake --build $build_path --target install
cp bundle/linux/kemai.desktop $build_path/AppDir/
cp bundle/linux/kemai.png $build_path/AppDir/

# Gets AppImage tools
pushd "$build_path"
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy*.AppImage

# Export vars
export QMAKE=$qt_path/bin/qmake
export LD_LIBRARY_PATH=$qt_path/lib
export VERSION=$(cat version.txt)
export EXTRA_QT_PLUGINS=platforms,iconengines,wayland-decoration-client,wayland-graphics-integration-client,wayland-shell-integration,platformthemes,tls

# Run appimage builder
./linuxdeploy-x86_64.AppImage --appdir AppDir --plugin qt --output appimage

