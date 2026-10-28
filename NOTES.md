I've written [dist.sh](dist.sh), the bash twin of [dist.ps1](dist.ps1) for Linux and macOS. It passes a bash syntax check, and its changelog step gives the same output as `dist.ps1`. I couldn't run a real build with it from Windows, so the first true test will be on your Mac.

**Same as `dist.ps1`:**
- `./dist.sh` (patch), `./dist.sh minor`, `./dist.sh major` and `./dist.sh none`.
- It refuses to bump when the Unreleased section is empty, and restores `VERSION` and `CHANGELOG.md` if the build fails.
- It stages the same files and writes `dist/Mellangan-<version>-<linux|macos>.tar.gz`.
- It runs on macOS's built-in bash 3.2 and uses only commands that behave the same on macOS and Linux.

**New:**
- **Checksum:** it writes the `.sha256` next to the archive, so releases don't need that done by hand any more.
- **macOS bundle:** a Mac build links the Vulkan loader from the SDK, which players won't have. So the script:
  - copies `libvulkan.1.dylib` and `libMoltenVK.dylib` from `$VULKAN_SDK` into `lib/`, plus an ICD JSON that points at the bundled MoltenVK;
  - adds an `@executable_path/lib` rpath, then re-signs the binary ad-hoc, which Apple Silicon requires after changing it;
  - adds a `Mellangan.command` launcher that points the loader at the bundled driver. Players can double-click it in Finder.

I also added a short paragraph to the README's packaging section.

**When you're on a Mac:**
1. Make the script executable in git with `git update-index --chmod=+x dist.sh`. Windows can't set the bit.
2. Source the SDK's `setup-env.sh` so `VULKAN_SDK` is set, then run `./dist.sh none`.
3. Check that `otool -L build/Mellangan | grep vulkan` shows `@rpath/libvulkan.1.dylib`. The bundling relies on that, and I couldn't confirm it from here.
4. A downloaded tarball will be quarantined by Gatekeeper. Players need right-click → Open, or `xattr -dr com.apple.quarantine <folder>`. Only signing and notarizing would avoid that.


## Test class

Currently it is not perfect and only tests the pool allocator and level lifecycle. It is not run automatically, but can be run with `./build/Mellangan --test`. It will log to `mellangan.log` and exit with 0 on success, 1 on failure.