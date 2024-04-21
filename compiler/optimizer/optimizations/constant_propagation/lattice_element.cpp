/*
 * Copyright (c) 2024 Shenzhen Kaihong Digital Industry Development Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "constant_propagation.h"

namespace panda::compiler {
LatticeElement::LatticeElement(LatticeType type) : type_(type) {}

TopElement::TopElement() : LatticeElement(LatticeType::LATTICE_TOP) {}

LatticeElement *TopElement::Meet(LatticeElement *other)
{
    return other;
}

LatticeElement *TopElement::GetInstance()
{
    static TopElement instance;
    return &instance;
}

std::string TopElement::ToString()
{
    return "Top";
}

BottomElement::BottomElement() : LatticeElement(LatticeType::LATTICE_BOTTOM) {}

LatticeElement *BottomElement::Meet(LatticeElement *other)
{
    return this;
}

LatticeElement *BottomElement::GetInstance()
{
    static BottomElement instance;
    return &instance;
}

std::string BottomElement::ToString()
{
    return "Bottom";
}

ConstantElement::ConstantElement(bool val)
    : LatticeElement(LatticeType::LATTICE_CONSTANT), type_(ConstantType::CONSTANT_BOOL), val_(val)
{
}

ConstantElement::ConstantElement(int32_t val)
    : LatticeElement(LatticeType::LATTICE_CONSTANT), type_(ConstantType::CONSTANT_INT32), val_(val)
{
}

ConstantElement::ConstantElement(int64_t val)
    : LatticeElement(LatticeType::LATTICE_CONSTANT), type_(ConstantType::CONSTANT_INT64), val_(val)
{
}

ConstantElement::ConstantElement(double val)
    : LatticeElement(LatticeType::LATTICE_CONSTANT), type_(ConstantType::CONSTANT_DOUBLE), val_(val)
{
}

LatticeElement *ConstantElement::Meet(LatticeElement *other)
{
    if (other->IsTopElement()) {
        return this;
    }
    if (other->IsBottomElement()) {
        return other;
    }

    if ((type_ == other->AsConstant()->GetType()) && (val_ == other->AsConstant()->GetVal())) {
        return this;
    }

    return BottomElement::GetInstance();
}

std::string ConstantElement::ToString()
{
    return "Constant";
}

ConstantElement *ConstantElement::AsConstant()
{
    return this;
}
}  // namespace panda::compiler
