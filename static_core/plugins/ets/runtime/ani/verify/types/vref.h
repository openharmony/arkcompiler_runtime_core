/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VREF_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VREF_H

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ani/ani.h"
#include "common_interfaces/base/mem.h"

namespace ark::ets::ani::verify {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class VRef {
public:
    ani_ref GetRef();
    VRef() = delete;
    ~VRef() = delete;
};

class VObject : public VRef {
public:
    ani_object GetRef()
    {
        return static_cast<ani_object>(VRef::GetRef());
    }
};

class VType : public VObject {
public:
    ani_type GetRef()
    {
        return static_cast<ani_type>(VObject::GetRef());
    }
};

class VClass : public VType {
public:
    ani_class GetRef()
    {
        return static_cast<ani_class>(VType::GetRef());
    }
};

class VFnObject final : public VObject {
public:
    ani_fn_object GetRef()
    {
        return static_cast<ani_fn_object>(VObject::GetRef());
    }
};

class VEnum final : public VObject {
public:
    ani_enum GetRef()
    {
        return static_cast<ani_enum>(VObject::GetRef());
    }
};

class VEnumItem final : public VObject {
public:
    ani_enum_item GetRef()
    {
        return static_cast<ani_enum_item>(VObject::GetRef());
    }
};

class VModule final : public VClass {
public:
    ani_module GetRef()
    {
        return static_cast<ani_module>(VClass::GetRef());
    }
};

class VNamespace final : public VClass {
public:
    ani_namespace GetRef()
    {
        return static_cast<ani_namespace>(VClass::GetRef());
    }
};

class VString final : public VObject {
public:
    ani_string GetRef()
    {
        return static_cast<ani_string>(VObject::GetRef());
    }
};

class VError final : public VObject {
public:
    ani_error GetRef()
    {
        return static_cast<ani_error>(VObject::GetRef());
    }
};

class VArray final : public VObject {
public:
    ani_array GetRef()
    {
        return static_cast<ani_array>(VObject::GetRef());
    }
};

class VTupleValue final : public VObject {
public:
    ani_tuple_value GetRef()
    {
        return static_cast<ani_tuple_value>(VObject::GetRef());
    }
};

class VArrayBuffer final : public VObject {
public:
    ani_arraybuffer GetRef()
    {
        return static_cast<ani_arraybuffer>(VObject::GetRef());
    }
};

class VFixedArray : public VObject {
public:
    ani_fixedarray GetRef()
    {
        return static_cast<ani_fixedarray>(VObject::GetRef());
    }
};

class VValueArray : public VObject {
public:
    ani_valuearray GetRef()
    {
        return static_cast<ani_valuearray>(VObject::GetRef());
    }
};

class VValueArrayBoolean final : public VValueArray {
public:
    ani_valuearray_boolean GetRef()
    {
        return static_cast<ani_valuearray_boolean>(VValueArray::GetRef());
    }
};

class VValueArrayChar final : public VValueArray {
public:
    ani_valuearray_char GetRef()
    {
        return static_cast<ani_valuearray_char>(VValueArray::GetRef());
    }
};

class VValueArrayByte final : public VValueArray {
public:
    ani_valuearray_byte GetRef()
    {
        return static_cast<ani_valuearray_byte>(VValueArray::GetRef());
    }
};

class VValueArrayShort final : public VValueArray {
public:
    ani_valuearray_short GetRef()
    {
        return static_cast<ani_valuearray_short>(VValueArray::GetRef());
    }
};

class VValueArrayInt final : public VValueArray {
public:
    ani_valuearray_int GetRef()
    {
        return static_cast<ani_valuearray_int>(VValueArray::GetRef());
    }
};

class VValueArrayLong final : public VValueArray {
public:
    ani_valuearray_long GetRef()
    {
        return static_cast<ani_valuearray_long>(VValueArray::GetRef());
    }
};

class VValueArrayFloat final : public VValueArray {
public:
    ani_valuearray_float GetRef()
    {
        return static_cast<ani_valuearray_float>(VValueArray::GetRef());
    }
};

class VValueArrayDouble final : public VValueArray {
public:
    ani_valuearray_double GetRef()
    {
        return static_cast<ani_valuearray_double>(VValueArray::GetRef());
    }
};

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VREF_H
