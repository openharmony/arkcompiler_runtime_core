# Copyright (c) 2023 Huawei Device Co., Ltd.
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

if (NOT PANDA_LLVM_IRTOC)
    message(FATAL_ERROR "LLVMInlineModules must be included before Definitions.cmake")
endif ()

message(STATUS "LLVM IR inlining for IRTOC temporarily disabled")
set(PANDA_LLVM_INTERPRETER_INLINING false)
