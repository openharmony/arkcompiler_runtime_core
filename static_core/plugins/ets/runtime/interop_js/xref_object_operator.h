/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_XREF_OBJECT_OPERATOR_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_XREF_OBJECT_OPERATOR_H_

#include <string>

#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets::interop::js {

class XRefObjectOperator {
public:
    static XRefObjectOperator FromEtsObject(EtsObject *etsObject);

    EtsObject *AsObject();

    const EtsObject *AsObject() const;

    EtsObject *GetProperty(EtsCoroutine *coro, const std::string &name) const;

    EtsObject *GetProperty(EtsCoroutine *coro, EtsObject *keyObject) const;

    EtsObject *GetProperty(EtsCoroutine *coro, const uint32_t index) const;

    bool SetProperty(EtsCoroutine *coro, const std::string &name, EtsObject *valueObject) const;

    bool SetProperty(EtsCoroutine *coro, EtsObject *keyObject, EtsObject *valueObject) const;

    bool SetProperty(EtsCoroutine *coro, const uint32_t index, EtsObject *valueObject) const;

    bool IsInstanceOf(EtsCoroutine *coro, const XRefObjectOperator &rhsObject) const;

    EtsObject *Invoke(EtsCoroutine *coro, Span<VMHandle<ObjectHeader>> args) const;

    EtsObject *InvokeMethod(EtsCoroutine *coro, EtsObject *methodObject, Span<VMHandle<ObjectHeader>> args) const;

    EtsObject *InvokeMethod(EtsCoroutine *coro, const std::string &name, Span<VMHandle<ObjectHeader>> args) const;

    bool HasProperty(EtsCoroutine *coro, const std::string &name, bool isOwnProperty = false) const;

    bool HasProperty(EtsCoroutine *coro, EtsObject *keyObject, bool isOwnProperty = false) const;

    bool HasProperty(EtsCoroutine *coro, const uint32_t index) const;

    EtsObject *Instantiate(EtsCoroutine *coro, Span<VMHandle<ObjectHeader>> args) const;

    std::string TypeOf(EtsCoroutine *coro) const;

    bool IsTrue(EtsCoroutine *coro) const;

    napi_value GetNapiValue(EtsCoroutine *coro) const;

    static bool StrictEquals(EtsCoroutine *coro, const XRefObjectOperator &lhs, const XRefObjectOperator &rhs);

private:
    static napi_value ConvertStaticObjectToDynamic(EtsCoroutine *coro, EtsObject *object);

    napi_valuetype GetValueType(EtsCoroutine *coro) const;

    explicit XRefObjectOperator(EtsObject *etsObject);

    EtsObject *etsObject_;
};

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_XREF_OBJECT_OPERATOR_H_
