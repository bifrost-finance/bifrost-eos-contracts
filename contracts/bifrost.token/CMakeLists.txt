add_contract(bifrost.token bifrost.token ${CMAKE_CURRENT_SOURCE_DIR}/src/bifrost.token.cpp)

target_include_directories(bifrost.token
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(bifrost.token
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/bifrost.token.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/bifrost.token.contracts.md @ONLY )

target_compile_options( bifrost.token PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
