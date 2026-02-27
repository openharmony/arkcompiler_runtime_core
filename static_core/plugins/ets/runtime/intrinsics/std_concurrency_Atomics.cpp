/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/types/atomics/ets_atomic_primitives.h"
#include "plugins/ets/runtime/types/atomics/ets_atomic_ref.h"

namespace ark::ets::intrinsics {

// CC-OFFNXT(G.PRE.06): code generation
#define DEFINE_DEFAULT_ATOMIC_OPERATIONS(typeName)                                                               \
    extern "C" Ets##typeName EtsAtomic##typeName##LoadValue(EtsObject *atomic)                                   \
    {                                                                                                            \
        /* CC-OFFNXT(G.PRE.05): code generation */                                                               \
        return EtsAtomic##typeName::FromEtsObject(atomic)->LoadValue();                                          \
    }                                                                                                            \
                                                                                                                 \
    extern "C" void EtsAtomic##typeName##StoreValue(EtsObject *atomic, Ets##typeName ref)                        \
    {                                                                                                            \
        /* CC-OFFNXT(G.PRE.05): code generation */                                                               \
        return EtsAtomic##typeName::FromEtsObject(atomic)->StoreValue(ref);                                      \
    }                                                                                                            \
                                                                                                                 \
    extern "C" Ets##typeName EtsAtomic##typeName##ExchangeValue(EtsObject *atomic, Ets##typeName ref)            \
    {                                                                                                            \
        /* CC-OFFNXT(G.PRE.05): code generation */                                                               \
        return EtsAtomic##typeName::FromEtsObject(atomic)->ExchangeValue(ref);                                   \
    }                                                                                                            \
                                                                                                                 \
    extern "C" Ets##typeName EtsAtomic##typeName##CompareAndSwapValue(EtsObject *atomic, Ets##typeName expected, \
                                                                      Ets##typeName ref)                         \
    {                                                                                                            \
        /* CC-OFFNXT(G.PRE.05): code generation */                                                               \
        return EtsAtomic##typeName::FromEtsObject(atomic)->CompareAndSwapValue(expected, ref);                   \
    }                                                                                                            \
                                                                                                                 \
    extern "C" EtsBoolean EtsAtomic##typeName##IsLockFreeCheck()                                                 \
    {                                                                                                            \
        /* CC-OFFNXT(G.PRE.05): code generation */                                                               \
        return EtsAtomic##typeName::IsLockFreeCheck();                                                           \
    }

// CC-OFFNXT(G.PRE.06): code generation
#define DEFILE_FETCH_ADD_FETCH_SUB(typeName)                                                            \
    extern "C" Ets##typeName EtsAtomic##typeName##FetchAddValue(EtsObject *atomic, Ets##typeName value) \
    {                                                                                                   \
        /* CC-OFFNXT(G.PRE.05): code generation */                                                      \
        return EtsAtomic##typeName::FromEtsObject(atomic)->FetchAddValue(value);                        \
    }                                                                                                   \
                                                                                                        \
    extern "C" Ets##typeName EtsAtomic##typeName##FetchSubValue(EtsObject *atomic, Ets##typeName value) \
    {                                                                                                   \
        /* CC-OFFNXT(G.PRE.05): code generation */                                                      \
        return EtsAtomic##typeName::FromEtsObject(atomic)->FetchSubValue(value);                        \
    }

// CC-OFFNXT(G.PRE.06): code generation
#define DEFILE_FETCH_AND_FETCH_OR_FETCH_XOR(typeName)                                                   \
    extern "C" Ets##typeName EtsAtomic##typeName##FetchAndValue(EtsObject *atomic, Ets##typeName value) \
    {                                                                                                   \
        /* CC-OFFNXT(G.PRE.05): code generation */                                                      \
        return EtsAtomic##typeName::FromEtsObject(atomic)->FetchAndValue(value);                        \
    }                                                                                                   \
                                                                                                        \
    extern "C" Ets##typeName EtsAtomic##typeName##FetchOrValue(EtsObject *atomic, Ets##typeName value)  \
    {                                                                                                   \
        /* CC-OFFNXT(G.PRE.05): code generation */                                                      \
        return EtsAtomic##typeName::FromEtsObject(atomic)->FetchOrValue(value);                         \
    }                                                                                                   \
                                                                                                        \
    extern "C" Ets##typeName EtsAtomic##typeName##FetchXorValue(EtsObject *atomic, Ets##typeName value) \
    {                                                                                                   \
        /* CC-OFFNXT(G.PRE.05): code generation */                                                      \
        return EtsAtomic##typeName::FromEtsObject(atomic)->FetchXorValue(value);                        \
    }

// CC-OFFNXT(G.PRE.02-CPP) code generation
#define DEFINE_ATOMIC_METHODS_FOR_INTEGRAL_TYPE(type) \
    DEFINE_DEFAULT_ATOMIC_OPERATIONS(type)            \
    DEFILE_FETCH_ADD_FETCH_SUB(type)                  \
    DEFILE_FETCH_AND_FETCH_OR_FETCH_XOR(type)

// CC-OFFNXT(G.PRE.02-CPP) code generation
#define DEFINE_ATOMIC_METHODS_FOR_NON_INTEGRAL_TYPE(type) DEFINE_DEFAULT_ATOMIC_OPERATIONS(type)

using EtsReference = EtsObject *;
DEFINE_ATOMIC_METHODS_FOR_NON_INTEGRAL_TYPE(Reference)

DEFINE_ATOMIC_METHODS_FOR_NON_INTEGRAL_TYPE(Char)
DEFINE_ATOMIC_METHODS_FOR_NON_INTEGRAL_TYPE(Boolean)
DEFINE_ATOMIC_METHODS_FOR_NON_INTEGRAL_TYPE(Float)
DEFINE_ATOMIC_METHODS_FOR_NON_INTEGRAL_TYPE(Double)

DEFINE_ATOMIC_METHODS_FOR_INTEGRAL_TYPE(Int);
DEFINE_ATOMIC_METHODS_FOR_INTEGRAL_TYPE(Short);
DEFINE_ATOMIC_METHODS_FOR_INTEGRAL_TYPE(Byte);
DEFINE_ATOMIC_METHODS_FOR_INTEGRAL_TYPE(Long);

#undef DEFINE_ATOMIC_METHODS_FOR_NON_INTEGRAL_TYPE
#undef DEFINE_ATOMIC_METHODS_FOR_INTEGRAL_TYPE
#undef DEFINE_DEFAULT_ATOMIC_OPERATIONS
#undef DEFILE_FETCH_ADD_FETCH_SUB
#undef DEFILE_FETCH_AND_FETCH_OR_FETCH_XOR

}  // namespace ark::ets::intrinsics
