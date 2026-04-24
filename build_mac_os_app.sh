#!/bin/sh

# Pekka Kana 2 by Janne Kivilahti (2003–2007) and the Piste Gamez community.
# https://pistegamez.net/game_pk2.html
#
#
# This script builds and packages Pekka Kana 2 as a macOS application
# that can be run without a terminal.
#
# You may need to install dylibbundler:
#   brew install dylibbundler


CERTIFICATE_NAME="SaturninTheAlien" # set yours

APP=Pekka-Kana-2
APP_NAME="Pekka Kana 2"

WRAPPER_NAME="pekka-kana-2"

VER=1.5.2


BINARY_NAME="pk2_greta_${VER}"
MACOS_APP="bin/Pekka Kana 2.app"

APP_ROOT="${MACOS_APP}"/Contents
MACOS_DIR="${APP_ROOT}/MacOS"
RESOURCES_DIR="${APP_ROOT}/Resources"

ASSETS_DIR="${RESOURCES_DIR}/assets"
LIB_DIR="${RESOURCES_DIR}/lib"


BINARY_PATH="${RESOURCES_DIR}/${BINARY_NAME}"

echo "== Build =="
make clean
make -j4

echo "== Create dirs =="

rm -rf "${APP_ROOT}"

mkdir -p "${APP_ROOT}"
mkdir -p "${MACOS_DIR}"
mkdir -p "${RESOURCES_DIR}"
mkdir -p "${ASSETS_DIR}"
mkdir -p "${LIB_DIR}"

echo "== Info.plist =="

cat > "${APP_ROOT}/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>LSApplicationCategoryType</key>
    <string>public.app-category.games</string>

    <key>CFBundleName</key>
    <string>${APP_NAME}</string>

    <key>CFBundleDisplayName</key>
    <string>${APP_NAME}</string>

    <key>CFBundleExecutable</key>
    <string>${WRAPPER_NAME}</string>

    <key>CFBundleIdentifier</key>
    <string>net.pistegamez.pk2</string>

    <key>CFBundleVersion</key>
    <string>${VER}</string>

    <key>CFBundleShortVersionString</key>
    <string>${VER}</string>

    <key>CFBundlePackageType</key>
    <string>APPL</string>

    <key>CFBundleIconFile</key>
    <string>icon</string>

    <key>LSMinimumSystemVersion</key>
    <string>11.0</string>

</dict>
</plist>
EOF

echo "== Wrapper =="

WRAPPER_PATH="${MACOS_DIR}/${WRAPPER_NAME}"
cat <<'EOF' | sed "s/@BINARY@/${BINARY_NAME}/g" > "${WRAPPER_PATH}"
#!/bin/sh

DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$DIR/.." && pwd)"

RES="$ROOT/Resources"
ASSETS="$RES/assets"
LIB="$RES/lib"
BIN="$RES/@BINARY@"

export DYLD_LIBRARY_PATH="$LIB:$DYLD_LIBRARY_PATH"

exec "$BIN" \
    --assets-path "$ASSETS" \
    --data-path PREF_PATH \
    "$@"
EOF

chmod 755 "${WRAPPER_PATH}"

echo "== Binary =="

install -m 755 bin/pekka-kana-2 \
    "${BINARY_PATH}"

echo "== Bundle libs =="

dylibbundler -od -b \
-x "${BINARY_PATH}" \
-d "${LIB_DIR}"
-cd

echo "== Icon =="
cp misc/mac/icon.icns "${RESOURCES_DIR}/icon.icns"

echo "== Assets =="
rsync -a \
  --exclude='data' \
  res/ \
  "${ASSETS_DIR}"

find "${ASSETS_DIR}" -type d -exec chmod 755 {} +
find "${ASSETS_DIR}" -type f -exec chmod 644 {} +

echo "== Sign the APP == "

codesign --deep --force \
--timestamp=none \
--sign "${CERTIFICATE_NAME}" \
"${MACOS_APP}"


echo "== Create DMG =="

DMG_NAME="bin/${APP}_${VER}_MacOS.dmg"
TMP_DIR="bin/dmg_tmp"

rm -rf "$TMP_DIR"
mkdir "$TMP_DIR"

cp -R "${MACOS_APP}" "$TMP_DIR/"
ln -s /Applications "$TMP_DIR/Applications"

hdiutil create -volname "${APP_NAME}" \
  -srcfolder "$TMP_DIR" \
  -ov -format UDZO \
  "${DMG_NAME}"

rm -rf "$TMP_DIR"

echo "== Sign DMG =="

codesign --force \
--sign "${CERTIFICATE_NAME}" \
"${DMG_NAME}"





