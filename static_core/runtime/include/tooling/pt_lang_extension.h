/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_TOOLING_PT_LANG_EXTENSION_H
#define PANDA_TOOLING_PT_LANG_EXTENSION_H

#include "runtime/include/tooling/pt_object.h"
#include "runtime/include/tooling/pt_property.h"
#include "runtime/include/tooling/pt_value.h"

// NOTE(maksenov): remove this file after refactoring js_runtime
// NOTE(a.urakov): move here current InspectorExtension instead
namespace ark::tooling {
class PtMethod {
public:
    PtMethod() = default;
    explicit PtMethod(void * /* unused */) {}
};

class PtClass {
public:
    PtClass() = default;
    explicit PtClass(void * /* unused */) {}
};

class PtLangExt {
public:
    PtLangExt() = default;
    virtual ~PtLangExt() = default;

    // PtValue API
    virtual PtObject ValueToObject(PtValue value) const = 0;

    // PtClass API
    virtual PtClass GetClass(PtObject object) const = 0;
    virtual PtClass GetClass(PtProperty property) const = 0;
    virtual void ReleaseClass(PtClass klass) const = 0;
    virtual const char *GetClassDescriptor(PtClass klass) const = 0;

    // PtObject API
    virtual PandaList<PtProperty> GetProperties(PtObject object) const = 0;
    virtual PtProperty GetProperty(PtObject object, const char *propertyName) const = 0;
    virtual bool AddProperty(PtObject object, const char *propertyName, PtValue value) const = 0;
    virtual bool RemoveProperty(PtObject object, const char *propertyName) const = 0;

    // PtProperty API
    virtual const char *GetPropertyName(PtProperty propery) const = 0;
    virtual PtValue GetPropertyValue(PtProperty property) const = 0;
    virtual void SetPropertyPtValue(PtProperty property, PtValue value) const = 0;
    virtual void ReleasePtValue(const PtValue *value) const = 0;

    NO_COPY_SEMANTIC(PtLangExt);
    NO_MOVE_SEMANTIC(PtLangExt);
};
}  // namespace ark::tooling

#endif  // PANDA_TOOLING_PT_LANG_EXTENSION_H
