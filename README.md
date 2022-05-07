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
- `Alt+Tab` while FullScreen should minimize screen (remember to take into account the fact that people might have multiple monitors, so clicking outside of the fullscreen window in a different monitor shouldn't minimize it, but pressing `Alt+Tab` while fullscreen window is in focus will minimize it)
- When resizing the window super fast, there appears to be a white area on the new part of the window. (get rid of that)
- See if the dxc compiler produces higher quality shader asm than fxc
- while resizing take into account passage of time during redraws
- Use EnumAdapterByGpuPreference to get the adapter
- Take into account different DirectX12/DXGI Versions better
- While fullscreen use a better sync interval (v-sync off possible?)
- There seems to be ghosting while in fullscreen, need to fix that.
- have code entry point at WinMainCRTStartup instead maybe?
- look into shader optimization with https://github.com/jbarczak/Pyramid and https://renderdoc.org/
