/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#ifndef VERIFIER_VERIFIER_H
#define VERIFIER_VERIFIER_H

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "bytecode_instruction_enum_gen.h"
#include "bytecode_instruction-inl.h"
#include "class_data_accessor.h"
#include "code_data_accessor-inl.h"
#include "file.h"
#include "file_format_version.h"
#include "file_items.h"
#include "literal_data_accessor.h"
#include "method_data_accessor-inl.h"
#include "utils/bit_utils.h"
#include "utils/span.h"
#include "utils/utf.h"

namespace panda::verifier {

using Opcode = BytecodeInstruction::Opcode;
using TaggedType = uint64_t;

enum class ActionType {
    CHECKCONSTPOOLCONTENT,
    COLLECTINFOS,
};

class Verifier {
public:
    explicit Verifier(const std::string &filename);
    ~Verifier() = default;

    bool Verify();
    bool CollectIdInfos();
    bool VerifyChecksum();
    bool VerifyConstantPool();
    bool VerifyRegisterIndex();
    bool VerifyConstantPoolIndex();
    bool VerifyConstantPoolContent();

    bool include_literal_array_ids = true;
    std::unordered_set<uint32_t> literal_ids_;
    std::unordered_map<uint32_t, uint32_t> inner_literal_map_;
    std::unordered_map<uint32_t, uint32_t> inner_method_map_;

    static constexpr size_t TAG_BITS_SIZE = 16;
    static constexpr size_t TAG_BITS_SHIFT = BitNumbers<TaggedType>() - TAG_BITS_SIZE;
    static_assert((TAG_BITS_SHIFT + TAG_BITS_SIZE) == sizeof(TaggedType) * CHAR_BIT, "Insufficient bits!");
    static constexpr TaggedType TAG_MASK = ((1ULL << TAG_BITS_SIZE) - 1ULL) << TAG_BITS_SHIFT;
    static constexpr TaggedType TAG_INT = TAG_MASK;
    static constexpr size_t DOUBLE_ENCODE_OFFSET_BIT = 48;
    static constexpr TaggedType DOUBLE_ENCODE_OFFSET = 1ULL << DOUBLE_ENCODE_OFFSET_BIT;

private:
    struct InstructionIds {
        std::optional<panda_file::File::EntityId> literal_id {};
        std::optional<panda_file::File::EntityId> method_id {};
        std::optional<panda_file::File::EntityId> string_id {};
    };
    struct InsnRegInfo;
    class RegisterDefinitionDataflow;
    class IcSlotAssignment;

    enum class LiteralValueResult {
        OK,
        INVALID_TAG,
        OUT_OF_BOUNDS,
        IMPURE_NAN,
    };

    struct LiteralArrayWalkResult {
        uint32_t literal_vals_num {0};
        uint32_t method_like_tag_num {0};
        uint32_t last_tag {0};
        uint32_t last_value {0};
        uint32_t prev_tag {0};
        uint32_t prev_value {0};
        bool has_array_tag {false};
        std::vector<uint32_t> string_ids {};
        std::vector<uint32_t> method_ids {};
        std::vector<uint32_t> literal_ids {};
    };

    struct IcSlotInsnInfo {
        bool is_two_slot {false};
        bool is_8bit_only {false};
        uint32_t slot {0};
    };

    struct IcSlotState {
        uint32_t ann_slot_num {0};
        bool ic_check_enabled {false};
    };

    struct MethodInfos {
        BytecodeInstruction bc_ins_init;
        BytecodeInstruction bc_ins_last;
        panda_file::MethodDataAccessor &method_accessor;
        panda_file::File::EntityId method_id;
        uint64_t valid_regs_num {0};
        const uint8_t *ins_arr {nullptr};
        IcSlotState ic_state {};
        std::vector<IcSlotInsnInfo> ic_insns {};
    };

