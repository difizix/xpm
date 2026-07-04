cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(pnextract_project)

add_executable(pnextract # WIN32
  "src/pnm/pnextract/nextract.cpp"  
  "src/pnm/pnextract/blockNet.cpp"  
  "src/pnm/pnextract/medialSurf.cpp"  
  "src/pnm/pnextract/writers_vxl.cpp"  
  "src/pnm/pnextract/writers_vtk.cpp"  
  "src/libvoxel/voxelImage.cpp"    
)

target_compile_features(pnextract PRIVATE cxx_std_17)

find_package(ZLIB REQUIRED)
target_link_libraries(pnextract PRIVATE ZLIB::ZLIB)
target_compile_definitions(pnextract PRIVATE ZLIB)

target_include_directories(pnextract PRIVATE "src/include")
target_include_directories(pnextract PRIVATE "src/libvoxel")
set_target_properties(pnextract PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

install(TARGETS pnextract RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")