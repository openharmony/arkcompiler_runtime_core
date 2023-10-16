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

#include "assembler/assembly-program.h"
#include "assembler/assembly-function.h"
#include "assembler/assembly-ins.h"
#include "ins_create_api.h"

#include "runtime/include/runtime.h"
#include "plugins/ets/runtime/ets_coroutine.h"

#include "ets_typeapi_create.h"
#include "ets_typeapi_create_panda_constants.h"
#include "ets_object.h"
#include "ets_array.h"

namespace panda::ets {

constexpr size_t TYPEAPI_CTX_DATA_CCTOR_ARR_REG = 0;

std::string TypeCreatorCtx::AddInitField(uint32_t id, const pandasm::Type &type)
{
    ASSERT(!type.IsPrimitive());
    auto &rec = GetTypeAPICtxDataRecord();
    auto filed_id_for_ins = rec.name;
    pandasm::Field fld {SourceLanguage::ETS};
    fld.name = "f";
    fld.name += std::to_string(id);
    filed_id_for_ins += ".";
    filed_id_for_ins += fld.name;

    fld.type = type;
    fld.metadata->SetAttribute(typeapi_create_consts::ATTR_FINAL);
    fld.metadata->SetAttribute(typeapi_create_consts::ATTR_STATIC);
    rec.field_list.emplace_back(std::move(fld));

    init_fn_.AddInstruction(pandasm::Create_LDAI(id));
    init_fn_.AddInstruction(pandasm::Create_LDARR_OBJ(TYPEAPI_CTX_DATA_CCTOR_ARR_REG));
    init_fn_.AddInstruction(pandasm::Create_CHECKCAST(type.GetPandasmName()));
    init_fn_.AddInstruction(pandasm::Create_STSTATIC_OBJ(filed_id_for_ins));

    return filed_id_for_ins;
}

void TypeCreatorCtx::FlushTypeAPICtxDataRecordsToProgram()
{
    if (ctx_data_record_.name.empty()) {
        return;
    }

    // prevent space leak
    init_fn_.AddInstruction(pandasm::Create_LDA_NULL());
    init_fn_.AddInstruction(pandasm::Create_STSTATIC_OBJ(ctx_data_init_record_.name + ".arr"));
    init_fn_.AddInstruction(pandasm::Create_RETURN_VOID());

    auto name = ctx_data_record_.name;
    prog_.record_table.emplace(std::move(name), std::move(ctx_data_record_));
    name = ctx_data_init_record_.name;
    prog_.record_table.emplace(std::move(name), std::move(ctx_data_init_record_));
    name = init_fn_.name;
    prog_.function_table.emplace(std::move(name), std::move(init_fn_));
}

void TypeCreatorCtx::InitializeCtxDataRecord(EtsCoroutine *coro, VMHandle<EtsArray> &arr)
{
    if (ctx_data_init_record_name_.empty()) {
        return;
    }
    auto name = pandasm::Type(ctx_data_init_record_name_, 0).GetDescriptor();
    auto klass = coro->GetPandaVM()->GetClassLinker()->GetClass(name.c_str());
    ASSERT(!klass->IsInitialized());
    auto linker = Runtime::GetCurrent()->GetClassLinker();
    linker->InitializeClass(coro, klass->GetRuntimeClass());

    auto fld = klass->GetStaticFieldIDByName("arr");
    ASSERT(fld != nullptr);
    auto store_obj = EtsObject::FromCoreType(arr->GetCoreType());
    klass->SetStaticFieldObject(fld, store_obj);
}

pandasm::Record &TypeCreatorCtx::GetTypeAPICtxDataRecord()
{
    if (!ctx_data_record_.name.empty()) {
        return ctx_data_record_;
    }
    ctx_data_record_.name = "TypeAPI$CtxData$";
    static std::atomic_uint ctx_data_next_name {};
    ctx_data_record_.name += std::to_string(ctx_data_next_name++);

    ctx_data_init_record_name_ = ctx_data_record_.name;
    ctx_data_init_record_name_ += "$init";
    ctx_data_init_record_.name = ctx_data_init_record_name_;

    pandasm::Field fld {SourceLanguage::ETS};
    fld.name = "arr";
    fld.type = pandasm::Type {typeapi_create_consts::TYPE_OBJECT, 1};
    fld.metadata->SetAttribute(typeapi_create_consts::ATTR_STATIC);
    AddRefTypeAsExternal(std::string {typeapi_create_consts::TYPE_OBJECT});
    AddRefTypeAsExternal(fld.type.GetName());
    ctx_data_init_record_.field_list.emplace_back(std::move(fld));

    init_fn_.name = ctx_data_record_.name;
    init_fn_.name += '.';
    init_fn_.name += panda_file::GetCctorName(SourceLanguage::ETS);
    init_fn_.metadata->SetAttribute(typeapi_create_consts::ATTR_CCTOR);
    init_fn_.metadata->SetAttribute(typeapi_create_consts::ATTR_STATIC);
    init_fn_.regs_num = 1;
    init_fn_.AddInstruction(pandasm::Create_LDSTATIC_OBJ(ctx_data_init_record_.name + ".arr"));
    init_fn_.AddInstruction(pandasm::Create_STA_OBJ(TYPEAPI_CTX_DATA_CCTOR_ARR_REG));

    return ctx_data_record_;
}

void TypeCreatorCtx::AddRefTypeAsExternal(const std::string &name)
{
    pandasm::Record object_rec {name, SourceLanguage::ETS};
    object_rec.metadata->SetAttribute(typeapi_create_consts::ATTR_EXTERNAL);
    prog_.record_table.emplace(object_rec.name, std::move(object_rec));
}

const std::pair<std::string, std::string> &TypeCreatorCtx::DeclarePrimitive(const std::string &prim_type_name)
{
    if (auto found = primitive_types_ctor_dtor_.find(prim_type_name); found != primitive_types_ctor_dtor_.end()) {
        return found->second;
    }

    auto prim_type = pandasm::Type {prim_type_name, 0};

    std::string object_type_name {typeapi_create_consts::TYPE_BOXED_PREFIX};
    if (prim_type_name == "u1") {
        object_type_name += "Boolean";
    } else if (prim_type_name == "i8") {
        object_type_name += "Byte";
    } else if (prim_type_name == "i16") {
        object_type_name += "Short";
    } else if (prim_type_name == "i32") {
        object_type_name += "Int";
    } else if (prim_type_name == "i64") {
        object_type_name += "Long";
    } else if (prim_type_name == "u16") {
        object_type_name += "Char";
    } else {
        UNREACHABLE();
    }
    AddRefTypeAsExternal(object_type_name);
    auto object_type = pandasm::Type {object_type_name, 0};

    PandasmMethodCreator ctor {object_type_name + "." + panda_file::GetCtorName(SourceLanguage::ETS), this};
    ctor.AddParameter(object_type);
    ctor.AddParameter(prim_type);
    ctor.GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_EXTERNAL);
    ctor.GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_CTOR);
    ctor.Create();

    PandasmMethodCreator unboxed {object_type_name + ".unboxed", this};
    unboxed.AddParameter(object_type);
    unboxed.AddResult(prim_type);
    unboxed.GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_EXTERNAL);
    unboxed.Create();

    return primitive_types_ctor_dtor_
        .emplace(prim_type_name, std::make_pair(ctor.GetFunctionName(), unboxed.GetFunctionName()))
        .first->second;
}

