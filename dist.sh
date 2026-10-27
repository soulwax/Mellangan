#!/usr/bin/env bash
# Builds a versioned production release of Mellangan on Linux or macOS and packs it into
# dist/Mellangan-<version>-<platform>.tar.gz, plus a .sha256 next to it. The bash twin of dist.ps1.
#
#   ./dist.sh          patch bump: 0.2.6 -> 0.2.7
#   ./dist.sh minor    0.2.7 -> 0.3.0
#   ./dist.sh none     rebuild the current version, changelog untouched
#
# On macOS the Vulkan loader and MoltenVK are copied from $VULKAN_SDK into lib/, and Mellangan.command starts the
# game with the bundled driver, so players need nothing installed.
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
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
trap - ERR

# --- package -----------------------------------------------------------------------------------------------------
name=Mellangan-$version-$platform
stage=dist/$name
archive=dist/$name.tar.gz
rm -rf "$stage" "$archive" "$archive.sha256"
mkdir -p "$stage/res/shaders"

bin=build
[[ -f build/Release/Mellangan ]] && bin=build/Release
cp "$bin/Mellangan" CHANGELOG.md "$stage/"
cp res/atlas.png res/particles.png res/font.png res/CREDITS.md "$stage/res/"
cp "$bin"/res/shaders/*.spv "$stage/res/shaders/"
cp -R res/sfx res/music "$stage/res/"

if [[ $platform == macos ]]; then
  sdk=${VULKAN_SDK:?set VULKAN_SDK to the SDK macOS folder, e.g. by sourcing its setup-env.sh}
  mkdir -p "$stage/lib"
  cp -L "$sdk/lib/libvulkan.1.dylib" "$sdk/lib/libMoltenVK.dylib" "$stage/lib/"
  sed -E 's|"library_path": *"[^"]*"|"library_path": "./libMoltenVK.dylib"|' \
    "$sdk/share/vulkan/icd.d/MoltenVK_icd.json" > "$stage/lib/MoltenVK_icd.json"
  install_name_tool -add_rpath @executable_path/lib "$stage/Mellangan"
  codesign --force --sign - "$stage/Mellangan"
  cat > "$stage/Mellangan.command" << 'EOF'
#!/bin/sh
cd "$(dirname "$0")"
export VK_DRIVER_FILES="$PWD/lib/MoltenVK_icd.json" VK_ICD_FILENAMES="$PWD/lib/MoltenVK_icd.json"
exec ./Mellangan "$@"
EOF
  chmod +x "$stage/Mellangan.command"
fi

tar -C dist -czf "$archive" "$name"
if command -v sha256sum > /dev/null; then
  (cd dist && sha256sum "$name.tar.gz" > "$name.tar.gz.sha256")
else
  (cd dist && shasum -a 256 "$name.tar.gz" > "$name.tar.gz.sha256")
fi
echo "$(pwd)/$archive ($(du -h "$archive" | cut -f1))"
cat "$archive.sha256"
