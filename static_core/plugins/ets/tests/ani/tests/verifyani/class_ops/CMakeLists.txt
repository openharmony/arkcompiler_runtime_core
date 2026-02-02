# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

ani_verify_add_gtest(ani_verify_test_class_call_static_method_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_class_call_static_method_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_class_call_static_method_boolean_test.ets
)

# Generated tests for Class_GetStaticField APIs
ani_verify_add_generated_tests(ani_verify_test_class_static_field_get
    ETS_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.json
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_class_static_field_get_test.cpp.j2
    J2_ETS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_class_static_field_get_test.ets.j2
    PARAMS "boolean: Type=Boolean,type=boolean,field=booleanField,wrong_field=staticIntField,ets_type=boolean,wrong_ets_type=int"
           "byte: Type=Byte,type=byte,field=byteField,wrong_field=staticIntField,ets_type=byte,wrong_ets_type=int"
           "char: Type=Char,type=char,field=charField,wrong_field=staticIntField,ets_type=char,wrong_ets_type=int"
           "short: Type=Short,type=short,field=shortField,wrong_field=staticIntField,ets_type=short,wrong_ets_type=int"
           "int: Type=Int,type=int,field=intField,wrong_field=booleanField,ets_type=int,wrong_ets_type=boolean"
           "long: Type=Long,type=long,field=longField,wrong_field=staticIntField,ets_type=long,wrong_ets_type=int"
           "float: Type=Float,type=float,field=floatField,wrong_field=staticIntField,ets_type=float,wrong_ets_type=int"
           "double: Type=Double,type=double,field=doubleField,wrong_field=staticIntField,ets_type=double,wrong_ets_type=int"
           "ref: Type=Ref,type=ref,field=refField,wrong_field=staticIntField,ets_type=Object,wrong_ets_type=int"
)

# Generated tests for Class_SetStaticField APIs
ani_verify_add_generated_tests(ani_verify_test_class_static_field_set
    ETS_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.json
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_class_static_field_set_test.cpp.j2
    J2_ETS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_class_static_field_set_test.ets.j2
    PARAMS "boolean: Type=Boolean,type=boolean,field=booleanField,wrong_field=staticIntField,ets_type=boolean,wrong_ets_type=int,test_value=ANI_TRUE"
           "byte: Type=Byte,type=byte,field=byteField,wrong_field=staticIntField,ets_type=byte,wrong_ets_type=int,test_value=1"
           "char: Type=Char,type=char,field=charField,wrong_field=staticIntField,ets_type=char,wrong_ets_type=int,test_value=97"
           "short: Type=Short,type=short,field=shortField,wrong_field=staticIntField,ets_type=short,wrong_ets_type=int,test_value=1"
           "int: Type=Int,type=int,field=intField,wrong_field=booleanField,ets_type=int,wrong_ets_type=boolean,test_value=1234"
           "long: Type=Long,type=long,field=longField,wrong_field=staticIntField,ets_type=long,wrong_ets_type=int,test_value=1"
           "float: Type=Float,type=float,field=floatField,wrong_field=staticIntField,ets_type=float,wrong_ets_type=int,test_value=1.0"
           "double: Type=Double,type=double,field=doubleField,wrong_field=staticIntField,ets_type=double,wrong_ets_type=int,test_value=1.0"
           "ref: Type=Ref,type=ref,field=refField,wrong_field=staticIntField,ets_type=Object,wrong_ets_type=int"
)