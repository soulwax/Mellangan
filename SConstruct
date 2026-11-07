import glob
import os
import re
import shutil
import subprocess
import sys

EnsureSConsVersion(4, 11)
SetOption('num_jobs', os.cpu_count() or 4)
SConsignFile('build-scons/.sconsign')

MODES = ('debug', 'release')
mode = ARGUMENTS.get('mode', 'debug')
if mode not in MODES:
    print(f'mode must be one of {", ".join(MODES)}')
    Exit(1)
if sys.platform != 'win32':
    print('The SCons build is the Windows / Visual Studio workflow; use CMake on other platforms.')
    Exit(1)

ROOT = Dir('#').abspath
OUT = f'build-scons/{mode}'
BIN = f'{OUT}/bin'
GEN = 'build-scons/generated'


def natural(path):
    return [int(p) if p.isdigit() else p for p in re.split(r'(\d+)', path)]


sdks = sorted(glob.glob('C:/VulkanSDK/*'), key=natural)
VULKAN_SDK = os.environ.get('VULKAN_SDK') or (sdks[-1] if sdks else None)
if not VULKAN_SDK:
    print('Vulkan SDK not found; install it or set VULKAN_SDK.')
    Exit(1)
SLANGC = os.path.join(VULKAN_SDK, 'Bin', 'slangc.exe')

CMAKE = shutil.which('cmake')
if not CMAKE:
    print('CMake is needed to build SDL3.')
    Exit(1)
VK2D = 'extern/Vulkan2D'
SDL_SRC = f'{VK2D}/extern/SDL'
SDL_BUILD = f'vendor/build/SDL3-{mode}'
SDL = f'vendor/SDL3/{mode.capitalize()}'
sdl_rev = subprocess.run(['git', '-C', SDL_SRC, 'rev-parse', 'HEAD'], capture_output=True, text=True).stdout.strip()


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    if not os.path.exists(path) or open(path, encoding='utf-8').read() != text:
        with open(path, 'w', encoding='utf-8') as f:
            f.write(text)


version = open('VERSION', encoding='utf-8').read().strip()
major, minor, patch = (re.findall(r'\d+', version) + ['0', '0', '0'])[:3]
write(f'{GEN}/Version.hpp', f'#pragma once\n\n#define MELLANGAN_VERSION "{version}"\n')
write(f'{GEN}/pch.cpp', '#include "pch.hpp"\n')
rc = open('res/Mellangan.rc.in', encoding='utf-8').read()
for key, value in {'CMAKE_CURRENT_SOURCE_DIR': ROOT.replace('\\', '/'), 'MELLANGAN_VERSION': version,
                   'PROJECT_VERSION_MAJOR': major, 'PROJECT_VERSION_MINOR': minor,
                   'PROJECT_VERSION_PATCH': patch}.items():
    rc = rc.replace(f'@{key}@', value)
write(f'{GEN}/Mellangan.rc', rc)

debug = mode == 'debug'
base = Environment(TARGET_ARCH='x86_64', tools=['default', 'msvs'])
base.Append(
    CFLAGS=['/std:c11'],
    CXXFLAGS=['/std:c++20', '/EHsc'],
    CCFLAGS=['/Z7', '/FS'] + (['/Od', '/Ob0', '/MTd', '/RTC1'] if debug else ['/O2', '/Ob2', '/MT']),
    CPPDEFINES=['_DEBUG'] if debug else ['NDEBUG'],
    LINKFLAGS=['/DEBUG'] + (['/SUBSYSTEM:CONSOLE'] if debug else ['/OPT:REF', '/OPT:ICF', '/SUBSYSTEM:WINDOWS']),
)
def build_sdl(target, source, env):
    config = mode.capitalize()
    steps = [
        [CMAKE, '-S', SDL_SRC, '-B', SDL_BUILD, '-G', 'Visual Studio 18 2026', '-A', 'x64', '-DSDL_SHARED=OFF',
         '-DSDL_STATIC=ON', '-DSDL_TEST_LIBRARY=OFF', '-DSDL_TESTS=OFF', '-DSDL_EXAMPLES=OFF',
         '-DCMAKE_POLICY_DEFAULT_CMP0091=NEW',
         f'-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded{"Debug" if debug else ""}'],
        [CMAKE, '--build', SDL_BUILD, '--config', config, '--parallel'],
        [CMAKE, '--install', SDL_BUILD, '--config', config, '--prefix', SDL],
    ]
    for step in steps:
        if subprocess.run(step, cwd=ROOT, env=os.environ).returncode:
            return 1
    return 0


