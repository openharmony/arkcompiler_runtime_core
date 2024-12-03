/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef CPP_ABCKIT_LITERAL_IMPL_H
#define CPP_ABCKIT_LITERAL_IMPL_H

#include "./literal.h"

namespace abckit {

inline bool Literal::GetBool() const
{
    bool ret = GetApiConfig()->cIapi_->literalGetBool(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline abckit::LiteralArray Literal::GetLiteralArray() const
{
    LiteralArray ret(GetApiConfig()->cIapi_->literalGetLiteralArray(GetView()), GetApiConfig(), GetResource());
    CheckError(GetApiConfig());
    return ret;
}

}  // namespace abckit

#endif  // CPP_ABCKIT_LITERAL_IMPL_H
