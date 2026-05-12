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

class VFnObject final : public VObject {
public:
    ani_fn_object GetRef()
    {
        return static_cast<ani_fn_object>(VObject::GetRef());
    }
};

class VClass : public VObject {
public:
    ani_class GetRef()
    {
        return static_cast<ani_class>(VObject::GetRef());
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

class VFixedArrayBoolean final : public VFixedArray {
public:
    ani_fixedarray_boolean GetRef()
    {
        return static_cast<ani_fixedarray_boolean>(VFixedArray::GetRef());
    }
};

class VFixedArrayChar final : public VFixedArray {
public:
    ani_fixedarray_char GetRef()
    {
        return static_cast<ani_fixedarray_char>(VFixedArray::GetRef());
    }
};

class VFixedArrayByte final : public VFixedArray {
public:
    ani_fixedarray_byte GetRef()
    {
        return static_cast<ani_fixedarray_byte>(VFixedArray::GetRef());
    }
};

class VFixedArrayShort final : public VFixedArray {
public:
    ani_fixedarray_short GetRef()
    {
        return static_cast<ani_fixedarray_short>(VFixedArray::GetRef());
    }
};

class VFixedArrayInt final : public VFixedArray {
public:
    ani_fixedarray_int GetRef()
    {
        return static_cast<ani_fixedarray_int>(VFixedArray::GetRef());
    }
};

class VFixedArrayLong final : public VFixedArray {
public:
    ani_fixedarray_long GetRef()
    {
        return static_cast<ani_fixedarray_long>(VFixedArray::GetRef());
    }
};

class VFixedArrayFloat final : public VFixedArray {
public:
    ani_fixedarray_float GetRef()
    {
        return static_cast<ani_fixedarray_float>(VFixedArray::GetRef());
    }
};

class VFixedArrayDouble final : public VFixedArray {
public:
    ani_fixedarray_double GetRef()
    {
        return static_cast<ani_fixedarray_double>(VFixedArray::GetRef());
    }
};

class VFixedArrayRef final : public VFixedArray {
public:
    ani_fixedarray_ref GetRef()
    {
        return static_cast<ani_fixedarray_ref>(VFixedArray::GetRef());
    }
};

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VREF_H
