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

FROM python:3.10
WORKDIR /playground


COPY ./build/ ./build
COPY ./plugins/ets/stdlib ./stdlib
COPY ./plugins/ets/playground/backend/dist ./dist
COPY ./plugins/ets/playground/backend/docker/config.yaml ./config.yaml
COPY ./plugins/ets/playground/backend/docker/arktsconfig.json ./build/bin/arktsconfig.json

RUN apt-get update -qq && apt-get install -yqq libdwarf-dev libatomic1
RUN python3 -m pip install --no-cache-dir /playground/dist/*.whl
CMD ["arkts-playground", "-c", "/playground/config.yaml"]
