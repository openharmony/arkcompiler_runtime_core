set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

panda_ets_interop_js_gtest(ets_interop_js_operator_new
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/operator_new.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/operator_new.sts
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_operator_new_ets_to_js
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/operator_new_ets_to_js.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/operator_new_ets_to_js.sts
    ETS_CONFIG ${ETS_CONFIG}
)