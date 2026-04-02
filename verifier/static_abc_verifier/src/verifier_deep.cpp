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

#ifdef STATIC_ABC_VERIFIER_USE_ARKFILE

#include "verifier.h"

#include <cstring>
#include <fstream>
#include <sstream>

#include "libarkfile/file.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/field_data_accessor-inl.h"
#include "libarkfile/code_data_accessor-inl.h"
#include "libarkfile/literal_data_accessor-inl.h"
#include "libarkfile/bytecode_instruction-inl.h"

namespace ark::static_verifier {

namespace {

struct DeepVerifyContext {
    const panda_file::File &arkFile;
    const AbcHeader *header;
    std::vector<VerifyError> &errors;
    bool valid = true;

    void AddError(ErrorCode code, const std::string &msg)
    {
        errors.push_back({code, msg});
        valid = false;
    }
};

std::unique_ptr<const panda_file::File> OpenArkFile(const std::string &filePath,
                                                    const uint8_t *data, size_t size)
{
    if (!filePath.empty()) {
        return panda_file::File::Open(filePath);
    }
    if (data == nullptr || size == 0) {
        return nullptr;
    }
    char tmpPath[] = "/tmp/sav_deep_XXXXXX";
    int fd = mkstemp(tmpPath);
    if (fd < 0) {
        return nullptr;
    }
    auto written = write(fd, data, size);
    close(fd);
    if (written != static_cast<ssize_t>(size)) {
        unlink(tmpPath);
        return nullptr;
    }
    auto file = panda_file::File::Open(tmpPath);
    unlink(tmpPath);
    return file;
}

// ===================== Literal tag helpers =====================

size_t GetLiteralValueSize(panda_file::LiteralTag tag)
{
    switch (tag) {
        case panda_file::LiteralTag::BOOL:
        case panda_file::LiteralTag::ACCESSOR:
        case panda_file::LiteralTag::NULLVALUE:
            return sizeof(uint8_t);
        case panda_file::LiteralTag::METHODAFFILIATE:
            return sizeof(uint16_t);
        case panda_file::LiteralTag::TAGVALUE:
        case panda_file::LiteralTag::INTEGER:
        case panda_file::LiteralTag::FLOAT:
        case panda_file::LiteralTag::STRING:
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCGENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCMETHOD:
        case panda_file::LiteralTag::LITERALARRAY:
            return sizeof(uint32_t);
        case panda_file::LiteralTag::DOUBLE:
        case panda_file::LiteralTag::BIGINT:
            return sizeof(uint64_t);
        default:
            return 0;
    }
}

bool IsArrayTag(panda_file::LiteralTag tag)
{
    switch (tag) {
        case panda_file::LiteralTag::ARRAY_U1:
        case panda_file::LiteralTag::ARRAY_U8:
        case panda_file::LiteralTag::ARRAY_I8:
        case panda_file::LiteralTag::ARRAY_U16:
        case panda_file::LiteralTag::ARRAY_I16:
        case panda_file::LiteralTag::ARRAY_U32:
        case panda_file::LiteralTag::ARRAY_I32:
        case panda_file::LiteralTag::ARRAY_U64:
        case panda_file::LiteralTag::ARRAY_I64:
        case panda_file::LiteralTag::ARRAY_F32:
        case panda_file::LiteralTag::ARRAY_F64:
        case panda_file::LiteralTag::ARRAY_STRING:
            return true;
        default:
            return false;
    }
}

bool IsKnownLiteralTag(uint8_t tagVal)
{
    auto tag = static_cast<panda_file::LiteralTag>(tagVal);
    return IsArrayTag(tag) || GetLiteralValueSize(tag) > 0;
}

bool IsReferenceTag(panda_file::LiteralTag tag)
{
    switch (tag) {
        case panda_file::LiteralTag::STRING:
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCGENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCMETHOD:
        case panda_file::LiteralTag::LITERALARRAY:
            return true;
        default:
            return false;
    }
}

void SkipLiteralValue(Span<const uint8_t> &sp, size_t valSize)
{
    switch (valSize) {
        case sizeof(uint8_t):
            panda_file::helpers::Read<sizeof(uint8_t)>(&sp);
            break;
        case sizeof(uint16_t):
            panda_file::helpers::Read<sizeof(uint16_t)>(&sp);
            break;
        case sizeof(uint32_t):
            panda_file::helpers::Read<sizeof(uint32_t)>(&sp);
            break;
        case sizeof(uint64_t):
            panda_file::helpers::Read<sizeof(uint64_t)>(&sp);
            break;
        default:
            break;
    }
}

// ===================== Class verification helpers =====================

void VerifySingleClassData(DeepVerifyContext &ctx, uint32_t offset, uint32_t index)
{
    panda_file::File::EntityId classId(offset);
    if (ctx.arkFile.IsExternal(classId)) {
        return;
    }
    try {
        panda_file::ClassDataAccessor cda(ctx.arkFile, classId);

        auto sourceLang = cda.GetSourceLang();
        if (sourceLang.has_value() &&
            !IsValidSourceLang(static_cast<SourceLanguage>(sourceLang.value()))) {
            std::ostringstream oss;
            oss << "Class[" << index << "] at offset 0x" << std::hex << offset
                << " has invalid SourceLang value: " << static_cast<int>(sourceLang.value());
            ctx.AddError(ErrorCode::INVALID_SOURCE_LANG, oss.str());
        }

        uint32_t superOff = cda.GetSuperClassId().GetOffset();
        if (superOff != 0 && superOff >= ctx.header->fileSize) {
            std::ostringstream oss;
            oss << "Class[" << index << "] super class offset 0x"
                << std::hex << superOff << " exceeds file size";
            ctx.AddError(ErrorCode::INVALID_CLASS_DATA, oss.str());
        }

        cda.EnumerateFields([&](panda_file::FieldDataAccessor &fda) {
            uint32_t fieldNameOff = fda.GetNameId().GetOffset();
            if (fieldNameOff >= ctx.header->fileSize) {
                std::ostringstream oss;
                oss << "Class[" << index << "] field name offset 0x"
                    << std::hex << fieldNameOff << " exceeds file size";
                ctx.AddError(ErrorCode::INVALID_CLASS_DATA, oss.str());
            }
        });
    } catch (...) {
        std::ostringstream oss;
        oss << "Class[" << index << "] at offset 0x" << std::hex << offset
            << " failed to parse (corrupted structure)";
        ctx.AddError(ErrorCode::INVALID_CLASS_DATA, oss.str());
    }
}

// ===================== Bytecode verification helpers =====================

void CheckInstructionRegisters(DeepVerifyContext &ctx, BytecodeInstructionSafe &inst,
                               BytecodeInstructionSafe::Format format,
                               uint32_t totalRegs, uint32_t methodOff)
{
    if (totalRegs == 0) {
        return;
    }
    size_t vregIdx = 0;
    while (BytecodeInstructionSafe::HasVReg(format, vregIdx)) {
        uint16_t vreg = inst.GetVReg(vregIdx);
        if (!inst.IsValid()) {
            break;
        }
        if (vreg >= totalRegs) {
            std::ostringstream oss;
            oss << "Register v" << vreg << " >= totalRegs(" << totalRegs
                << ") at bytecode offset " << inst.GetOffset()
                << " in method 0x" << std::hex << methodOff;
            ctx.AddError(ErrorCode::INVALID_REGISTER_INDEX, oss.str());
        }
        ++vregIdx;
    }
}

void CheckInstructionJump(DeepVerifyContext &ctx, BytecodeInstructionSafe &inst,
                          uint32_t codeSize, uint32_t methodOff)
{
    if (!inst.IsValid() || !inst.HasFlag(BytecodeInstructionSafe::Flags::JUMP)) {
        return;
    }
    auto imm = inst.GetImm64();
    if (!inst.IsValid()) {
        return;
    }
    int64_t targetOff = static_cast<int64_t>(inst.GetOffset()) + static_cast<int64_t>(imm);
    if (targetOff < 0 || targetOff >= static_cast<int64_t>(codeSize)) {
        std::ostringstream oss;
        oss << "Jump target offset " << targetOff << " out of code bounds [0, "
            << codeSize << ") at bytecode offset " << inst.GetOffset()
            << " in method 0x" << std::hex << methodOff;
        ctx.AddError(ErrorCode::INVALID_JUMP_TARGET, oss.str());
    }
}

void VerifyBytecodeStream(DeepVerifyContext &ctx, const uint8_t *insns,
                          uint32_t codeSize, uint32_t totalRegs, uint32_t methodOff)
{
    const uint8_t *insnsEnd = insns + codeSize;
    BytecodeInstructionSafe inst(insns, insns, insnsEnd);

    while (inst.IsValid()) {
        if (!inst.IsPrimaryOpcodeValid()) {
            std::ostringstream oss;
            oss << "Invalid opcode 0x" << std::hex
                << static_cast<int>(inst.GetPrimaryOpcode())
                << " at bytecode offset " << std::dec << inst.GetOffset()
                << " in method 0x" << std::hex << methodOff;
            ctx.AddError(ErrorCode::INVALID_BYTECODE, oss.str());
            break;
        }

        auto format = inst.GetFormat();
        if (!inst.IsValid()) {
            break;
        }

        CheckInstructionRegisters(ctx, inst, format, totalRegs, methodOff);
        CheckInstructionJump(ctx, inst, codeSize, methodOff);

        if (!inst.IsValid() || inst.IsLast()) {
            break;
        }
        inst = inst.GetNext();
    }
}

// ===================== Try/Catch verification helpers =====================

void VerifySingleTryBlock(DeepVerifyContext &ctx,
                          panda_file::CodeDataAccessor::TryBlock &tryBlock,
                          uint32_t codeSize, uint32_t methodOff)
{
    uint32_t startPc = tryBlock.GetStartPc();
    uint32_t length = tryBlock.GetLength();

    if (startPc > codeSize) {
        std::ostringstream oss;
        oss << "TryBlock startPc=" << startPc << " exceeds codeSize=" << codeSize
            << " in method 0x" << std::hex << methodOff;
        ctx.AddError(ErrorCode::INVALID_TRY_CATCH, oss.str());
    }
    if (static_cast<uint64_t>(startPc) + length > codeSize) {
        std::ostringstream oss;
        oss << "TryBlock range [" << startPc << ", " << startPc + length
            << ") exceeds codeSize=" << codeSize
            << " in method 0x" << std::hex << methodOff;
        ctx.AddError(ErrorCode::INVALID_TRY_CATCH, oss.str());
    }

    tryBlock.EnumerateCatchBlocks(
        [&](panda_file::CodeDataAccessor::CatchBlock &catchBlock) {
        if (catchBlock.GetHandlerPc() >= codeSize) {
            std::ostringstream oss;
            oss << "CatchBlock handlerPc=" << catchBlock.GetHandlerPc()
                << " >= codeSize=" << codeSize
                << " in method 0x" << std::hex << methodOff;
            ctx.AddError(ErrorCode::INVALID_TRY_CATCH, oss.str());
        }
        return true;
    });
}

void VerifyTryCatchBlocks(DeepVerifyContext &ctx, panda_file::CodeDataAccessor &codeDa,
                          uint32_t codeSize, uint32_t methodOff)
{
    codeDa.EnumerateTryBlocks(
        [&](panda_file::CodeDataAccessor::TryBlock &tryBlock) {
        VerifySingleTryBlock(ctx, tryBlock, codeSize, methodOff);
        return true;
    });
}

// ===================== Method verification helpers =====================

void VerifyMethodCode(DeepVerifyContext &ctx, const panda_file::File &arkFile,
                      panda_file::MethodDataAccessor &mda,
                      panda_file::File::EntityId codeId)
{
    try {
        panda_file::CodeDataAccessor codeDa(arkFile, codeId);
        uint32_t totalRegs = codeDa.GetNumVregs() + codeDa.GetNumArgs();
        uint32_t codeSize = codeDa.GetCodeSize();
        uint32_t methodOff = mda.GetMethodId().GetOffset();

        if (codeSize > 0) {
            VerifyBytecodeStream(ctx, codeDa.GetInstructions(), codeSize, totalRegs, methodOff);
            VerifyTryCatchBlocks(ctx, codeDa, codeSize, methodOff);
        }
    } catch (...) {
        std::ostringstream oss;
        oss << "Failed to parse code data at offset 0x" << std::hex << codeId.GetOffset()
            << " in method 0x" << mda.GetMethodId().GetOffset();
        ctx.AddError(ErrorCode::INVALID_CODE_DATA, oss.str());
    }
}

void VerifySingleMethod(DeepVerifyContext &ctx, const panda_file::File &arkFile,
                        panda_file::MethodDataAccessor &mda)
{
    if (mda.IsExternal()) {
        return;
    }
    uint32_t methodOff = mda.GetMethodId().GetOffset();
    uint32_t nameOff = mda.GetNameId().GetOffset();
    if (nameOff >= ctx.header->fileSize) {
        std::ostringstream oss;
        oss << "Method at offset 0x" << std::hex << methodOff
            << " name offset 0x" << nameOff << " exceeds file size";
        ctx.AddError(ErrorCode::INVALID_METHOD_DATA, oss.str());
    }

    uint32_t protoOff = mda.GetProtoId().GetOffset();
    if (protoOff >= ctx.header->fileSize) {
        std::ostringstream oss;
        oss << "Method at offset 0x" << std::hex << methodOff
            << " proto offset 0x" << protoOff << " exceeds file size";
        ctx.AddError(ErrorCode::INVALID_METHOD_DATA, oss.str());
    }

    auto codeId = mda.GetCodeId();
    if (!codeId.has_value()) {
        return;
    }
    if (codeId->GetOffset() >= ctx.header->fileSize) {
        std::ostringstream oss;
        oss << "Method at offset 0x" << std::hex << methodOff
            << " code offset 0x" << codeId->GetOffset() << " exceeds file size";
        ctx.AddError(ErrorCode::INVALID_CODE_DATA, oss.str());
        return;
    }
    VerifyMethodCode(ctx, arkFile, mda, *codeId);
}

void VerifyClassMethods(DeepVerifyContext &ctx, const panda_file::File &arkFile,
                        uint32_t classOffset, uint32_t classIndex)
{
    panda_file::File::EntityId classId(classOffset);
    if (arkFile.IsExternal(classId)) {
        return;
    }
    try {
        panda_file::ClassDataAccessor cda(arkFile, classId);
        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            VerifySingleMethod(ctx, arkFile, mda);
        });
    } catch (...) {
        std::ostringstream oss;
        oss << "Failed to parse class at offset 0x" << std::hex << classOffset
            << " during method/code verification";
        ctx.AddError(ErrorCode::INVALID_CLASS_DATA, oss.str());
    }
}

