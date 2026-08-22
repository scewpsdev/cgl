@echo off
D:\Dev\Rainfall\RainfallResourceCompiler\bin\x64\Release\RainfallResourceCompiler.exe res bin/res png vert frag comp
pushd bin
D:\Dev\2023\CGL\bin\x64\Release\snekc run ../src/triangle.src ../lib/libSDL3.dll.a ../lib/SDL3.dll
popd
