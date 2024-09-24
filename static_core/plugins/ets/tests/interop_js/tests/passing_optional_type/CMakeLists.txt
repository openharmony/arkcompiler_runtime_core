set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

panda_ets_interop_js_gtest(ets_interop_js_passing_optional_type
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/passing_optional_type.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/passing_optional_type.sts
    ETS_CONFIG ${ETS_CONFIG}
)