    void GetLiteralIds();
    template <typename T>
    void PushToLiteralIds(T &ids);
    void GetConstantPoolIds();
    bool ResolveInstructionIds(const BytecodeInstruction &bc_ins, const panda_file::File::EntityId &method_id,
                               InstructionIds &ids);
    void CollectClassLiteralId(const BytecodeInstruction &bc_ins, const panda_file::File::EntityId &literal_id);
    bool CollectIdInInstructions(const panda_file::File::EntityId &method_id, bool module_instructions_allowed);
    void CollectModuleLiteralId(const panda_file::File::EntityId &field_id);
    void CollectRecordField(panda_file::FieldDataAccessor &field_accessor, const panda_file::File::EntityId &record_id,
                            bool is_esmodule_record_class, bool &record_is_esmodule, bool &record_has_tla);
    void CollectRecordInfo(panda_file::ClassDataAccessor &class_accessor, const panda_file::File::EntityId &record_id,
                           bool &record_is_esmodule, bool &record_has_tla);
    bool DetectSpecialRecords();
    bool ForEachInternalRecord(
        const std::function<bool(const panda_file::File::EntityId &, panda_file::ClassDataAccessor &)> &cb);
    bool CheckRecord(const panda_file::File::EntityId &record_id, panda_file::ClassDataAccessor &class_accessor,
                     const verifier::ActionType type);
    bool CheckConstantPool(const verifier::ActionType type);
    static size_t GetVRegCount(const BytecodeInstruction &bc_ins);
    bool CheckConstantPoolActions(const verifier::ActionType type, panda_file::File::EntityId method_id,
                                  bool module_instructions_allowed);
    bool VerifyMethodId(const uint32_t &method_id) const;
    bool VerifyLiteralId(const uint32_t &literal_id) const;
    bool VerifyStringId(const uint32_t &string_id) const;
    bool IsDeclaredMethodId(const uint32_t &method_id) const;
    bool IsExternalMethodId(const uint32_t &method_id) const;
    bool IsRangeInstAndHasInvalidRegIdx(const BytecodeInstruction &bc_ins, const size_t count, uint64_t valid_regs_num);
    bool IsRegIdxOutOfBounds(uint64_t reg_idx, uint64_t valid_regs_num);
    bool CheckVRegIdx(const BytecodeInstruction &bc_ins, const size_t count, uint64_t valid_regs_num);
    static std::optional<int64_t> GetFirstImmFromInstruction(const BytecodeInstruction &bc_ins);
    std::optional<uint64_t> FindAnnotationElementValue(panda_file::MethodDataAccessor &method_accessor,
                                                       std::string_view annotation_name,
                                                       std::string_view element_name) const;
    std::optional<uint64_t> GetSlotNumberFromAnnotation(panda_file::MethodDataAccessor &method_accessor);
    std::optional<uint32_t> GetCallTypeFromAnnotation(panda_file::MethodDataAccessor &method_accessor);
    bool VerifyMethodIdInLiteralArray(const uint32_t &id);
    bool VerifyStringItem(uint32_t string_offset, std::string_view where) const;
    bool VerifyLiteralIdInLiteralArray(const uint32_t &id);
    bool IsModuleLiteralId(const panda_file::File::EntityId &id) const;
    static void RecordLiteralItem(panda_file::LiteralTag tag, uint32_t value, LiteralArrayWalkResult &result);
    static LiteralValueResult ReadLiteralValue(Span<const uint8_t> *sp, panda_file::LiteralTag tag, uint32_t &value);
    static bool SkipArrayLiteralData(Span<const uint8_t> *sp, panda_file::LiteralTag tag);
    bool BeginLiteralArrayWalk(const panda_file::File::EntityId &literal_id, Span<const uint8_t> &sp,
                               LiteralArrayWalkResult &result);
    bool WalkLiteralArray(const panda_file::File::EntityId &literal_id, LiteralArrayWalkResult &result);
    bool ValidateInstructionBasics();
    bool ValidateMethodInstructionBasics(panda_file::MethodDataAccessor &method_accessor,
                                         std::unordered_set<uint32_t> &validated_literals,
                                         std::vector<uint32_t> &pending_literals);
    bool ValidateLiteralStructure(const panda_file::File::EntityId &literal_id,
                                  std::unordered_set<uint32_t> &validated_literals,
                                  std::vector<uint32_t> &pending_literals);
    bool VerifySingleLiteralArray(const panda_file::File::EntityId &literal_id);
    bool VerifyNonStaticNum(const panda_file::File::EntityId &literal_id, const LiteralArrayWalkResult &result);
    bool ReadClassLiteralNonStaticNum(const LiteralArrayWalkResult &result, bool is_sendable,
                                      uint32_t &non_static_num) const;
    bool CheckInstructionOpcodes(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last,
                                 const panda_file::File::EntityId &code_id,
                                 const panda_file::File::EntityId &method_id);
    void CollectResolvedInstructionIds(const BytecodeInstruction &bc_ins, const InstructionIds &ids);
    bool VerifyTryBlockOffsets(const BytecodeInstruction &try_begin_bc_ins, const BytecodeInstruction &try_end_bc_ins,
                               const BytecodeInstruction &bc_ins_last);
    bool VerifyModuleRecord(const panda_file::File::EntityId &module_id);
    bool VerifyModuleRequests(Span<const uint8_t> *sp, uint32_t num_module_requests);
    bool VerifyModuleEntries(Span<const uint8_t> *sp, uint32_t num_module_requests, uint32_t string_id_count,
                             bool has_module_request_idx);
    bool VerifyLazyImportFlags(const panda_file::File::EntityId &literal_id);
    bool VerifyLiteralArrays();
    bool VerifyJumpInstruction(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last,
                               const BytecodeInstruction &bc_ins_init, const uint8_t *ins_arr,
                               panda_file::File::EntityId code_id);
    bool CollectIcSlotInsn(const BytecodeInstruction &bc_ins, std::vector<IcSlotInsnInfo> &ic_insns);
    bool VerifyIcSlotAllocation(const std::vector<IcSlotInsnInfo> &ic_insns, const IcSlotState &ic_state,
                                const panda_file::File::EntityId &method_id);
    bool VerifyIcSlotRanges(const std::vector<IcSlotInsnInfo> &ic_insns, uint32_t ann_slot_num,
                            const panda_file::File::EntityId &method_id);
    static uint32_t GetRuntimeSlotCount(uint32_t ann_slot_num);
    bool CheckConstantPoolMethodContent(const panda_file::File::EntityId &method_id);
    bool CheckConstantPoolIndex() const;
    bool CheckConstantPoolIdsBounds() const;
    std::optional<uint64_t> SafeAdd(uint64_t a, uint64_t b) const;
    bool VerifyCatchBlocks(panda_file::CodeDataAccessor::TryBlock &try_block, const BytecodeInstruction &bc_ins,
                           const BytecodeInstruction &bc_ins_last, const panda_file::File::EntityId &method_id);
    bool VerifyCatchBlockOffsets(panda_file::CodeDataAccessor::CatchBlock &catch_block,
                                 const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last);
    bool VerifyTryBlocks(panda_file::CodeDataAccessor &code_accessor, const BytecodeInstruction &bc_ins,
                         const BytecodeInstruction &bc_ins_last, const panda_file::File::EntityId &method_id);
    bool PrecomputeInstructionIndices(const BytecodeInstruction &bc_ins_start, const BytecodeInstruction &bc_ins_last);
    bool IsMethodBytecodeInstruction(const BytecodeInstruction &bc_ins_cur);
    static bool IsInstructionEndInBounds(const BytecodeInstruction &bc_ins, const BytecodeInstruction &bc_ins_last);
    bool VerifyMethodRegisterIndex(panda_file::CodeDataAccessor &code_accessor,
                                   std::optional<uint64_t> &valid_regs_num);
    bool VerifyMethodHeaderInfo(panda_file::CodeDataAccessor &code_accessor,
                                panda_file::MethodDataAccessor &method_accessor, MethodInfos &infos);
    void PrepareIcSlotCheck(panda_file::MethodDataAccessor &method_accessor, IcSlotState &ic_state);
    void ReportCorruptedInstructionSequence(const panda_file::File::EntityId &code_id,
                                            const panda_file::File::EntityId &method_id) const;
    bool VerifyMethodInstructions(MethodInfos &infos);
    bool VerifyMethodCodeContent(panda_file::CodeDataAccessor &code_accessor, MethodInfos &infos);
    static bool IsSecondaryOpcodeValid(const BytecodeInstruction &bc_ins);
    bool VerifyMethodNumArgs(panda_file::CodeDataAccessor &code_accessor,
                             panda_file::MethodDataAccessor &method_accessor);
    bool VerifyProtoArgNumber(uint64_t num_args, panda_file::MethodDataAccessor &method_accessor);
    std::optional<uint32_t> GetProtoArgNumber(const panda_file::File::EntityId &proto_id) const;
    bool VerifyMethodInRecord(const uint32_t &method_id) const;
    void CollectInsnRegInfo(const BytecodeInstruction &bc_ins, uint64_t num_regs, InsnRegInfo &info,
                            std::vector<bool> &written_anywhere) const;
    void CollectInsnControlFlow(const BytecodeInstruction &bc_ins, InsnRegInfo &info) const;
    bool VerifyMethodRegisterInitialization(panda_file::CodeDataAccessor &code_accessor);
    void SeedCatchHandlerStates(panda_file::CodeDataAccessor &code_accessor, RegisterDefinitionDataflow &dataflow);
    void CollectMethodInsnRegInfos(panda_file::CodeDataAccessor &code_accessor, uint64_t num_regs,
                                   std::vector<InsnRegInfo> &insn_infos, std::vector<bool> &written_anywhere);
    bool CheckInsnRegisterInitialization(const std::vector<InsnRegInfo> &insn_infos,
                                         const RegisterDefinitionDataflow &dataflow);
    void CollectTlaBlockEntries(panda_file::CodeDataAccessor &code_accessor, const BytecodeInstruction &bc_ins,
                                const BytecodeInstruction &bc_ins_last,
                                std::unordered_set<const uint8_t *> &block_entries);
    void AddJumpTargetIfInRange(const BytecodeInstruction &current_ins, const BytecodeInstruction &bc_ins,
                                const BytecodeInstruction &bc_ins_last,
                                std::unordered_set<const uint8_t *> &block_entries);
    bool VerifyTlaMainFunctionReturn(panda_file::MethodDataAccessor &method_accessor);
    bool IsIcCheckEnabled() const;

