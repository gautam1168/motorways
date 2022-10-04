clang -g --target=wasm32 --no-standard-libraries -Wl,--import-memory -Wl,--no-entry -Wl,--export-all -o game.wasm win32.cc
