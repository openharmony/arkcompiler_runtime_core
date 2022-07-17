/*
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

#include <iomanip>
#include <tuple>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "annotation_data_accessor.h"
#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "class_data_accessor-inl.h"
#include "code_data_accessor-inl.h"
#include "debug_data_accessor-inl.h"
#include "debug_info_extractor.h"
#include "field_data_accessor-inl.h"
#include "file_items.h"
#include "lexer.h"
#include "method_data_accessor-inl.h"
#include "param_annotations_data_accessor.h"
#include "proto_data_accessor-inl.h"
#include "utils/span.h"
#include "utils/leb128.h"
#include "utils/utf.h"

namespace panda::test {

using namespace panda::pandasm;

static const uint8_t *GetTypeDescriptor(const std::string &name, std::string *storage)
{
    *storage = "L" + name + ";";
    std::replace(storage->begin(), storage->end(), '.', '/');
    return utf::CStringAsMutf8(storage->c_str());
}

TEST(emittertests, get_GLOBAL_lang_for_JS_func)
{
    Parser p;
    auto source = R"(
        .language ECMAScript

        .function any main() {
            return.dyn
        }
    )";

    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    std::string descriptor;

    auto class_id = pf->GetClassId(GetTypeDescriptor("_GLOBAL", &descriptor));
    ASSERT_TRUE(class_id.IsValid());

    panda_file::ClassDataAccessor cda(*pf, class_id);

    ASSERT_TRUE(cda.GetSourceLang().has_value());
    ASSERT_EQ(cda.GetSourceLang(), panda_file::SourceLang::ECMASCRIPT);
}

TEST(emittertests, ecmascript_debuginfo)
{
    Parser p;

    auto source = R"(
        .language ECMAScript
        .function void main() {
            return.void
        }
    )";

    std::string source_filename = "source.pa";
    auto res = p.Parse(source, source_filename);
    ASSERT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    panda_file::DebugInfoExtractor extractor(pf.get());
    auto methods = extractor.GetMethodIdList();
    ASSERT_EQ(methods.size(), 1);
    auto lineTable = extractor.GetLineNumberTable(methods[0]);
    auto columnTable = extractor.GetColumnNumberTable(methods[0]);

    EXPECT_EQ(lineTable.size(), 1);
    EXPECT_EQ(columnTable.size(), 1);
}

}  // namespace panda::test