sdl = base.Command([f'{SDL}/lib/SDL3-static.lib', f'{SDL}/include/SDL3/SDL.h'], base.Value(sdl_rev),
                   Action(build_sdl, f'Building SDL3 {mode} from {SDL_SRC}'))
base.VariantDir(f'{OUT}/obj/src', 'src', duplicate=0)
base.VariantDir(f'{OUT}/obj/extern', 'extern', duplicate=0)
base.VariantDir(f'{OUT}/obj/gen', GEN, duplicate=0)

VULKAN_INC = os.path.join(VULKAN_SDK, 'Include')
SPDLOG_DEFS = ['SPDLOG_COMPILED_LIB']

dep = base.Clone()
dep.Append(CCFLAGS=['/w'])

spdlog = dep.Clone()
spdlog.Append(CPPPATH=['extern/spdlog/include'], CPPDEFINES=SPDLOG_DEFS, CXXFLAGS=['/utf-8'])
spdlog_lib = spdlog.StaticLibrary(f'{OUT}/lib/spdlog',
                                  [f'{OUT}/obj/{p}' for p in sorted(glob.glob('extern/spdlog/src/*.cpp'))])

vk2d = dep.Clone()
vk2d.Append(CPPPATH=[f'{VK2D}/VK2D/include', f'{SDL}/include', f'{VK2D}/extern/VulkanMemoryAllocator/include',
                     VULKAN_INC],
            CPPDEFINES=['DISABLE_SHADER_COMPILATION'])
vk2d_sources = [f'{VK2D}/VK2D/src/{n}' for n in (
    'Buffer.c', 'Camera.c', 'Constants.c', 'DescriptorBuffer.c', 'DescriptorControl.c', 'Gui.c', 'Image.c',
    'Initializers.c', 'Logger.c', 'LogicalDevice.c', 'Model.c', 'nuklear.c', 'PhysicalDevice.c', 'Pipeline.c',
    'Polygon.c', 'Renderer.c', 'RendererMeta.c', 'Shader.c', 'ShadowEnvironment.c', 'Texture.c', 'Util.c',
    'Validation.c', 'VulkanInterface.c', 'ShaderCompiler.cpp')] + [
    f'{VK2D}/extern/VulkanMemoryAllocator/src/VmaUsage.cpp']
vk2d_objects = [vk2d.Object(f'{OUT}/obj/{p}') for p in vk2d_sources]
vk2d.Requires(vk2d_objects, sdl)
vk2d_lib = vk2d.StaticLibrary(f'{OUT}/lib/Vulkan2D', vk2d_objects)

stb_lib = dep.StaticLibrary(f'{OUT}/lib/stb_vorbis', [f'{OUT}/obj/extern/stb/stb_vorbis.c'])
mpeg_lib = dep.StaticLibrary(f'{OUT}/lib/pl_mpeg', [f'{OUT}/obj/extern/pl_mpeg/pl_mpeg.c'])

APP_INCLUDES = ['src', GEN, f'{VK2D}/VK2D/include', f'{SDL}/include', 'extern/entt/src', 'extern/spdlog/include',
                'extern/stb', 'extern/pl_mpeg', VULKAN_INC]
APP_DEFINES = ['NOMINMAX', 'WIN32_LEAN_AND_MEAN', '_CRT_SECURE_NO_WARNINGS'] + SPDLOG_DEFS
app = base.Clone()
app.Append(
    CPPPATH=APP_INCLUDES,
    CPPDEFINES=APP_DEFINES,
    CCFLAGS=['/W4', '/wd4459', '/permissive-', '/utf-8', '/external:anglebrackets', '/external:W0', '/WX'],
    LIBPATH=[os.path.join(VULKAN_SDK, 'Lib')],
    LIBS=[vk2d_lib, spdlog_lib, stb_lib, mpeg_lib, sdl[0], 'vulkan-1', 'dbghelp',
          'kernel32', 'user32', 'gdi32', 'winmm', 'imm32', 'ole32', 'oleaut32', 'version', 'uuid', 'advapi32',
          'setupapi', 'shell32', 'dinput8'],
    LINKFLAGS=['/MANIFEST:EMBED', f'/MANIFESTINPUT:{ROOT}\\res\\Mellangan.manifest'],
)
c_env = app.Clone()
pch = app.PCH(f'{OUT}/obj/gen/pch.pch', f'{OUT}/obj/gen/pch.cpp')
app['PCH'] = pch[0]
app['PCHSTOP'] = 'pch.hpp'
app.Append(CXXFLAGS=['/FIpch.hpp'])
app.Requires(pch, sdl)

