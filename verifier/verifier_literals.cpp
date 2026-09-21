/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "verifier_internal.h"

namespace panda::verifier {

bool Verifier::ValidateLiteralStructure(const panda_file::File::EntityId &literal_id,
                                        std::unordered_set<uint32_t> &validated_literals,
                                        std::vector<uint32_t> &pending_literals)
{
    if (validated_literals.count(literal_id.GetOffset()) > 0) {
        return true;
    }
    validated_literals.insert(literal_id.GetOffset());
    LiteralArrayWalkResult result;
    if (!WalkLiteralArray(literal_id, result)) {
        return false;
    }
    for (const auto &inner_id : result.literal_ids) {
        pending_literals.push_back(inner_id);
    }
    if (literal_walk_cache_.size() < MAX_LITERAL_WALK_CACHE_SIZE) {
        literal_walk_cache_.emplace(literal_id.GetOffset(), std::move(result));
    }
    return true;
}

bool Verifier::VerifyMethodIdInLiteralArray(const uint32_t &id)
{
    if (all_method_ids_.count(id) == 0) {
        LOG(ERROR, VERIFIER) << "Invalid method id(0x" << std::hex << id << ") in literal array!";
        return false;
    }
    return true;
}

bool Verifier::VerifyLiteralIdInLiteralArray(const uint32_t &id)
{
    if (literal_ids_.count(id) == 0) {
        LOG(ERROR, VERIFIER) << "Invalid literal id(0x" << std::hex << id << ") in literal array!";
        return false;
    }
    return true;
}

Verifier::LiteralValueResult Verifier::ReadLiteralValue(Span<const uint8_t> *sp, panda_file::LiteralTag tag,
                                                        uint32_t &value)
{
    size_t width = LiteralValueWidth(tag);
    if (width == 0) {
        return LiteralValueResult::INVALID_TAG;
    }
    auto v = ReadBounded(sp, width);
    if (!v.has_value()) {
        return LiteralValueResult::OUT_OF_BOUNDS;
    }
    if (tag == panda_file::LiteralTag::DOUBLE && IsImpureNaN(bit_cast<double>(v.value()))) {
        return LiteralValueResult::IMPURE_NAN;
    }
    value = static_cast<uint32_t>(v.value());
    return LiteralValueResult::OK;
}

void Verifier::RecordLiteralItem(panda_file::LiteralTag tag, uint32_t value, LiteralArrayWalkResult &result)
{
    switch (GetLiteralTagDesc(tag).kind) {
        case LiteralItemKind::STRING_ID:
            result.string_ids.push_back(value);
            break;
        case LiteralItemKind::METHOD_ID:
            result.method_ids.push_back(value);
            result.method_like_tag_num++;
            break;
        case LiteralItemKind::LITERAL_ID:
            result.literal_ids.push_back(value);
            break;
        default:
            break;
    }
    result.prev_tag = result.last_tag;
    result.prev_value = result.last_value;
    result.last_tag = static_cast<uint32_t>(tag);
    result.last_value = value;
}

bool Verifier::SkipArrayLiteralData(Span<const uint8_t> *sp, panda_file::LiteralTag tag)
{
    auto len_opt = ReadBounded(sp, sizeof(uint32_t));
    if (!len_opt.has_value()) {
        return false;
    }
    auto len = len_opt.value();
    size_t elem_size = ArrayElementSize(tag);
    if (elem_size == 0 || len > static_cast<uint64_t>(sp->Size()) / elem_size) {
        return false;
    }
    *sp = sp->SubSpan(len * elem_size);
    return true;
}

bool Verifier::BeginLiteralArrayWalk(const panda_file::File::EntityId &literal_id, Span<const uint8_t> &sp,
                                     LiteralArrayWalkResult &result)
{
    if (literal_id.GetOffset() == 0 || literal_id.GetOffset() >= file_->GetHeader()->file_size) {
        LOG(ERROR, VERIFIER) << "Invalid literal id. literal_id(0x" << std::hex << literal_id.GetOffset() << ")!";
        return false;
    }
    sp = file_->GetSpanFromId(literal_id);
    const auto literal_vals_num_opt = ReadBounded(&sp, sizeof(uint32_t));
    if (!literal_vals_num_opt.has_value()) {
        LOG(ERROR, VERIFIER) << "Literal array 0x" << std::hex << literal_id.GetOffset() << " is out of bounds!";
        return false;
    }
    result = LiteralArrayWalkResult {};
    result.literal_vals_num = static_cast<uint32_t>(literal_vals_num_opt.value());
    if (result.literal_vals_num % 2U != 0) {
        LOG(ERROR, VERIFIER) << "Invalid literal array size 0x" << std::hex << result.literal_vals_num
                             << " in literal array 0x" << literal_id.GetOffset();
        return false;
    }
    return true;
}

bool Verifier::WalkLiteralArray(const panda_file::File::EntityId &literal_id, LiteralArrayWalkResult &result)
{
    Span<const uint8_t> sp;
    if (!BeginLiteralArrayWalk(literal_id, sp, result)) {
        return false;
    }
    for (size_t i = 0; i < result.literal_vals_num; i += 2U) {  // 2u skip literal item
        if (sp.Size() < panda_file::TAG_SIZE) {
            LOG(ERROR, VERIFIER) << "Literal array 0x" << std::hex << literal_id.GetOffset() << " is out of bounds!";
            return false;
        }
        const auto tag = static_cast<panda_file::LiteralTag>(panda_file::helpers::Read<panda_file::TAG_SIZE>(&sp));
        if (IsArrayLiteralTag(tag)) {
            if (!SkipArrayLiteralData(&sp, tag)) {
                LOG(ERROR, VERIFIER) << "Array data is out of bounds in literal array 0x" << std::hex
                                     << literal_id.GetOffset() << "!";
                return false;
            }
            result.has_array_tag = true;
            return true;
        }
        uint32_t value = 0;
        switch (ReadLiteralValue(&sp, tag, value)) {
            case LiteralValueResult::OK:
                break;
            case LiteralValueResult::IMPURE_NAN:
                LOG(ERROR, VERIFIER) << "Impure NaN double value in literal array 0x" << std::hex
                                     << literal_id.GetOffset() << "!";
                return false;
            default:
                LOG(ERROR, VERIFIER) << "Fail to verify the item with tag 0x" << std::hex << static_cast<uint32_t>(tag)
                                     << " in literal array 0x" << literal_id.GetOffset() << "!";
                return false;
        }
        RecordLiteralItem(tag, value, result);
    }
    return true;
}

bool Verifier::VerifySingleLiteralArray(const panda_file::File::EntityId &literal_id)
{
    LiteralArrayWalkResult walked_result;
    const auto cached = literal_walk_cache_.find(literal_id.GetOffset());
    const LiteralArrayWalkResult *result = nullptr;
    if (cached != literal_walk_cache_.end()) {
        result = &cached->second;
    } else {
        if (!WalkLiteralArray(literal_id, walked_result)) {
            return false;
        }
        result = &walked_result;
    }
    for (const auto &string_id : result->string_ids) {
        if (!VerifyStringItem(string_id, "literal array")) {
            return false;
        }
    }
    for (const auto &method_id : result->method_ids) {
        inner_method_map_.emplace(literal_id.GetOffset(), method_id);
        if (!VerifyMethodIdInLiteralArray(method_id)) {
            return false;
        }
    }
    for (const auto &inner_literal_id : result->literal_ids) {
        inner_literal_map_.emplace(literal_id.GetOffset(), inner_literal_id);
        if (!VerifyLiteralIdInLiteralArray(inner_literal_id)) {
            return false;
        }
    }
    if (class_literal_ids_.count(literal_id.GetOffset()) > 0 ||
        sendable_class_literal_ids_.count(literal_id.GetOffset()) > 0) {
        return VerifyNonStaticNum(literal_id, *result);
    }
    return true;
}

bool Verifier::ReadClassLiteralNonStaticNum(const LiteralArrayWalkResult &result, bool is_sendable,
                                            uint32_t &non_static_num) const
{
    if (is_sendable) {
        if (result.last_tag != static_cast<uint32_t>(panda_file::LiteralTag::LITERALARRAY) ||
            result.prev_tag != static_cast<uint32_t>(panda_file::LiteralTag::INTEGER)) {
            return false;
        }
        non_static_num = result.prev_value;
        return true;
    }
    if (result.last_tag != static_cast<uint32_t>(panda_file::LiteralTag::INTEGER)) {
        return false;
    }
    non_static_num = result.last_value;
    return true;
}

bool Verifier::VerifyNonStaticNum(const panda_file::File::EntityId &literal_id, const LiteralArrayWalkResult &result)
{
    if (result.has_array_tag) {
        LOG(ERROR, VERIFIER) << "Invalid array tag in class literal array 0x" << std::hex << literal_id.GetOffset()
                             << "!";
        return false;
    }
    bool is_sendable = sendable_class_literal_ids_.count(literal_id.GetOffset()) > 0;
    uint32_t non_static_num = 0;
    if (!ReadClassLiteralNonStaticNum(result, is_sendable, non_static_num)) {
        LOG(ERROR, VERIFIER) << "Invalid class literal array 0x" << std::hex << literal_id.GetOffset() << "!";
        return false;
    }
    const uint32_t item_num = result.literal_vals_num / 2U;
    uint32_t reserved_num = is_sendable ? 2U : 1U;
    if (item_num < result.method_like_tag_num + reserved_num) {
        LOG(ERROR, VERIFIER) << "Invalid class literal array 0x" << std::hex << literal_id.GetOffset() << "!";
        return false;
    }
    uint32_t upper_bound = (item_num - result.method_like_tag_num - reserved_num) / 2U;
    if (non_static_num > upper_bound) {
        LOG(ERROR, VERIFIER) << "nonStaticNum " << non_static_num << " is out of bounds, max allowed: " << upper_bound;
        return false;
    }
    return true;
}

bool Verifier::VerifyLiteralArrays()
{
    for (const auto &module_record_id : module_record_literal_ids_) {
        if (!VerifyModuleRecord(panda_file::File::EntityId(module_record_id))) {
            return false;
        }
    }
    for (const auto &lazy_import_id : lazy_import_literal_ids_) {
        if (!VerifyLazyImportFlags(panda_file::File::EntityId(lazy_import_id))) {
            return false;
        }
    }
    for (const auto &arg_literal_id : literal_ids_) {
        const auto literal_id = panda_file::File::EntityId(arg_literal_id);
        if (IsModuleLiteralId(literal_id)) {
            continue;
        }
        if (!VerifySingleLiteralArray(literal_id)) {
            return false;
        }
    }
    return true;
}

}  // namespace panda::verifier
