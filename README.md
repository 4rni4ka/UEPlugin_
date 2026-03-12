# A template of Unreal Engine plugin

## How to build

1. Clone the repository to your machine
2. Find `RunUAT.bat` (for Windows) or `RunUAT.sh` (for Linux/MacOS): `<Unreal engine path>/Engine/Build/BatchFiles`
3. Build the plugin using `RunUAT`: `./RunUAT.sh BuildPlugin -plugin='<path to plugins repo>/FullScreenPass.uplugin' -package='<path to Unreal Engine project>/Plugins/FullScreenPass/'`