// ===================== Literal array verification helpers =====================

bool VerifyLiteralItem(DeepVerifyContext &ctx, Span<const uint8_t> &sp,
                       uint32_t index, uint32_t itemIdx)
{
    if (sp.Size() < sizeof(uint8_t)) {
        std::ostringstream oss;
        oss << "LiteralArray[" << index << "] truncated at item " << itemIdx;
        ctx.AddError(ErrorCode::INVALID_LITERAL_ARRAY_DATA, oss.str());
        return false;
    }

    uint8_t tagByte = panda_file::helpers::Read<sizeof(uint8_t)>(&sp);
    if (!IsKnownLiteralTag(tagByte)) {
        std::ostringstream oss;
        oss << "LiteralArray[" << index << "] item " << itemIdx
            << " has unknown tag 0x" << std::hex << static_cast<int>(tagByte);
        ctx.AddError(ErrorCode::INVALID_LITERAL_ARRAY_DATA, oss.str());
        return false;
    }

    auto tag = static_cast<panda_file::LiteralTag>(tagByte);
    if (IsArrayTag(tag)) {
        return false;
    }

    size_t valSize = GetLiteralValueSize(tag);
    if (sp.Size() < valSize) {
        std::ostringstream oss;
        oss << "LiteralArray[" << index << "] truncated value at item " << itemIdx
            << " (need " << valSize << " bytes, have " << sp.Size() << ")";
        ctx.AddError(ErrorCode::INVALID_LITERAL_ARRAY_DATA, oss.str());
        return false;
    }

    if (IsReferenceTag(tag)) {
        uint32_t refOff = panda_file::helpers::Read<sizeof(uint32_t)>(&sp);
        if (refOff >= ctx.header->fileSize) {
            std::ostringstream oss;
            oss << "LiteralArray[" << index << "] item " << itemIdx
                << " reference offset 0x" << std::hex << refOff << " exceeds file size";
            ctx.AddError(ErrorCode::INVALID_LITERAL_ARRAY_DATA, oss.str());
        }
    } else {
        SkipLiteralValue(sp, valSize);
    }
    return true;
}

