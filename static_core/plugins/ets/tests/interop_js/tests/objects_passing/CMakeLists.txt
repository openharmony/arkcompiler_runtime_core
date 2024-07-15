set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

panda_ets_interop_js_gtest(ets_interop_js_objects_passing
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/objects_passing.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/objects_passing.sts
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_objects_passing_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/objects_passing_sts.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/objects_passing_sts.sts
    ETS_CONFIG ${ETS_CONFIG}
)