cpp_files = sorted(p.replace('\\', '/') for p in glob.glob('src/**/*.cpp', recursive=True))
c_files = sorted(p.replace('\\', '/') for p in glob.glob('src/**/*.c', recursive=True))
objects = [app.Object(f'{OUT}/obj/{p}') for p in cpp_files] + [c_env.Object(f'{OUT}/obj/{p}') for p in c_files]
app.Requires(objects, sdl)
res = app.RES(f'{OUT}/obj/gen/Mellangan.res', f'{OUT}/obj/gen/Mellangan.rc')
exe = app.Program(f'{BIN}/MellanganApp', objects + [res], PDB=f'{BIN}/MellanganApp.pdb')
app.Depends(exe, 'res/Mellangan.manifest')

assets = []
for folder, _, names in os.walk('res'):
    for name in names:
        path = os.path.join(folder, name).replace('\\', '/')
        assets += app.InstallAs(f'{BIN}/{path}', path)
for shader in sorted(glob.glob('res/shaders/*.slang')):
    name = os.path.splitext(os.path.basename(shader))[0]
    assets += app.Command(f'{BIN}/res/shaders/{name}.spv', shader,
                          f'"{SLANGC}" $SOURCE -target spirv -profile glsl_450 -entry PixelShader '
                          '-stage fragment -o $TARGET')

base.VariantDir(f'{OUT}/obj/tools', 'tools', duplicate=0)
viewer_objects = [o for p, o in zip(cpp_files, objects) if p != 'src/main.cpp'] + objects[len(cpp_files):]
viewer_main = app.Object(f'{OUT}/obj/tools/biome_viewer/main.cpp')
app.Requires(viewer_main, sdl)
viewer = app.Program(f'{BIN}/BiomeViewer', viewer_objects + [viewer_main], PDB=f'{BIN}/BiomeViewer.pdb',
                     LINKFLAGS=[f for f in app['LINKFLAGS'] if not f.startswith('/SUBSYSTEM')] + ['/SUBSYSTEM:CONSOLE'])
Alias('viewer', viewer)

game = Alias('mellangan', [exe, assets])
Default(game)
run_tests = Alias('test', game, f'"{BIN}/MellanganApp.exe" --test')
AlwaysBuild(run_tests)

headers = sorted(p.replace('\\', '/') for p in glob.glob('src/**/*.h*', recursive=True))
variants = ['Debug|x64', 'Release|x64']
project = base.MSVSProject(
    target='#Mellangan' + base['MSVSPROJECTSUFFIX'],
    srcs=cpp_files + c_files,
    incs=headers,
    resources=['res/Mellangan.rc.in', 'res/Mellangan.manifest'],
    misc=['SConstruct', 'VERSION'] + sorted(glob.glob('res/shaders/*.slang')),
    buildtarget=['mellangan'] * len(variants),
    runfile=[os.path.join(ROOT, f'build-scons/{m}/bin/MellanganApp.exe') for m in MODES],
    cmdargs=[f'mode={m}' for m in MODES],
    cpppaths=[[p.replace(f'vendor/SDL3/{mode.capitalize()}', f'vendor/SDL3/{m.capitalize()}')
               for p in APP_INCLUDES] for m in MODES],
    cppdefines=[APP_DEFINES + ['_DEBUG'], APP_DEFINES + ['NDEBUG']],
    cppflags=[['/std:c++20']] * len(variants),
    DebugSettings=[{'LocalDebuggerCommandArguments': '--dev', 'LocalDebuggerWorkingDirectory': ROOT}] * len(variants),
    variant=variants,
    auto_build_solution=1,
)
Alias('msvs', project)
