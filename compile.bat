rem C:\Users\Namek\.babun\cygwin\home\Namek\clones\emsdk\emsdk_env.bat

rem -s DISABLE_EXCEPTION_CATCHING=1 

em++ src/wasm/game.cpp  -o build/game.wasm -Os -s WASM=1 -s SIDE_MODULE=1 -s ONLY_MY_CODE=1 -s "EXPORTED_FUNCTIONS=['_preinit','_generateTextures','_init','_render']" -s TOTAL_MEMORY=16MB  & C:\Users\Namek\Downloads\Temp\binaryen-version_89-x86_64-windows\wasm-opt.exe -O4 build/game.wasm -o build/game-opt.wasm