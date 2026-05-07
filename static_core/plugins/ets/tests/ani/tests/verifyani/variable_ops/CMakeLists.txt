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

ani_verify_add_generated_tests(ani_verify_test_variable_set_value
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_variable_set_value_test.cpp.j2
    J2_ETS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_variable_set_value_test.ets.j2
    PARAMS "boolean: Type=Boolean,type=boolean,test_value=ANI_TRUE,alt_value=ANI_FALSE,const_init=true"
           "char: Type=Char,type=char,test_value=97,alt_value=122,const_init=c'c'"
           "byte: Type=Byte,type=byte,test_value=18,alt_value=19,const_init=5"
           "short: Type=Short,type=short,test_value=4660,alt_value=4661,const_init=6"
           "int: Type=Int,type=int,test_value=1193046,alt_value=1193047,const_init=7"
           "long: Type=Long,type=long,test_value=305419896,alt_value=305419897,const_init=8"
           "float: Type=Float,type=float,test_value=1.25F,alt_value=2.5f,const_init=3.0f"
           "double: Type=Double,type=double,test_value=2.5,alt_value=3.5,const_init=4.0"
           "ref: Type=Ref,type=ref,test_value='verify',alt_value='updated',const_init='locked'"
)

ani_verify_add_generated_tests(ani_verify_test_variable_get_value
    J2_CPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_variable_get_value_test.cpp.j2
    J2_ETS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/verify_variable_get_value_test.ets.j2
    PARAMS "boolean: Type=Boolean,type=boolean,ets_init=true,expected_cpp=ANI_TRUE"
           "char: Type=Char,type=char,ets_init=c'a',expected_cpp=97"
           "byte: Type=Byte,type=byte,ets_init=18,expected_cpp=18"
           "short: Type=Short,type=short,ets_init=4660,expected_cpp=4660"
           "int: Type=Int,type=int,ets_init=1193046,expected_cpp=1193046"
           "long: Type=Long,type=long,ets_init=305419896,expected_cpp=305419896"
           "float: Type=Float,type=float,ets_init=1.25f,expected_cpp=1.25F"
           "double: Type=Double,type=double,ets_init=2.5,expected_cpp=2.5"
           "ref: Type=Ref,type=ref,ets_init='verify'"
)
