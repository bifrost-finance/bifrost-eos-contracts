add_contract(bifrost.bridge bifrost.bridge ${CMAKE_CURRENT_SOURCE_DIR}/src/bifrost.bridge.cpp)

target_include_directories(bifrost.bridge
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(bifrost.bridge
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
