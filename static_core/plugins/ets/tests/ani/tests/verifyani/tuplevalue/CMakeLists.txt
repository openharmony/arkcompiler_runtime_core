# Copyright (c) 2026 Huawei Device Co., Ltd.
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

ani_verify_add_gtest(ani_verify_test_tuplevalue_get_number_of_items
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_tuplevalue_get_number_of_items_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_tuplevalue_get_number_of_items_test.ets
)

ani_verify_add_generated_tests(ani_verify_test_tuplevalue_get_item
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_tuplevalue_get_item_test.cpp.j2
    J2_ETS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_tuplevalue_get_item_test.ets.j2
    PARAMS "boolean: Type=Boolean,type=boolean,expected_cpp_value=ANI_TRUE"
           "char: Type=Char,type=char,expected_cpp_value='x'"
           "byte: Type=Byte,type=byte,expected_cpp_value=1"
           "short: Type=Short,type=short,expected_cpp_value=10"
           "int: Type=Int,type=int,expected_cpp_value=100"
           "long: Type=Long,type=long,expected_cpp_value=1000"
           "float: Type=Float,type=float,expected_cpp_value=1.25F"
           "double: Type=Double,type=double,expected_cpp_value=1.25"
           "ref: Type=Ref,type=ref,expected_cpp_value=nullptr"
)

ani_verify_add_generated_tests(ani_verify_test_tuplevalue_set_item
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_tuplevalue_set_item_test.cpp.j2
    J2_ETS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_tuplevalue_set_item_test.ets.j2
    PARAMS "boolean: Type=Boolean,type=boolean,valid_cpp_value=ANI_TRUE,updated_cpp_value=ANI_FALSE"
           "char: Type=Char,type=char,valid_cpp_value='x',updated_cpp_value='q'"
           "byte: Type=Byte,type=byte,valid_cpp_value=1,updated_cpp_value=9"
           "short: Type=Short,type=short,valid_cpp_value=10,updated_cpp_value=90"
           "int: Type=Int,type=int,valid_cpp_value=100,updated_cpp_value=900"
           "long: Type=Long,type=long,valid_cpp_value=1000,updated_cpp_value=9000"
           "float: Type=Float,type=float,valid_cpp_value=1.25F,updated_cpp_value=9.75F"
           "double: Type=Double,type=double,valid_cpp_value=1.25,updated_cpp_value=9.75"
           "ref: Type=Ref,type=ref,valid_cpp_value=nullptr,updated_cpp_value=nullptr"
)
