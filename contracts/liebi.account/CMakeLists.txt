add_contract(liebi.account liebi.account ${CMAKE_CURRENT_SOURCE_DIR}/src/account.cpp)

target_include_directories(liebi.account
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(liebi.account
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
