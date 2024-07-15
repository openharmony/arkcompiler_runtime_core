set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

panda_ets_interop_js_gtest(ets_interop_js_strict_equality
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/strict_equality.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/strict_equality.sts
    ETS_CONFIG ${ETS_CONFIG}
)