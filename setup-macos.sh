#!/usr/bin/env bash
# Installs everything needed to build Mellangan on an Apple Silicon Mac: cmake, ninja and the Vulkan stack
# (headers, loader, MoltenVK, validation layers, vulkaninfo) from Homebrew, plus pinned, checksummed slangc and
# MoltenVK releases in build-deps/; the app bundle embeds that MoltenVK. Safe to re-run. Configure afterwards with the command it prints.
set -euo pipefail
cd "$(dirname "$0")"

[[ $(uname -s) == Darwin && $(uname -m) == arm64 ]] || { echo "setup-macos.sh targets Apple Silicon macOS" >&2; exit 1; }
xcode-select -p > /dev/null 2>&1 || { echo "install the Xcode command line tools first: xcode-select --install" >&2; exit 1; }
command -v brew > /dev/null || { echo "install Homebrew first: https://brew.sh" >&2; exit 1; }

brew install cmake ninja vulkan-headers vulkan-loader molten-vk vulkan-validationlayers vulkan-tools

layers=$HOME/.config/vulkan/explicit_layer.d
mkdir -p "$layers"
sed -E "s|\"library_path\": *\"[^\"]*\"|\"library_path\": \"$(brew --prefix)/lib/libVkLayer_khronos_validation.dylib\"|" \
  "$(brew --prefix)/share/vulkan/explicit_layer.d/VkLayer_khronos_validation.json" > "$layers/VkLayer_khronos_validation.json"

slang=2026.19
sum=fa46822c92d81404a7951de3746093985c1e6f520f4b533e2aa3bea576f0f2f0
dir=build-deps/slang-$slang
if [[ ! -x $dir/bin/slangc ]]; then
  tgz=build-deps/slang-$slang.tar.gz
  mkdir -p build-deps
  curl -fsSL --retry 3 -o "$tgz" "https://github.com/shader-slang/slang/releases/download/v$slang/slang-$slang-macos-aarch64.tar.gz"
  echo "$sum  $tgz" | shasum -a 256 -c -
  rm -rf "$dir" && mkdir -p "$dir"
  tar -xzf "$tgz" -C "$dir"
  rm "$tgz"
fi
"$dir/bin/slangc" -v

mvk=1.4.2
sum=f95765a6229cb7b915990a2890ce12ebe36a730b021545d3d52ae69ce4c4024e
dir=build-deps/moltenvk-$mvk
if [[ ! -f $dir/libMoltenVK.dylib ]]; then
  tar=build-deps/MoltenVK-macos-$mvk.tar
  curl -fsSL --retry 3 -o "$tar" "https://github.com/KhronosGroup/MoltenVK/releases/download/v$mvk/MoltenVK-macos.tar"
  echo "$sum  $tar" | shasum -a 256 -c -
  rm -rf "$dir" && mkdir -p "$dir"
  tar -xf "$tar" -C "$dir" --strip-components 5 MoltenVK/MoltenVK/dynamic/dylib/macOS
  rm "$tar"
fi
lipo -info "$dir/libMoltenVK.dylib"

cat << EOF

Ready. Build with:
  cmake -S . -B build-macos -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_OSX_ARCHITECTURES=arm64
  cmake --build build-macos
  open build-macos/bin/Mellangan.app
EOF
