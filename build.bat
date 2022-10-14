@echo off

rem echo "Starting wasm build"
rem clang -g --target=wasm32 --no-standard-libraries -Wl,--import-memory -Wl,--no-entry -Wl,--export-all -o game.wasm wasm_game.cc
rem echo "Wasm build done"

echo "Starting windows build"
mkdir ..\motorwaysbuild
pushd ..\motorwaysbuild
cl -Zi ..\motorways\win32_game.cc user32.lib Gdi32.lib
popd
echo "Windows build done"

