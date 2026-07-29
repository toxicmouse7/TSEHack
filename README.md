# Build

The project uses CMake, vcpkg manifest mode, and the `x86-windows-static` triplet:

```bash
cmake -S . -B build -A Win32 -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x86-windows-static
cmake --build build --config Release
```

The resulting `showdown_hack.dll` is placed in the selected build directory.

# Architecture

```text
DLL entry → Runtime → Lua hooks → Game-state reader → Target assist
                     └→ DX11 hooks → Overlay → CheatState
```

- `src/dll` owns process entry and subsystem lifetime.
- `src/hooks` owns external API lookup and MinHook integration.
- `src/game` converts the Lua-stack result to plain C++ data structures.
- `src/features` contains game-facing behavior and scripts.
- `src/overlay` is a presentation adapter: it can read and change `core::CheatState`, but has no dependency on Lua hooks, game state, or feature implementation.
- `src/platform` contains Win32-specific low-level helpers.

# Controls

Use the overlay checkboxes to toggle target assist and ESP. Press `Insert` to show or hide the overlay.