void LambdaTypeCreator::AddParameter(pandasm::Type param)
{
    // auto type = GetPandasmTypeFromDescriptor(creator->ctx, type_descriptor);
    ASSERT(!param.IsVoid());
    auto param_name = param.GetName();
    fn_.params.emplace_back(std::move(param), SourceLanguage::ETS);
    name_ += '-';
    std::replace(param_name.begin(), param_name.end(), '.', '-');
    name_ += param_name;
}

void LambdaTypeCreator::AddResult(const pandasm::Type &type)
{
    fn_.return_type = type;
    name_ += '-';
    auto append_name = type.GetName();
    std::replace(append_name.begin(), append_name.end(), '.', '-');
    name_ += append_name;
}

void LambdaTypeCreator::Create()
{
    ASSERT(!finished_);
    finished_ = true;
    // IMPORTANT: must be synchronized with
    // plugins/ecmascript/es2panda/binder/ETSBinder.cpp
    // ETSBinder::FormLambdaName
    name_ += "-0";

    // TODO(kprokopenko): check if this class exists, otherwise create it
    // it is possible that GetClass will throw an exception, so todo
    // auto panda_name = ClassNameToPandaName(name);
    // EtsCoroutine::GetCurrent()->GetPandaVM()->GetClassLinker()->GetClass(panda_name);

    rec_.name = name_;
    fn_name_ = fn_.name = name_ + ".invoke";
    fn_.params.insert(fn_.params.begin(), pandasm::Function::Parameter(pandasm::Type(name_, 0), SourceLanguage::ETS));
    for (const auto &attr : typeapi_create_consts::ATTR_ABSTRACT_METHOD) {
        fn_.metadata->SetAttribute(attr);
    }
    fn_.metadata->SetAttributeValue(typeapi_create_consts::ATTR_ACCESS, typeapi_create_consts::ATTR_ACCESS_VAL_PUBLIC);
    GetCtx()->Program().record_table.emplace(name_, std::move(rec_));
    GetCtx()->Program().function_table.emplace(fn_name_, std::move(fn_));
}

void PandasmMethodCreator::AddParameter(pandasm::Type param)
{
    name_ += ':';
    name_ += param.GetName();
    fn_.params.emplace_back(std::move(param), SourceLanguage::ETS);
}

void PandasmMethodCreator::AddResult(pandasm::Type type)
{
    fn_.return_type = std::move(type);
}

void PandasmMethodCreator::Create()
{
    ASSERT(!finished_);
    finished_ = true;
    fn_.name = name_;
    auto ok = ctx_->Program().function_table.emplace(name_, std::move(fn_)).second;
    if (!ok) {
        ctx_->AddError("duplicate function " + name_);
    }
}
}  // namespace panda::ets
