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

#include "common/abc_file_utils.h"
#include "dump_utils.h"

namespace panda::abc2program {

LiteralTagToStringMap PandasmDumperUtils::literal_tag_to_string_map_ = {
    {panda_file::LiteralTag::BOOL, "u1"},
    {panda_file::LiteralTag::ARRAY_U1, "u1[]"},
    {panda_file::LiteralTag::ARRAY_U8, "u8[]"},
    {panda_file::LiteralTag::ARRAY_I8, "i8[]"},
    {panda_file::LiteralTag::ARRAY_U16, "u16[]"},
    {panda_file::LiteralTag::ARRAY_I16, "i16[]"},
    {panda_file::LiteralTag::ARRAY_U32, "u32[]"},
    {panda_file::LiteralTag::INTEGER, "i32"},
    {panda_file::LiteralTag::ARRAY_I32, "i32[]"},
    {panda_file::LiteralTag::ARRAY_U64, "u64[]"},
    {panda_file::LiteralTag::ARRAY_I64, "i64[]"},
    {panda_file::LiteralTag::ARRAY_F32, "f32[]"},
    {panda_file::LiteralTag::DOUBLE, "f64"},
    {panda_file::LiteralTag::ARRAY_F64, "f64[]"},
    {panda_file::LiteralTag::STRING, "string"},
    {panda_file::LiteralTag::ARRAY_STRING, "string[]"},
    {panda_file::LiteralTag::METHOD, "method"},
    {panda_file::LiteralTag::GETTER, "getter"},
    {panda_file::LiteralTag::SETTER, "setter"},
    {panda_file::LiteralTag::GENERATORMETHOD, "generator_method"},
    {panda_file::LiteralTag::ASYNCGENERATORMETHOD, "async_generator_method"},
    {panda_file::LiteralTag::ACCESSOR, "accessor"},
    {panda_file::LiteralTag::METHODAFFILIATE, "method_affiliate"},
    {panda_file::LiteralTag::NULLVALUE, "null_value"},
    {panda_file::LiteralTag::TAGVALUE, "tag_value"},
    {panda_file::LiteralTag::LITERALBUFFERINDEX, "literal_buffer_index"},
    {panda_file::LiteralTag::LITERALARRAY, "literal_array"},
    {panda_file::LiteralTag::BUILTINTYPEINDEX, "builtin_type_index"},
};

FunctionKindToStringMap PandasmDumperUtils::function_kind_to_string_map_ = {
    {panda_file::FunctionKind::NONE, "FunctionKind::NONE"},
    {panda_file::FunctionKind::FUNCTION, "FunctionKind::FUNCTION"},
    {panda_file::FunctionKind::NC_FUNCTION, "FunctionKind::NC_FUNCTION"},
    {panda_file::FunctionKind::GENERATOR_FUNCTION, "FunctionKind::GENERATOR_FUNCTION"},
    {panda_file::FunctionKind::ASYNC_FUNCTION, "FunctionKind::ASYNC_FUNCTION"},
    {panda_file::FunctionKind::ASYNC_GENERATOR_FUNCTION, "FunctionKind::ASYNC_GENERATOR_FUNCTION"},
    {panda_file::FunctionKind::ASYNC_NC_FUNCTION, "FunctionKind::ASYNC_NC_FUNCTION"},
    {panda_file::FunctionKind::CONCURRENT_FUNCTION, "FunctionKind::CONCURRENT_FUNCTION"},
};

std::string PandasmDumperUtils::GetFunctionKindString(panda_file::FunctionKind function_kind)
{
    auto it = function_kind_to_string_map_.find(function_kind);
    ASSERT(it != function_kind_to_string_map_.end());
    return it->second;
}

std::string PandasmDumperUtils::LiteralTagToString(const panda_file::LiteralTag &tag)
{
    auto it = literal_tag_to_string_map_.find(tag);
    ASSERT(it != literal_tag_to_string_map_.end());
    return it->second;
}

bool PandasmDumperUtils::IsMatchLiteralId(const pandasm::Ins &pa_ins)
{
    auto it = opcode_literal_id_index_map_.find(pa_ins.opcode);
    return (it != opcode_literal_id_index_map_.end());
}

size_t PandasmDumperUtils::GetLiteralIdIndex4Ins(const pandasm::Ins &pa_ins)
{
    auto it = opcode_literal_id_index_map_.find(pa_ins.opcode);
    ASSERT(it != opcode_literal_id_index_map_.end());
    return it->second;
}

std::string PandasmDumperUtils::GetMappedLabel(const std::string &label, const LabelMap &label_map)
{
    auto it = label_map.find(label);
    if (it != label_map.end()) {
        return it->second;
    } else {
        return "";
    }
}

pandasm::Ins PandasmDumperUtils::DeepCopyIns(const pandasm::Ins &input)
{
    pandasm::Ins res{};
    res.opcode = input.opcode;
    res.regs=input.regs;
    res.ids=input.ids;
    for (size_t i = 0; i < input.imms.size(); ++i) {
        pandasm::Ins::IType new_imm = input.imms[i];
        res.imms.emplace_back(new_imm);
    }
    res.label = input.label;
    res.set_label = input.set_label;
    auto &debug_ins = res.ins_debug;
    debug_ins.line_number = input.ins_debug.line_number;
    debug_ins.column_number = input.ins_debug.column_number;
    debug_ins.bound_left = input.ins_debug.bound_left;
    debug_ins.bound_right = input.ins_debug.bound_right;
    return res;
}

pandasm::Function::CatchBlock PandasmDumperUtils::DeepCopyCatchBlock(
    const pandasm::Function::CatchBlock &catch_block)
{
    pandasm::Function::CatchBlock res{};
    res.whole_line = catch_block.whole_line;
    res.exception_record = catch_block.exception_record;
    res.try_begin_label = catch_block.try_begin_label;
    res.try_end_label = catch_block.try_end_label;
    res.catch_begin_label = catch_block.catch_begin_label;
    res.catch_end_label = catch_block.catch_end_label;
    return res;
}

uint32_t PandasmDumperUtils::GetLiteralArrayIdFromName(const std::string &literal_array_id_name)
{
    auto pos = literal_array_id_name.rfind(UNDERLINE);
    ASSERT(pos != std::string::npos);
    std::stringstream id_str(literal_array_id_name.substr(pos + 1));
    uint32_t id = 0;
    id_str >> id;
    ASSERT(!id_str.fail());
    return id;
}

}  // namespace panda::abc2program