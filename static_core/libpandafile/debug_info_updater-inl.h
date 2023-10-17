/**
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

#ifndef LIBPANDAFILE_DEBUG_INFO_UPDATER_INL_H_
#define LIBPANDAFILE_DEBUG_INFO_UPDATER_INL_H_

#include <type_traits>
#include "debug_data_accessor.h"
#include "debug_data_accessor-inl.h"
#include "line_number_program.h"

namespace panda::panda_file {

/* T is a CRTP with following methods:
 *   StringItem *GetOrCreateStringItem(const std::string &s)
 *   BaseClassItem *GetType(File::EntityId type_id, const std::string &type_name)
 */
template <typename T>
class DebugInfoUpdater {
public:
    explicit DebugInfoUpdater(const File *file) : file_(file) {}

    void Scrap(File::EntityId debug_info_id)
    {
        DebugInfoDataAccessor debug_acc(*file_, debug_info_id);
        const uint8_t *program = debug_acc.GetLineNumberProgram();
        auto size = file_->GetSpanFromId(file_->GetIdFromPointer(program)).size();
        auto opcode_sp = Span(program, size);

        size_t i = 0;
        LineNumberProgramItem::Opcode opcode;
        panda_file::LineProgramState state(*file_, File::EntityId(0), debug_acc.GetLineStart(),
                                           debug_acc.GetConstantPool());
        while ((opcode = LineNumberProgramItem::Opcode(opcode_sp[i++])) !=
               LineNumberProgramItem::Opcode::END_SEQUENCE) {
            switch (opcode) {
                case LineNumberProgramItem::Opcode::ADVANCE_PC:
                case LineNumberProgramItem::Opcode::ADVANCE_LINE:
                case LineNumberProgramItem::Opcode::SET_PROLOGUE_END:
                case LineNumberProgramItem::Opcode::SET_EPILOGUE_BEGIN: {
                    break;
                }
                case LineNumberProgramItem::Opcode::START_LOCAL: {
                    [[maybe_unused]] int32_t reg_number;
                    size_t n;
                    bool is_full;
                    std::tie(reg_number, n, is_full) = leb128::DecodeSigned<int32_t>(&opcode_sp[i]);
                    LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                    i += n;

                    auto name_id = File::EntityId(state.ReadULeb128());
                    std::string name = utf::Mutf8AsCString(file_->GetStringData(name_id).data);
                    This()->GetOrCreateStringItem(name);

                    auto type_id = File::EntityId(state.ReadULeb128());
                    std::string type_name = utf::Mutf8AsCString(file_->GetStringData(type_id).data);
                    This()->GetType(type_id, type_name);
                    break;
                }
                case LineNumberProgramItem::Opcode::START_LOCAL_EXTENDED: {
                    [[maybe_unused]] int32_t reg_number;
                    size_t n;
                    bool is_full;
                    std::tie(reg_number, n, is_full) = leb128::DecodeSigned<int32_t>(&opcode_sp[i]);
                    LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                    i += n;

                    auto name_id = File::EntityId(state.ReadULeb128());
                    std::string name = utf::Mutf8AsCString(file_->GetStringData(name_id).data);
                    This()->GetOrCreateStringItem(name);

                    auto type_id = File::EntityId(state.ReadULeb128());
                    std::string type_name = utf::Mutf8AsCString(file_->GetStringData(type_id).data);
                    This()->GetType(type_id, type_name);

                    auto type_signature_id = File::EntityId(state.ReadULeb128());
                    std::string type_signature = utf::Mutf8AsCString(file_->GetStringData(type_signature_id).data);
                    This()->GetOrCreateStringItem(type_signature);
                    break;
                }
                case LineNumberProgramItem::Opcode::END_LOCAL:
                case LineNumberProgramItem::Opcode::RESTART_LOCAL: {
                    [[maybe_unused]] int32_t reg_number;
                    size_t n;
                    bool is_full;
                    std::tie(reg_number, n, is_full) = leb128::DecodeSigned<int32_t>(&opcode_sp[i]);
                    LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                    i += n;
                    break;
                }
                case LineNumberProgramItem::Opcode::SET_FILE: {
                    auto source_file_id = File::EntityId(state.ReadULeb128());
                    std::string source_file = utf::Mutf8AsCString(file_->GetStringData(source_file_id).data);
                    This()->GetOrCreateStringItem(source_file);
                    break;
                }
                case LineNumberProgramItem::Opcode::SET_SOURCE_CODE: {
                    auto source_code_id = File::EntityId(state.ReadULeb128());
                    std::string source_code = utf::Mutf8AsCString(file_->GetStringData(source_code_id).data);
                    This()->GetOrCreateStringItem(source_code);
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }

    void Emit(DebugInfoItem *debug_info_item, File::EntityId debug_info_id)
    {
        auto *lnp_item = debug_info_item->GetLineNumberProgram();
        if (!lnp_item->GetData().empty()) {
            return;
        }

        DebugInfoDataAccessor debug_acc(*file_, debug_info_id);
        const uint8_t *program = debug_acc.GetLineNumberProgram();
        auto size = file_->GetSpanFromId(file_->GetIdFromPointer(program)).size();
        auto opcode_sp = Span(program, size);

        size_t i = 0;
        LineNumberProgramItem::Opcode opcode;
        panda_file::LineProgramState state(*file_, File::EntityId(0), debug_acc.GetLineStart(),
                                           debug_acc.GetConstantPool());
        while ((opcode = LineNumberProgramItem::Opcode(opcode_sp[i++])) !=
               LineNumberProgramItem::Opcode::END_SEQUENCE) {
            switch (opcode) {
                case LineNumberProgramItem::Opcode::ADVANCE_PC: {
                    lnp_item->EmitAdvancePc(debug_info_item->GetConstantPool(), state.ReadULeb128());
                    break;
                }
                case LineNumberProgramItem::Opcode::ADVANCE_LINE: {
                    lnp_item->EmitAdvanceLine(debug_info_item->GetConstantPool(), state.ReadSLeb128());
                    break;
                }
                case LineNumberProgramItem::Opcode::START_LOCAL: {
                    auto [reg_number, n, is_full] = leb128::DecodeSigned<int32_t>(&opcode_sp[i]);
                    LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                    i += n;

                    auto name_id = File::EntityId(state.ReadULeb128());
                    std::string name = utf::Mutf8AsCString(file_->GetStringData(name_id).data);
                    auto *name_item = This()->GetOrCreateStringItem(name);

                    auto type_id = File::EntityId(state.ReadULeb128());
                    std::string type_name = utf::Mutf8AsCString(file_->GetStringData(type_id).data);
                    auto *type_item = This()->GetType(type_id, type_name);

                    lnp_item->EmitStartLocal(debug_info_item->GetConstantPool(), reg_number, name_item,
                                             type_item->GetNameItem());
                    break;
                }
                case LineNumberProgramItem::Opcode::START_LOCAL_EXTENDED: {
                    auto [reg_number, n, is_full] = leb128::DecodeSigned<int32_t>(&opcode_sp[i]);
                    LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                    i += n;

                    auto name_id = File::EntityId(state.ReadULeb128());
                    std::string name = utf::Mutf8AsCString(file_->GetStringData(name_id).data);
                    auto *name_item = This()->GetOrCreateStringItem(name);

                    auto type_id = File::EntityId(state.ReadULeb128());
                    std::string type_name = utf::Mutf8AsCString(file_->GetStringData(type_id).data);
                    auto *type_item = This()->GetType(type_id, type_name);

                    auto type_signature_id = File::EntityId(state.ReadULeb128());
                    std::string type_signature = utf::Mutf8AsCString(file_->GetStringData(type_signature_id).data);
                    auto *type_signature_item = This()->GetOrCreateStringItem(type_signature);

                    lnp_item->EmitStartLocalExtended(debug_info_item->GetConstantPool(), reg_number, name_item,
                                                     type_item->GetNameItem(), type_signature_item);
                    break;
                }
                case LineNumberProgramItem::Opcode::END_LOCAL: {
                    auto [reg_number, n, is_full] = leb128::DecodeSigned<int32_t>(&opcode_sp[i]);
                    LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                    i += n;

                    lnp_item->EmitEndLocal(reg_number);
                    break;
                }
                case LineNumberProgramItem::Opcode::RESTART_LOCAL: {
                    auto [reg_number, n, is_full] = leb128::DecodeSigned<int32_t>(&opcode_sp[i]);
                    LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                    i += n;

                    lnp_item->EmitRestartLocal(reg_number);
                    break;
                }
                case LineNumberProgramItem::Opcode::SET_PROLOGUE_END: {
                    lnp_item->EmitPrologEnd();
                    break;
                }
                case LineNumberProgramItem::Opcode::SET_EPILOGUE_BEGIN: {
                    lnp_item->EmitEpilogBegin();
                    break;
                }
                case LineNumberProgramItem::Opcode::SET_FILE: {
                    auto source_file_id = File::EntityId(state.ReadULeb128());
                    std::string source_file = utf::Mutf8AsCString(file_->GetStringData(source_file_id).data);
                    auto *source_file_item = This()->GetOrCreateStringItem(source_file);
                    lnp_item->EmitSetFile(debug_info_item->GetConstantPool(), source_file_item);
                    break;
                }
                case LineNumberProgramItem::Opcode::SET_SOURCE_CODE: {
                    auto source_code_id = File::EntityId(state.ReadULeb128());
                    std::string source_code = utf::Mutf8AsCString(file_->GetStringData(source_code_id).data);
                    auto *source_code_item = This()->GetOrCreateStringItem(source_code);
                    lnp_item->EmitSetSourceCode(debug_info_item->GetConstantPool(), source_code_item);
                    break;
                }
                case LineNumberProgramItem::Opcode::SET_COLUMN: {
                    lnp_item->EmitColumn(debug_info_item->GetConstantPool(), 0, state.ReadULeb128());
                    break;
                }
                default: {
                    auto opcode_value = static_cast<uint8_t>(opcode);
                    auto adjust_opcode = opcode_value - LineNumberProgramItem::OPCODE_BASE;
                    uint32_t pc_diff = adjust_opcode / LineNumberProgramItem::LINE_RANGE;
                    int32_t line_diff =
                        adjust_opcode % LineNumberProgramItem::LINE_RANGE + LineNumberProgramItem::LINE_BASE;
                    lnp_item->EmitSpecialOpcode(pc_diff, line_diff);
                    break;
                }
            }
        }
        lnp_item->EmitEnd();
    }

protected:
    const File *GetFile() const
    {
        return file_;
    }

private:
    const File *file_;

    T *This()
    {
        static_assert(std::is_base_of_v<DebugInfoUpdater, T>);
        return static_cast<T *>(this);
    }
};
}  // namespace panda::panda_file

#endif  // LIBPANDAFILE_DEBUG_INFO_UPDATER_INL_H_