void VerifySingleLiteralArray(DeepVerifyContext &ctx, const panda_file::File &arkFile,
                              uint32_t litOffset, uint32_t index)
{
    try {
        auto sp = arkFile.GetSpanFromId(panda_file::File::EntityId(litOffset));
        if (sp.data() == nullptr || sp.Size() < sizeof(uint32_t)) {
            std::ostringstream oss;
            oss << "LiteralArray[" << index << "] at offset 0x"
                << std::hex << litOffset << " has invalid span";
            ctx.AddError(ErrorCode::INVALID_LITERAL_ARRAY_DATA, oss.str());
            return;
        }

        uint32_t numItems = panda_file::helpers::Read<sizeof(uint32_t)>(&sp);
        for (uint32_t j = 0; j < numItems; j += 2U) {
            if (!VerifyLiteralItem(ctx, sp, index, j)) {
                break;
            }
        }
    } catch (...) {
        std::ostringstream oss;
        oss << "LiteralArray[" << index << "] at offset 0x" << std::hex << litOffset
            << " failed to parse (corrupted structure)";
        ctx.AddError(ErrorCode::INVALID_LITERAL_ARRAY_DATA, oss.str());
    }
}

}  // namespace

// ===================== Public methods =====================

bool StaticAbcVerifier::VerifyClassesDeep()
{
    const auto *header = file_->GetHeader();
    if (header->numClasses == 0 || header->numIndexes == 0) {
        return true;
    }
    auto arkFile = OpenArkFile(filePath_, file_->GetBase(), file_->GetFileSize());
    if (arkFile == nullptr) {
        AddError(ErrorCode::INVALID_CLASS_DATA, "Failed to open ABC file via libarkfile");
        return false;
    }
    const auto *classIdx = file_->GetClassIndex();
    if (classIdx == nullptr) {
        AddError(ErrorCode::INVALID_CLASS_DATA, "Class index not available");
        return false;
    }

    DeepVerifyContext ctx {*arkFile, header, errors_};
    for (uint32_t i = 0; i < header->numClasses; ++i) {
        VerifySingleClassData(ctx, classIdx[i], i);
    }
    return ctx.valid;
}

