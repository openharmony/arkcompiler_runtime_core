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

if (PANDA_TARGET_ARM32)
    return()
endif()

run_int_ets_code(${CMAKE_CURRENT_SOURCE_DIR}/a.sts ${CMAKE_CURRENT_BINARY_DIR} imported_modules_TL_exec_1A EXPORT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/b.sts;${CMAKE_CURRENT_SOURCE_DIR}/c.sts
)