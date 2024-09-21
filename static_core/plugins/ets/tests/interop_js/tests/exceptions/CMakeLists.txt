set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

panda_ets_interop_js_gtest(ets_interop_js_dynamic_function_exception
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/dynamic_function_exception.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/dynamic_function_exception.sts
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_static_function_exception
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/static_function_exception.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/static_function_exception.sts
    ETS_CONFIG ${ETS_CONFIG}
)