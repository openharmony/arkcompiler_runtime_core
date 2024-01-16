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

namespace ark::panda_file {

/* T is a CRTP with following methods:
 *   StringItem *GetOrCreateStringItem(const std::string &s)
 *   BaseClassItem *GetType(File::EntityId type_id, const std::string &type_name)
 */
template <typename T>
class DebugInfoUpdater {
public:
    explicit DebugInfoUpdater(const File *file) : file_(file) {}

    void Scrap(File::EntityId debugInfoId)
    {
        DebugInfoDataAccessor debugAcc(*file_, debugInfoId);
        const uint8_t *program = debugAcc.GetLineNumberProgram();
        auto size = file_->GetSpanFromId(file_->GetIdFromPointer(program)).size();
        auto opcodeSp = Span(program, size);

        size_t i = 0;
        LineNumberProgramItem::Opcode opcode;
        panda_file::LineProgramState state(*file_, File::EntityId(0), debugAcc.GetLineStart(),
                                           debugAcc.GetConstantPool());
        while ((opcode = LineNumberProgramItem::Opcode(opcodeSp[i++])) != LineNumberProgramItem::Opcode::END_SEQUENCE) {
            switch (opcode) {
                case LineNumberProgramItem::Opcode::ADVANCE_PC:
                case LineNumberProgramItem::Opcode::ADVANCE_LINE:
                case LineNumberProgramItem::Opcode::SET_PROLOGUE_END:
                case LineNumberProgramItem::Opcode::SET_EPILOGUE_BEGIN: {
                    break;
                }
                case LineNumberProgramItem::Opcode::START_LOCAL: {
                    [[maybe_unused]] int32_t regNumber;
                    size_t n;
                    bool isFull;
                    std::tie(regNumber, n, isFull) = leb128::DecodeSigned<int32_t>(&opcodeSp[i]);
                    LOG_IF(!isFull, FATAL, COMMON) << "Cannot read a register number";
                    i += n;

                    auto nameId = File::EntityId(state.ReadULeb128());
                    std::string name = utf::Mutf8AsCString(file_->GetStringData(nameId).data);
                    This()->GetOrCreateStringItem(name);

                    auto typeId = File::EntityId(state.ReadULeb128());
                    std::string typeName = utf::Mutf8AsCString(file_->GetStringData(typeId).data);
                    This()->GetType(typeId, typeName);
                    break;
                }
                case LineNumberProgramItem::Opcode::START_LOCAL_EXTENDED: {
                    [[maybe_unused]] int32_t regNumber;
                    size_t n;
                    bool isFull;
                    std::tie(regNumber, n, isFull) = leb128::DecodeSigned<int32_t>(&opcodeSp[i]);
                    LOG_IF(!isFull, FATAL, COMMON) << "Cannot read a register number";
                    i += n;

                    auto nameId = File::EntityId(state.ReadULeb128());
                    std::string name = utf::Mutf8AsCString(file_->GetStringData(nameId).data);
                    This()->GetOrCreateStringItem(name);

                    auto typeId = File::EntityId(state.ReadULeb128());
                    std::string typeName = utf::Mutf8AsCString(file_->GetStringData(typeId).data);
                    This()->GetType(typeId, typeName);

                    auto typeSignatureId = File::EntityId(state.ReadULeb128());
                    std::string typeSignature = utf::Mutf8AsCString(file_->GetStringData(typeSignatureId).data);
                    This()->GetOrCreateStringItem(typeSignature);
                    break;
                }
                case LineNumberProgramItem::Opcode::END_LOCAL:
                case LineNumberProgramItem::Opcode::RESTART_LOCAL: {
                    [[maybe_unused]] int32_t regNumber;
                    size_t n;
                    bool isFull;
                    std::tie(regNumber, n, isFull) = leb128::DecodeSigned<int32_t>(&opcodeSp[i]);
                    LOG_IF(!isFull, FATAL, COMMON) << "Cannot read a register number";
                    i += n;
                    break;
                }
                case LineNumberProgramItem::Opcode::SET_FILE: {
                    auto sourceFileId = File::EntityId(state.ReadULeb128());
                    std::string sourceFile = utf::Mutf8AsCString(file_->GetStringData(sourceFileId).data);
                    This()->GetOrCreateStringItem(sourceFile);
                    break;
                }
                case LineNumberProgramItem::Opcode::SET_SOURCE_CODE: {
                    auto sourceCodeId = File::EntityId(state.ReadULeb128());
                    std::string sourceCode = utf::Mutf8AsCString(file_->GetStringData(sourceCodeId).data);
                    This()->GetOrCreateStringItem(sourceCode);
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }

    void Emit(DebugInfoItem *debugInfoItem, File::EntityId debugInfoId)
    {
        auto *lnpItem = debugInfoItem->GetLineNumberProgram();
        if (!lnpItem->GetData().empty()) {
            return;
        }

        DebugInfoDataAccessor debugAcc(*file_, debugInfoId);
        const uint8_t *program = debugAcc.GetLineNumberProgram();
        auto size = file_->GetSpanFromId(file_->GetIdFromPointer(program)).size();
        auto opcodeSp = Span(program, size);

        size_t i = 0;
        LineNumberProgramItem::Opcode opcode;
        panda_file::LineProgramState state(*file_, File::EntityId(0), debugAcc.GetLineStart(),
                                           debugAcc.GetConstantPool());
        while ((opcode = LineNumberProgramItem::Opcode(opcodeSp[i++])) != LineNumberProgramItem::Opcode::END_SEQUENCE) {
            switch (opcode) {
                case LineNumberProgramItem::Opcode::ADVANCE_PC: {
                    lnpItem->EmitAdvancePc(debugInfoItem->GetConstantPool(), state.ReadULeb128());
                    break;
                }
                case LineNumberProgramItem::Opcode::ADVANCE_LINE: {
                    lnpItem->EmitAdvanceLine(debugInfoItem->GetConstantPool(), state.ReadSLeb128());
                    break;
                }
                case LineNumberProgramItem::Opcode::START_LOCAL: {
                    auto [reg_number, n, is_full] = leb128::DecodeSigned<int32_t>(&opcodeSp[i]);
                    LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                    i += n;

                    auto nameId = File::EntityId(state.ReadULeb128());
                    std::string name = utf::Mutf8AsCString(file_->GetStringData(nameId).data);
                    auto *nameItem = This()->GetOrCreateStringItem(name);

                    auto typeId = File::EntityId(state.ReadULeb128());
                    std::string typeName = utf::Mutf8AsCString(file_->GetStringData(typeId).data);
                    auto *typeItem = This()->GetType(typeId, typeName);

                    lnpItem->EmitStartLocal(debugInfoItem->GetConstantPool(), reg_number, nameItem,
                                            typeItem->GetNameItem());
                    break;
                }
                case LineNumberProgramItem::Opcode::START_LOCAL_EXTENDED: {
                    auto [reg_number, n, is_full] = leb128::DecodeSigned<int32_t>(&opcodeSp[i]);
                    LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                    i += n;

                    auto nameId = File::EntityId(state.ReadULeb128());
                    std::string name = utf::Mutf8AsCString(file_->GetStringData(nameId).data);
                    auto *nameItem = This()->GetOrCreateStringItem(name);

                    auto typeId = File::EntityId(state.ReadULeb128());
                    std::string typeName = utf::Mutf8AsCString(file_->GetStringData(typeId).data);
                    auto *typeItem = This()->GetType(typeId, typeName);

                    auto typeSignatureId = File::EntityId(state.ReadULeb128());
                    std::string typeSignature = utf::Mutf8AsCString(file_->GetStringData(typeSignatureId).data);
                    auto *typeSignatureItem = This()->GetOrCreateStringItem(typeSignature);

                    lnpItem->EmitStartLocalExtended(debugInfoItem->GetConstantPool(), reg_number, nameItem,
                                                    typeItem->GetNameItem(), typeSignatureItem);
                    break;
                }
                case LineNumberProgramItem::Opcode::END_LOCAL: {
                    auto [reg_number, n, is_full] = leb128::DecodeSigned<int32_t>(&opcodeSp[i]);
                    LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                    i += n;

                    lnpItem->EmitEndLocal(reg_number);
                    break;
                }
                case LineNumberProgramItem::Opcode::RESTART_LOCAL: {
                    auto [reg_number, n, is_full] = leb128::DecodeSigned<int32_t>(&opcodeSp[i]);
                    LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                    i += n;

                    lnpItem->EmitRestartLocal(reg_number);
                    break;
                }
                case LineNumberProgramItem::Opcode::SET_PROLOGUE_END: {
                    lnpItem->EmitPrologEnd();
                    break;
                }
                case LineNumberProgramItem::Opcode::SET_EPILOGUE_BEGIN: {
                    lnpItem->EmitEpilogBegin();
                    break;
                }
                case LineNumberProgramItem::Opcode::SET_FILE: {
                    auto sourceFileId = File::EntityId(state.ReadULeb128());
                    std::string sourceFile = utf::Mutf8AsCString(file_->GetStringData(sourceFileId).data);
                    auto *sourceFileItem = This()->GetOrCreateStringItem(sourceFile);
                    lnpItem->EmitSetFile(debugInfoItem->GetConstantPool(), sourceFileItem);
                    break;
                }
                case LineNumberProgramItem::Opcode::SET_SOURCE_CODE: {
                    auto sourceCodeId = File::EntityId(state.ReadULeb128());
                    std::string sourceCode = utf::Mutf8AsCString(file_->GetStringData(sourceCodeId).data);
                    auto *sourceCodeItem = This()->GetOrCreateStringItem(sourceCode);
                    lnpItem->EmitSetSourceCode(debugInfoItem->GetConstantPool(), sourceCodeItem);
                    break;
                }
                case LineNumberProgramItem::Opcode::SET_COLUMN: {
                    lnpItem->EmitColumn(debugInfoItem->GetConstantPool(), 0, state.ReadULeb128());
                    break;
                }
                default: {
                    auto opcodeValue = static_cast<uint8_t>(opcode);
                    ASSERT(opcodeValue >= LineNumberProgramItem::OPCODE_BASE);
                    uint32_t adjustOpcode = opcodeValue - LineNumberProgramItem::OPCODE_BASE;
                    uint32_t pcDiff = adjustOpcode / LineNumberProgramItem::LINE_RANGE;
                    int32_t lineDiff =
                        adjustOpcode % LineNumberProgramItem::LINE_RANGE + LineNumberProgramItem::LINE_BASE;
                    lnpItem->EmitSpecialOpcode(pcDiff, lineDiff);
                    break;
                }
            }
        }
        lnpItem->EmitEnd();
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
}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_DEBUG_INFO_UPDATER_INL_H_
