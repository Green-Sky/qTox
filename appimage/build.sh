#!/usr/bin/env bash

# SPDX-License-Identifier: GPL-3.0-or-later.
# Copyright © 2019 by The qTox Project Contributors
# Copyright © 2024 The TokTok team

# Fail out on error
set -exuo pipefail

usage() {
    echo "$0 --src-dir SRC_DIR"
    echo "Builds an app image in the CWD based off qtox installation at SRC_DIR"
}

while (( $# > 0 )); do
    case $1 in
        --src-dir) QTOX_SRC_DIR=$2; shift 2 ;;
        --help|-h) usage; exit 1 ;;
        *) echo "Unexpected argument $1"; usage; exit 1;;
    esac
done

if [ -z "${QTOX_SRC_DIR+x}" ]; then
    echo "--src-dir is a required argument"
    usage
    exit 1
fi

# directory paths
BUILD_DIR=$(realpath .)
readonly BUILD_DIR
QTOX_APP_DIR="$BUILD_DIR"/appdir
readonly QTOX_APP_DIR
readonly LOCAL_LIB_DIR="$QTOX_APP_DIR"/local/lib

wget -c https://github.com/$(wget -q https://github.com/probonopd/go-appimage/releases/expanded_assets/continuous -O - | grep "appimagetool-.*-x86_64.AppImage" | head -n 1 | cut -d '"' -f 2)
chmod +x appimagetool-*.AppImage

# update information to be embeded in AppImage
#readonly UPDATE_INFO="gh-releases-zsync|TokTok|qTox|latest|qTox-*.x86_64.AppImage.zsync"
#export GIT_VERSION=$(git -C "${QTOX_SRC_DIR}" rev-parse --short HEAD)

echo $QTOX_APP_DIR
cmake "${QTOX_SRC_DIR}" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DDESKTOP_NOTIFICATIONS=ON \
  -DUPDATE_CHECK=ON
cmake --build .
rm -fr appdir
#make DESTDIR=appdir install
cmake --install . --prefix appdir/usr

unset QTDIR; unset QT_PLUGIN_PATH; unset LD_LIBRARY_PATH;

export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib/x86_64-linux-gnu/

eval ./appimagetool-*.AppImage --appimage-extract-and-run -s deploy $QTOX_APP_DIR/usr/share/applications/*.desktop

# Move the required files to the correct directory
mv "$QTOX_APP_DIR"/usr/* "$QTOX_APP_DIR/"
rm -rf "$QTOX_APP_DIR/usr"

## Warning: This is hard coded to debain:stretch.
#libs=(
## copy OpenSSL libs to AppImage
#/usr/lib/x86_64-linux-gnu/libssl.so
#/usr/lib/x86_64-linux-gnu/libcrypt.so
#/usr/lib/x86_64-linux-gnu/libcrypto.so
## Also bundle libjack.so* without which the AppImage does not work in Fedora Workstation
#/usr/lib/x86_64-linux-gnu/libjack.so.0
## And libglib needed by Red Hat and derivatives to work with our old gnutls
#/usr/lib/x86_64-linux-gnu/libglib-2.0.so.0
#/usr/lib/x86_64-linux-gnu/libgobject-2.0.so.0
#/usr/lib/x86_64-linux-gnu/libgio-2.0.so.0
#/usr/lib/x86_64-linux-gnu/libpango-1.0.so.0
#/usr/lib/x86_64-linux-gnu/libpangoft2-1.0.so.0
#/usr/lib/x86_64-linux-gnu/libpangocairo-1.0.so.0
#)

#for lib in "${libs[@]}"; do
#    lib_file_name=$(basename "$lib")
#    cp -P $(echo "$lib"*) "$LOCAL_LIB_DIR"
#    patchelf --set-rpath '$ORIGIN' "$LOCAL_LIB_DIR/$lib_file_name"
#done

#appimagetool -u "$UPDATE_INFO" $QTOX_APP_DIR qTox-${GIT_VERSION}.x86_64.AppImage
#VERSION=${GIT_VERSION} ./appimagetool-*.AppImage --appimage-extract-and-run $QTOX_APP_DIR
