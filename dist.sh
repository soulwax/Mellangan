#!/usr/bin/env bash
# Builds a versioned production release of Mellangan on Linux or macOS and packs it into
# dist/Mellangan-<version>-<platform>.tar.gz, plus a .sha256 next to it. The bash twin of dist.ps1.
#
#   ./dist.sh          patch bump: 0.2.6 -> 0.2.7
#   ./dist.sh minor    0.2.7 -> 0.3.0
#   ./dist.sh none     rebuild the current version, changelog untouched
#
# On macOS it packs an ad hoc signed Mellangan.app (arm64, embedded MoltenVK, see setup-macos.sh) into
# dist/Mellangan-<version>-macos-arm64.zip instead, so players need nothing installed.
set -euo pipefail
cd "$(dirname "$0")"

bump=${1:-patch}
case $bump in major | minor | patch | none) ;; *) echo "usage: $0 [major|minor|patch|none]" >&2; exit 2 ;; esac

case $(uname -s) in
Darwin) platform=macos ;;
Linux) platform=linux ;;
*) echo "dist.sh packages Linux and macOS; use dist.ps1 on Windows" >&2; exit 1 ;;
esac

# --- version -----------------------------------------------------------------------------------------------------
current=$(tr -d '[:space:]' < VERSION)
if ! [[ $current =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)$ ]]; then
  echo "VERSION must be MAJOR.MINOR.PATCH, got '$current'" >&2
  exit 1
fi
major=${BASH_REMATCH[1]} minor=${BASH_REMATCH[2]} patch=${BASH_REMATCH[3]}
case $bump in
major) major=$((major + 1)) minor=0 patch=0 ;;
minor) minor=$((minor + 1)) patch=0 ;;
patch) patch=$((patch + 1)) ;;
esac
version=$major.$minor.$patch

backup=$(mktemp -d)
cp VERSION CHANGELOG.md "$backup/"
restore() {
  cp "$backup/VERSION" "$backup/CHANGELOG.md" .
  echo "Build failed; VERSION and CHANGELOG.md restored." >&2
}

if [[ $bump != none ]]; then
  awk '
    { line = $0; sub(/\r$/, "", line) }
    line == "## [Unreleased]" { open = 1; found = 1; next }
    open && line ~ /^## \[/ { open = 0 }
    open && line ~ /[^[:space:]]/ && line !~ /^###/ { notes = 1 }
    END { exit !(found && notes) }
  ' CHANGELOG.md || {
    echo "Nothing under '## [Unreleased]' in CHANGELOG.md. Describe the changes in $version first," \
      "or run './dist.sh none' to rebuild $current as is." >&2
    exit 1
  }
  awk -v head="## [$version] - $(date +%F)" '
    { line = $0; sub(/\r$/, "", line); print line }
    !done && line == "## [Unreleased]" { print ""; print head; done = 1 }
  ' "$backup/CHANGELOG.md" > CHANGELOG.md
  printf '%s\n' "$version" > VERSION
  trap restore ERR
fi
echo "Mellangan $version"

# --- build -------------------------------------------------------------------------------------------------------
if [[ $platform == macos ]]; then
  cmake -S . -B build-macos-dist -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DMELLANGAN_VULKAN_LOADER=OFF
  cmake --build build-macos-dist --parallel
else
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build --config Release --parallel
fi
trap - ERR

if [[ $platform == macos ]]; then
  name=Mellangan-$version-macos-arm64
  stage=dist/$name
  archive=dist/$name.zip
  built=build-macos-dist/bin/Mellangan.app
  app=$stage/Mellangan.app
  rm -rf "$stage" "$archive" "$archive.sha256"
  mkdir -p "$stage"
  cp -R "$built" "$app"
  res=$app/Contents/Resources/res
  rm -rf "$res" && mkdir -p "$res/shaders"
  cp res/atlas.png res/particles.png res/font.png res/balance.ini res/CREDITS.md "$res/"
  cp "$built"/Contents/Resources/res/shaders/*.spv "$res/shaders/"
  cp -R res/sfx res/music "$res/"
  cp CHANGELOG.md "$stage/"

  exe=$app/Contents/MacOS/Mellangan
  mvk=$app/Contents/Frameworks/libMoltenVK.dylib
  lipo "$mvk" -thin arm64 -output "$mvk.arm64" && mv "$mvk.arm64" "$mvk"
  for f in "$exe" "$mvk"; do
    lipo "$f" -verify_arch arm64 || { echo "$f lacks arm64" >&2; exit 1; }
    if otool -L "$f" | tail -n +2 | grep -vE '^\s+(/System/Library/|/usr/lib/|@rpath/libMoltenVK\.dylib)'; then
      echo "$f links a library outside the system and the bundle" >&2
      exit 1
    fi
  done
  codesign --force --sign - "$mvk"
  codesign --force --sign - "$app"
  codesign --verify --deep --strict --verbose=2 "$app"

  (cd dist && ditto -c -k --keepParent "$name" "$name.zip" && shasum -a 256 "$name.zip" > "$name.zip.sha256")
  echo "$(pwd)/$archive ($(du -h "$archive" | cut -f1))"
  cat "$archive.sha256"
  exit 0
fi

# --- package -----------------------------------------------------------------------------------------------------
name=Mellangan-$version-$platform
stage=dist/$name
archive=dist/$name.tar.gz
rm -rf "$stage" "$archive" "$archive.sha256"
mkdir -p "$stage/res/shaders"

bin=build
[[ -f build/Release/Mellangan ]] && bin=build/Release
cp "$bin/Mellangan" CHANGELOG.md "$stage/"
cp res/atlas.png res/particles.png res/font.png res/balance.ini res/CREDITS.md "$stage/res/"
cp "$bin"/res/shaders/*.spv "$stage/res/shaders/"
cp -R res/sfx res/music "$stage/res/"

tar -C dist -czf "$archive" "$name"
if command -v sha256sum > /dev/null; then
  (cd dist && sha256sum "$name.tar.gz" > "$name.tar.gz.sha256")
else
  (cd dist && shasum -a 256 "$name.tar.gz" > "$name.tar.gz.sha256")
fi
echo "$(pwd)/$archive ($(du -h "$archive" | cut -f1))"
cat "$archive.sha256"
