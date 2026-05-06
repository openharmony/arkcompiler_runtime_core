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

ani_verify_add_generated_tests(ani_verify_test_primitive_box
    ETS_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.json
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_primitive_box_test.cpp.j2
    J2_ETS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_primitive_box_test.ets.j2
    PARAMS
        "boolean: type=boolean,Type=Boolean,shorty=z,test_value=ANI_TRUE,ets_value=true"
        "byte: type=byte,Type=Byte,shorty=b,test_value=12,ets_value=12,min_value=INT8_MIN,max_value=INT8_MAX,zero_value=0"
        "char: type=char,Type=Char,shorty=c,test_value='A',ets_value=c'z',min_value=0,max_value=UINT16_MAX,zero_value=0"
        "short: type=short,Type=Short,shorty=s,test_value=42,ets_value=42,min_value=INT16_MIN,max_value=INT16_MAX,zero_value=0"
        "int: type=int,Type=Int,shorty=i,test_value=42,ets_value=42,min_value=INT32_MIN,max_value=INT32_MAX,zero_value=0"
        "long: type=long,Type=Long,shorty=l,test_value=42,ets_value=42,min_value=INT64_MIN,max_value=INT64_MAX,zero_value=0"
        "float: type=float,Type=Float,shorty=f,test_value=1.25F,ets_value=1.25f,min_value=-FLT_MAX,max_value=FLT_MAX,zero_value=0.0F"
        "double: type=double,Type=Double,shorty=d,test_value=1.25,ets_value=1.25,min_value=-DBL_MAX,max_value=DBL_MAX,zero_value=0.0"
)

ani_verify_add_generated_tests(ani_verify_test_primitive_unbox
    ETS_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.json
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_primitive_unbox_test.cpp.j2
    J2_ETS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_primitive_unbox_test.ets.j2
    PARAMS
        "boolean: type=boolean,Type=Boolean,shorty=z,test_value=ANI_TRUE,ets_value=true"
        "byte: type=byte,Type=Byte,shorty=b,test_value=12,ets_value=12,min_value=INT8_MIN,max_value=INT8_MAX,zero_value=0"
        "char: type=char,Type=Char,shorty=c,test_value='A',ets_value=c'z',min_value=0,max_value=UINT16_MAX,zero_value=0"
        "short: type=short,Type=Short,shorty=s,test_value=42,ets_value=42,min_value=INT16_MIN,max_value=INT16_MAX,zero_value=0"
        "int: type=int,Type=Int,shorty=i,test_value=42,ets_value=42,min_value=INT32_MIN,max_value=INT32_MAX,zero_value=0"
        "long: type=long,Type=Long,shorty=l,test_value=42,ets_value=42,min_value=INT64_MIN,max_value=INT64_MAX,zero_value=0"
        "float: type=float,Type=Float,shorty=f,test_value=1.25F,ets_value=1.25f,min_value=-FLT_MAX,max_value=FLT_MAX,zero_value=0.0F"
        "double: type=double,Type=Double,shorty=d,test_value=1.25,ets_value=1.25,min_value=-DBL_MAX,max_value=DBL_MAX,zero_value=0.0"
)