bool StaticAbcVerifier::VerifyMethodsAndCode()
{
    const auto *header = file_->GetHeader();
    if (header->numClasses == 0 || header->numIndexes == 0) {
        return true;
    }
    auto arkFile = OpenArkFile(filePath_, file_->GetBase(), file_->GetFileSize());
    if (arkFile == nullptr) {
        AddError(ErrorCode::INVALID_METHOD_DATA, "Failed to open ABC file via libarkfile");
        return false;
    }
    const auto *classIdx = file_->GetClassIndex();
    if (classIdx == nullptr) {
        return false;
    }

    DeepVerifyContext ctx {*arkFile, header, errors_};
    for (uint32_t ci = 0; ci < header->numClasses; ++ci) {
        VerifyClassMethods(ctx, *arkFile, classIdx[ci], ci);
    }
    return ctx.valid;
}

bool StaticAbcVerifier::VerifyLiteralArraysDeep()
{
    const auto *header = file_->GetHeader();
    if (header->numLiteralarrays == 0) {
        return true;
    }
    auto arkFile = OpenArkFile(filePath_, file_->GetBase(), file_->GetFileSize());
    if (arkFile == nullptr) {
        AddError(ErrorCode::INVALID_LITERAL_ARRAY_DATA, "Failed to open ABC file via libarkfile");
        return false;
    }
    const auto *litIdx = file_->GetLiteralArrayIndex();
    if (litIdx == nullptr) {
        AddError(ErrorCode::INVALID_LITERAL_ARRAY_DATA, "Literal array index not available");
        return false;
    }

    DeepVerifyContext ctx {*arkFile, header, errors_};
    for (uint32_t i = 0; i < header->numLiteralarrays; ++i) {
        VerifySingleLiteralArray(ctx, *arkFile, litIdx[i], i);
    }
    return ctx.valid;
}

}  // namespace ark::static_verifier

#endif  // STATIC_ABC_VERIFIER_USE_ARKFILE