    static inline bool IsImpureNaN(double value)
    {
        return bit_cast<TaggedType>(value) >= (Verifier::TAG_INT - Verifier::DOUBLE_ENCODE_OFFSET);
    }

    static bool IsModuleRelatedInstruction(const BytecodeInstruction &bc_ins);
    static bool IsRegisterWriteInstruction(const BytecodeInstruction &bc_ins);

    std::unique_ptr<const panda_file::File> file_;
    std::unordered_set<uint32_t> constant_pool_ids_;
    std::unordered_set<uint32_t> all_method_ids_;
    std::unordered_set<uint32_t> ins_method_ids_;
    std::unordered_set<uint32_t> ins_literal_ids_;
    std::unordered_set<uint32_t> ins_string_ids_;
    std::unordered_set<uint32_t> module_literals_;
    std::unordered_map<uint32_t, uint32_t> method_class_map_;
    std::unordered_set<uint32_t> class_literal_ids_;
    std::unordered_set<uint32_t> sendable_class_literal_ids_;
    std::unordered_set<uint32_t> module_record_literal_ids_;
    std::unordered_set<uint32_t> lazy_import_literal_ids_;
    std::unordered_set<uint32_t> esmodule_record_ids_;
    std::unordered_set<uint32_t> tla_record_ids_;
    bool has_esmodule_record_class_ {false};
    bool has_tla_class_ {false};
    std::unordered_map<uint32_t, LiteralArrayWalkResult> literal_walk_cache_;
    static constexpr size_t DEFAULT_ARGUMENT_NUMBER = 3;
    static constexpr uint32_t FILE_CONTENT_OFFSET = 12U;
    static constexpr size_t FIRST_INDEX = 0;
    static constexpr size_t SECOND_INDEX = 1;
    static constexpr uint64_t MAX_REGISTER_INDEX = 0xffff;
    static constexpr size_t MAX_LITERAL_WALK_CACHE_SIZE = 0x10000;
    std::unordered_map<const uint8_t *, size_t> instruction_index_map_;

