cmake --build build/ --config Debug
cmd /c "cd build && ctest -C Debug --verbose"
