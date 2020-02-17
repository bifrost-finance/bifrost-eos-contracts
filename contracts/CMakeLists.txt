cmake_minimum_required( VERSION 3.5 )

project(contracts)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_subdirectory(bifrost.token)
add_subdirectory(bifrost.bridge)
add_subdirectory(liebi.account)
