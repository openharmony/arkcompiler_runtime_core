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

#include "header_writer.h"
#include "source_language.h"
#include "libpandafile/file-inl.h"
#include "libpandafile/proto_data_accessor-inl.h"
#include "plugins/ets/runtime/napi/ets_mangle.h"
#include <typeinfo>

namespace panda::ets::header_writer {

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
    : input_file_(std::move(input)), output_name_(std::move(output))
{
}

void HeaderWriter::OpenOutput()
{
    output_file_.close();
    output_file_.clear();
    output_file_.open(output_name_.c_str());
}

bool HeaderWriter::PrintFunction()
{
    auto classes_span = input_file_->GetClasses();
    for (auto id : classes_span) {
        if (input_file_->IsExternal(panda_file::File::EntityId(id))) {
            continue;
        }
        panda_file::ClassDataAccessor cda(*input_file_, panda_file::File::EntityId(id));
        if (cda.GetSourceLang() != SourceLanguage::ETS) {
            continue;
        }
        std::vector<std::string> methods;
        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            if (mda.IsNative()) {
                // Founded first native, need to create file and write beginning
                if (!need_header_) {
                    need_header_ = true;
                    CreateHeader();
                }

                std::string class_name = utf::Mutf8AsCString(cda.GetDescriptor());
                if (class_name[0] == 'L') {
                    class_name = class_name.substr(1, class_name.size() - 2);
                }
                PrintPrototype(class_name, mda, CheckOverloading(cda, mda));
            }
        });
    }
    if (need_header_) {
        PrintEnd();
    }
    return need_header_;
}

bool HeaderWriter::CheckOverloading(panda_file::ClassDataAccessor &cda, panda_file::MethodDataAccessor &mda)
{
    bool is_overloaded = false;
    cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda1) {
        if ((mda1.GetName().data == mda.GetName().data) && mda1.IsNative() &&
            !(mda.GetMethodId() == mda1.GetMethodId())) {
            is_overloaded = true;
        }
    });
    return is_overloaded;
}

void HeaderWriter::ProcessProtoType(panda_file::Type type, panda_file::File::EntityId klass, std::string &sign,
                                    std::string &args)
{
    if (type.IsPrimitive()) {
        sign.append(panda_file::Type::GetSignatureByTypeId(type));
        args.append(ConvertEtsPrimitiveTypeToString(ConvertPandaTypeToEtsType(type)));
    } else {
        std::string name = utf::Mutf8AsCString(input_file_->GetStringData(klass).data);
        sign.append(name);
        args.append(ConvertEtsReferenceToString(name));
    }
}

void HeaderWriter::PrintPrototype(const std::string &class_name, panda_file::MethodDataAccessor &mda,
                                  bool is_overloaded)
{
    std::string method_name = utf::Mutf8AsCString(mda.GetName().data);
    std::string second_arg = ConvertEtsReferenceToString("object");
    ;
    if (mda.IsStatic()) {
        second_arg = ConvertEtsReferenceToString("class");
    }
    std::string sign;
    std::string args;
    std::string return_sign;
    std::string return_arg;

    size_t ref_idx = 0;
    panda_file::ProtoDataAccessor pda(*input_file_, mda.GetProtoId());

    auto type = pda.GetReturnType();
    panda_file::File::EntityId class_id;

    if (!type.IsPrimitive()) {
        class_id = pda.GetReferenceType(ref_idx++);
    }

    ProcessProtoType(type, class_id, return_sign, return_arg);

    for (uint32_t idx = 0; idx < pda.GetNumArgs(); ++idx) {
        auto arg_type = pda.GetArgType(idx);
        panda_file::File::EntityId klass_id;
        if (!arg_type.IsPrimitive()) {
            klass_id = pda.GetReferenceType(ref_idx++);
        }
        args.append(", ");
        ProcessProtoType(arg_type, klass_id, sign, args);
    }

    std::string mangled_name = MangleMethodName(class_name, method_name);
    if (is_overloaded) {
        mangled_name = MangleMethodNameWithSignature(mangled_name, sign);
    }

    sign.append(":");
    sign.append(return_sign);

    output_file_ << "/*\n Class:     " << class_name << "\n"
                 << " Method:    " << method_name << "\n"
                 << " Signature: " << sign << "\n */\n"
                 << "ETS_EXPORT " << return_arg << " ETS_CALL " << mangled_name << "(EtsEnv *, " << second_arg << args
                 << ");\n\n";
}

}  // namespace panda::ets::header_writer
