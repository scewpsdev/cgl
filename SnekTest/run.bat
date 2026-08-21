@echo off
D:\Dev\Rainfall\RainfallResourceCompiler\bin\x64\Release\RainfallResourceCompiler.exe res bin/res png vert frag comp
snekc run src/triangle.src lib/libSDL3.dll.a lib/SDL3.dll
