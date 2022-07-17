# Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

set(SCHEDULER_SOURCES
    scheduler/worker_thread.cpp
    scheduler/task.cpp
)

# @HUAWEI(kwx790815): Policy CMP0076 is not set: target_sources() command converts relative paths
# to absolute.  Run "cmake --help-policy CMP0076" for policy details.
# Use the cmake_policy command to set the policy and suppress this warning.
target_sources(arkruntime PRIVATE ${SCHEDULER_SOURCES})

if(PANDA_WITH_TESTS)
    add_subdirectory(tests)
endif()
