/**
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

#ifndef LIBABCKIT_SRC_ADAPTER_STATIC_METADATA_MODIFY_STATIC_H
#define LIBABCKIT_SRC_ADAPTER_STATIC_METADATA_MODIFY_STATIC_H

#include "libabckit/include/c/metadata_core.h"
#include "libabckit/include/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/include/c/extensions/js/metadata_js.h"

namespace libabckit {

// ========================================
// Create / Update
// ========================================

AbckitString *CreateStringStatic(AbckitFile *file, const char *value);

void FunctionSetGraphStatic(AbckitCoreFunction *function, AbckitGraph *graph);

AbckitLiteral *CreateLiteralBoolStatic(AbckitFile *file, bool value);
AbckitLiteral *CreateLiteralU8Static(AbckitFile *file, uint8_t value);
AbckitLiteral *CreateLiteralU16Static(AbckitFile *file, uint16_t value);
AbckitLiteral *CreateLiteralMethodAffiliateStatic(AbckitFile *file, uint16_t value);
AbckitLiteral *CreateLiteralU32Static(AbckitFile *file, uint32_t value);
AbckitLiteral *CreateLiteralU64Static(AbckitFile *file, uint64_t value);
AbckitLiteral *CreateLiteralFloatStatic(AbckitFile *file, float value);
AbckitLiteral *CreateLiteralDoubleStatic(AbckitFile *file, double value);
AbckitLiteral *CreateLiteralStringStatic(AbckitFile *file, const char *value);
AbckitLiteral *CreateLiteralMethodStatic(AbckitFile *file, AbckitCoreFunction *function);
AbckitLiteralArray *CreateLiteralArrayStatic(AbckitFile *file, AbckitLiteral **value, size_t size);

AbckitValue *CreateValueU1Static(AbckitFile *file, bool value);
AbckitValue *CreateValueDoubleStatic(AbckitFile *file, double value);
AbckitValue *CreateValueStringStatic(AbckitFile *file, const char *value);
AbckitValue *CreateLiteralArrayValueStatic(AbckitFile *file, AbckitValue **value, size_t size);
}  // namespace libabckit

#endif
