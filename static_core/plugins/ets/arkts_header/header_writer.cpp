/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "header_writer.h"
#include "source_language.h"
#include "libpandafile/file-inl.h"
#include "libpandafile/proto_data_accessor-inl.h"
#include "plugins/ets/runtime/napi/ets_mangle.h"
#include <typeinfo>

namespace ark::ets::header_writer {

std::string ConvertEtsReferenceToString(const std::string &name)
{
    if (name == "Lstd/core/String;") {
        return "ets_string";
    }
    if (name == "class") {
        return "ets_class";
    }
    return "ets_object";
}

HeaderWriter::HeaderWriter(std::unique_ptr<const panda_file::File> &&input, std::string output)
    : inputFile_(std::move(input)), outputName_(std::move(output))
{
}

void HeaderWriter::OpenOutput()
{
    outputFile_.close();
    outputFile_.clear();
    outputFile_.open(outputName_.c_str());
}

bool HeaderWriter::PrintFunction()
{
    auto classesSpan = inputFile_->GetClasses();
    for (auto id : classesSpan) {
        if (inputFile_->IsExternal(panda_file::File::EntityId(id))) {
            continue;
        }
        panda_file::ClassDataAccessor cda(*inputFile_, panda_file::File::EntityId(id));
        if (cda.GetSourceLang() != SourceLanguage::ETS) {
            continue;
        }
        std::vector<std::string> methods;
        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            if (mda.IsNative()) {
                // Founded first native, need to create file and write beginning
                if (!needHeader_) {
                    needHeader_ = true;
                    CreateHeader();
                }

                std::string className = utf::Mutf8AsCString(cda.GetDescriptor());
                if (className[0] == 'L') {
                    className = className.substr(1, className.size() - 2U);
                }
                PrintPrototype(className, mda, CheckOverloading(cda, mda));
            }
        });
    }
    if (needHeader_) {
        PrintEnd();
    }
    return needHeader_;
}

bool HeaderWriter::CheckOverloading(panda_file::ClassDataAccessor &cda, panda_file::MethodDataAccessor &mda)
{
    bool isOverloaded = false;
    cda.EnumerateMethods([&mda, &isOverloaded](panda_file::MethodDataAccessor &mda1) {
        if ((mda1.GetName().data == mda.GetName().data) && mda1.IsNative() &&
            !(mda.GetMethodId() == mda1.GetMethodId())) {
            isOverloaded = true;
        }
    });
    return isOverloaded;
}

void HeaderWriter::ProcessProtoType(panda_file::Type type, panda_file::File::EntityId klass, std::string &sign,
                                    std::string &args)
{
    if (type.IsPrimitive()) {
        sign.append(panda_file::Type::GetSignatureByTypeId(type));
        args.append(ConvertEtsPrimitiveTypeToString(ConvertPandaTypeToEtsType(type)));
    } else {
        std::string name = utf::Mutf8AsCString(inputFile_->GetStringData(klass).data);
        sign.append(name);
        args.append(ConvertEtsReferenceToString(name));
    }
}

void HeaderWriter::PrintPrototype(const std::string &className, panda_file::MethodDataAccessor &mda, bool isOverloaded)
{
    std::string methodName = utf::Mutf8AsCString(mda.GetName().data);
    std::string secondArg = ConvertEtsReferenceToString("object");
    ;
    if (mda.IsStatic()) {
        secondArg = ConvertEtsReferenceToString("class");
    }
    std::string sign;
    std::string args;
    std::string returnSign;
    std::string returnArg;

    size_t refIdx = 0;
    panda_file::ProtoDataAccessor pda(*inputFile_, mda.GetProtoId());

    auto type = pda.GetReturnType();
    panda_file::File::EntityId classId;

    if (!type.IsPrimitive()) {
        classId = pda.GetReferenceType(refIdx++);
    }

    ProcessProtoType(type, classId, returnSign, returnArg);

    for (uint32_t idx = 0; idx < pda.GetNumArgs(); ++idx) {
        auto argType = pda.GetArgType(idx);
        panda_file::File::EntityId klassId;
        if (!argType.IsPrimitive()) {
            klassId = pda.GetReferenceType(refIdx++);
        }
        args.append(", ");
        ProcessProtoType(argType, klassId, sign, args);
    }

    std::string mangledName = MangleMethodName(className, methodName);
    if (isOverloaded) {
        mangledName = MangleMethodNameWithSignature(mangledName, sign);
    }

    sign.append(":");
    sign.append(returnSign);

    outputFile_ << "/*\n Class:     " << className << "\n"
                << " Method:    " << methodName << "\n"
                << " Signature: " << sign << "\n */\n"
                << "ETS_EXPORT " << returnArg << " ETS_CALL " << mangledName << "(EtsEnv *, " << secondArg << args
                << ");\n\n";
}

}  // namespace ark::ets::header_writer
