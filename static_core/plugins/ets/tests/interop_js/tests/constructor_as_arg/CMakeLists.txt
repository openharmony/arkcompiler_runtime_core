# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

panda_ets_interop_js_gtest(ets_interop_js_parent_class_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/parent_class/parent_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/parent_class/parent_class.sts
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_child_class_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/child_class/child_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/child_class/child_class.sts
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_anonymous_class_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/anonymous_class/anonymous_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/anonymous_class/anonymous_class.sts
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_IIFE_class_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/IIFE_class/IIFE_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/IIFE_class/IIFE_class.sts
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_method_constructor_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/method_constructor/method_constructor.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/method_constructor/method_constructor.sts
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_function_arg_constructor_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function/function.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function/function.sts
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_constructor_as_arg_error_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/error/error.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/error/error.sts
    ETS_CONFIG ${ETS_CONFIG}
)
