# Build

Build with CMake.

Inside source folder:

```bash
mkdir build && cd build
cmake -A Win32 ..
cmake --build . --config Release
```

Inside `Release` folder you could find `showdown_hack.dll`

# How to use

Simply inject built dll into `showdown.exe` process. In game toggle aimbot by pressing `F5`

# Developer details

This dll hooks Lua C API functions: `lua_pcall` and `lua_loadbuffer`.

By modifying `HookedLuaLoadBuffer` hook you could dump all lua scripts which game is loading. Scripts are JIT-compiled, so you have to decompile them with any lua JIT decompiler.

`HookedLuaPcall` hook contains code which injects custom lua code into the game. Lua code is contained in the `lua_script.hpp`.
It's simply looks for players in game state machine and returns coordinates of them.
This lua table is placed on top of the lua stack. After that table is being parsed and cursor is placed on top of a character model.
