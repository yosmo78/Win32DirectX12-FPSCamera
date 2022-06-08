# Win32DirectX12-FPSCamera
Basic FPS camera starter code for compiling an Win32/DirectX12 Shader environment, just installing visual studio


Steps to compile and run:
1) Install visual studio 2019
2) Clone repo
3) Open command prompt in repo
4) run: `.\Compile.bat`
5) run: `.\FPSCameraBasic.exe`

To Debug:
1) run: `.\Compile.bat`
2) run: `devenv .\FPSCameraBasicDebug.exe`
3) When Visual Studio is running, press `F11`

Controls
- `WSAD` or `ArrowKeys` to move
- `Space` and `shift` to move up and down
- `Mouse` to look around
- `Esc` to pause/unpause
- `F` to go in and out of fullscreen
- `V` to turn off and on V-Sync in windowed mode

Improvements to make:
- `Alt+Tab` while FullScreen should hide screen/make it non visible (minimizing screen has an animation which I don't recommend) (remember to take into account the fact that people might have multiple monitors, so clicking outside of the fullscreen window in a different monitor shouldn't minimize it, but pressing `Alt+Tab` while fullscreen window is in focus will minimize it) (look into fullscreen settings in here https://developer.nvidia.com/dx12-dos-and-donts and https://stackoverflow.com/questions/72156247/alt-tab-in-fullscreen-with-setfullscreenstate-directx12-does-not-minimize-window , maybe true fullscreen but a borderless window is better, have to test performance)
- When resizing the window super fast, there appears to be a white area on the new part of the window. (get rid of that)
- See if the dxc compiler produces higher quality shader asm than fxc
- while resizing take into account passage of time during redraws
- Use EnumAdapterByGpuPreference to get the adapter
- Take into account different DirectX12/DXGI Versions better
- While fullscreen use a better sync interval (v-sync off possible?)
- There seems to be ghosting while in fullscreen, need to fix that.
- have code entry point at WinMainCRTStartup instead for better startup speed for Release build for better start up time?
- look into shader optimization with https://github.com/jbarczak/Pyramid and https://renderdoc.org/
- Should upload all the models into one preallocated GPU heap using CreatePlacedResource in the advanced model. (upload in batches)
- Distribute dll's for directx12 if they have the ability to run it, but don't have it installed
- What should the D3D_FEATURE_LEVEL_12_0 be? or D3D_FEATURE_LEVEL_11_0?
- Virtual Alloc for memory allocation (taking into account RAM and VRAM limits) and every system reports its projected memory usage over the life time of the program (have a memory counter for always around persitent memory and per scene memory, can add them up to get total projected memory usage)
- Shader Hot reloading in debug build using .cso files in debug build with a hotkey
- Take into account different DPI
- Handle Device Removal and SwapChain/Resource loss https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-getdeviceremovedreason (this but in D3D12 https://docs.microsoft.com/en-us/previous-versions/windows/apps/dn458383(v=win.10)?redirectedfrom=MSDN)
- Bundle Command List commands
- Look into https://developer.nvidia.com/blog/advanced-api-performance-memory-and-resources/ and  Mesh Shaders, Sampler Feedback, Direct Storage (directx11 and directx12 features)
- Have MSAA option from the start for the engine (make it toggeable), too hard to implement years down the line
- If doing raytracing look into doing RTX combined with ReStir (unless another major breakthrough occurs before then)