    // NOTE: adjust this version if the check needs to be enabled on another bytecode version.
    static constexpr std::array<uint8_t, panda_file::File::VERSION_SIZE> IC_SLOT_CHECK_VERSION {24, 0, 0, 0};
    static constexpr uint32_t INVALID_IC_SLOT = 0xff;
    static constexpr uint32_t MAX_SLOT_SIZE = 0xffff;
    static constexpr uint32_t EXTEND_SLOT_SIZE = 2;
    static constexpr uint32_t IC_SLOT_16BIT_BASE = 0x100;
    static constexpr uint32_t IC_SLOT_MAX_16BIT = 0xffff;
    static constexpr std::string_view CALL_TYPE_ANNOTATION = "L_ESCallTypeAnnotation;";
    static constexpr std::string_view CALL_TYPE_ELEMENT_NAME = "callType";
    static constexpr uint32_t CALL_TYPE_MASK = 0xf;
    static constexpr uint32_t HAVE_THIS_BIT = 0x1;
    static constexpr uint32_t HAVE_NEW_TARGET_BIT = 0x2;
    static constexpr uint32_t HAVE_FUNC_BIT = 0x8;
    static constexpr uint32_t DEFAULT_CALL_TYPE = 0xf;
    static constexpr std::string_view ES_MODULE_RECORD_CLASS = "L_ESModuleRecord;";
    static constexpr std::string_view HAS_TLA_CLASS = "L_HasTopLevelAwait;";
    static constexpr std::string_view MODULE_RECORD_IDX_FIELD = "moduleRecordIdx";
    static constexpr std::string_view HAS_TOP_LEVEL_AWAIT_FIELD = "hasTopLevelAwait";
    static constexpr std::string_view MODULE_REQUEST_PHASE_IDX_FIELD = "moduleRequestPhaseIdx";
    static constexpr std::string_view MAIN_FUNCTION_NAME = "func_main_0";
    static constexpr uint64_t MAX_REGISTER_ANALYSIS_COMPLEXITY = 0x1000000;
    static constexpr uint64_t MAX_REGISTER_ANALYSIS_TIME_COMPLEXITY = 0x40000000;
};
}  // namespace panda::verifier
#endif
