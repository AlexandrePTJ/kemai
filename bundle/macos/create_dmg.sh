#!/bin/bash

set -x
set -e

# Run this script once Kemai is built. Install and AppImage will be handle here.
# It just path to Qt and to build directory.


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
rm dist/*.dmg || true

# Run install to dist
cmake --build $build_path --target install

# Gets create-dmg tools
brew install create-dmg

# Export vars
export VERSION=$(cat ${build_path}/version.txt)

# Run dmg builder
pushd dist
${qt_path}/bin/macdeployqt Kemai.app -executable=Kemai.app/Contents/MacOS/Kemai -always-overwrite
create-dmg --volicon Kemai.app/Contents/Resources/kemai.icns --app-drop-link 0 0 --skip-jenkins Kemai-$VERSION.dmg Kemai.app
