#!/bin/bash

# Run this script once Kemai is built. Install and AppImage will be handle here.
# It just need path to Qt and to build directory.

# Read args
usage() { echo "Usage: $0 -b <build dir> -o <openssl srcdir> -q <qt dir>"; exit 0; }

while getopts ":hb:o:q:" o; do
  case "${o}" in
    b)
      BUILD_DIR=${OPTARG}
      ;;
    o)
      OPENSSL_DIR=${OPTARG}
      ;;
    q)
      QT_DIR=${OPTARG}
      ;;
    h | *)
      usage
      ;;
  esac
done

if [[ -z ${BUILD_DIR+x} ]] || [[ -z ${OPENSSL_DIR+x} ]] || [[ -z ${QT_DIR+x} ]]; then
  usage
fi

# Cleanup
APPIMAGE_DESTDIR=${BUILD_DIR}/AppDir
rm -r ${APPIMAGE_DESTDIR}

# Run install to AppDir and add linux files
DESTDIR=AppDir cmake --build ${BUILD_DIR} --target install || exit 1

# Copy sysroot
cp -rv bundle/linux/sysroot/* ${APPIMAGE_DESTDIR}/
chmod +x ${APPIMAGE_DESTDIR}/usr/bin/kemai-wrapper.sh

# Gets AppImage tools
wget -nc -P ${BUILD_DIR}/ https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget -nc -P ${BUILD_DIR}/ https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x ${BUILD_DIR}/linuxdeploy*.AppImage

# Export vars
export QMAKE=${QT_DIR}/bin/qmake
export LD_LIBRARY_PATH=${QT_DIR}/lib
export VERSION=$(cat ${BUILD_DIR}/version.txt)
export EXTRA_QT_PLUGINS=platforms,iconengines,wayland-decoration-client,wayland-graphics-integration-client,wayland-shell-integration,platformthemes,tls

# Copy OpenSSLv3 binaries
OPENSSL_APPDIR=${APPIMAGE_DESTDIR}/fallback/libssl.so.3
mkdir -p ${OPENSSL_APPDIR}
cp -vL ${OPENSSL_DIR}/libssl.so* ${OPENSSL_APPDIR}/
cp -vL ${OPENSSL_DIR}/libcrypto.so* ${OPENSSL_APPDIR}/

# Run appimage builder
${BUILD_DIR}/linuxdeploy-x86_64.AppImage \
  --appimage-extract-and-run \
  --appdir ${APPIMAGE_DESTDIR} \
  --plugin qt \
  -d ${APPIMAGE_DESTDIR}/kemai-wrapper.desktop \
  -i ${APPIMAGE_DESTDIR}/kemai-wrapper.png \
  --output appimage
