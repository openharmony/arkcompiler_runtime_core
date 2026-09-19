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

cmake_minimum_required(VERSION 3.10)

project(ani_coldreload_app)

panda_add_executable(ani_coldreload_app main.cpp)

panda_target_link_libraries(ani_coldreload_app arkruntime)

panda_add_sanitizers(TARGET ani_coldreload_app SANITIZERS ${PANDA_SANITIZERS_LIST})
