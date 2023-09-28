/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bridge/arch/arm/proxy_entrypoint_armhf.S"


PROXY_ENTRYPOINT JSRuntimeJSCallBridge JSRuntimeJSCall
PROXY_ENTRYPOINT JSRuntimeJSCallByValueBridge JSRuntimeJSCallByValue
PROXY_ENTRYPOINT JSRuntimeJSNewBridge JSRuntimeJSNew

PROXY_ENTRYPOINT TSTypeCallStaticBridge TSTypeCallStatic
PROXY_ENTRYPOINT TSTypeCallThisBridge TSTypeCallThis
PROXY_ENTRYPOINT TSTypeCallCtorBridge TSTypeCallCtor

PROXY_ENTRYPOINT JSProxyCallBridge JSProxyCall
