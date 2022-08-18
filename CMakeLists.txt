# CMakeList.txt : CMake project for testprojVB, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("nightsImageTool")

# Add source to this project's executable.
add_executable (nightsImageTool "main.cpp" "ImgSpec.cpp" "SvrTest.cpp")

# TODO: Add tests and install targets if needed.
