cmake_minimum_required(VERSION 3.10)
project(systemc_mpi)

find_package(MPI REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/systemc-2.3.3/include ${MPI_INCLUDE_PATH})
#add_definitions(-DSC_DEFAULT_WRITER_POLICY)

add_executable(main main.cpp)

target_link_libraries(main 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/systemc-2.3.3/lib-linux64/libsystemc-2.3.3.so
    ${MPI_LIBRARIES}
    )

if(MPI_COMPILE_FLAGS)
    set_target_properties(main PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
    set_target_properties(main PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()

