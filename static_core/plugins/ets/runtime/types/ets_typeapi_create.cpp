/*
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

#include <cstdio>
#include <string>

#include "assembler/assembly-program.h"
#include "assembler/assembly-function.h"
#include "assembler/assembly-ins.h"
#include "assembly-type.h"
#include "ins_create_api.h"

#include "runtime/include/runtime.h"
#include "plugins/ets/runtime/ets_coroutine.h"

#include "ets_typeapi_create.h"
#include "ets_object.h"
#include "ets_array.h"

namespace ark::ets {

constexpr size_t TYPEAPI_CTX_DATA_CCTOR_ARR_REG = 0;

std::string TypeCreatorCtx::AddInitField(uint32_t id, const pandasm::Type &type)
{
    ASSERT(!type.IsPrimitive());
    auto &rec = GetTypeAPICtxDataRecord();
    auto filedIdForIns = rec.name;
    pandasm::Field fld {SourceLanguage::ETS};
    fld.name = "f";
    fld.name += std::to_string(id);
    filedIdForIns += ".";
    filedIdForIns += fld.name;

    fld.type = type;
    fld.metadata->SetAttribute(typeapi_create_consts::ATTR_FINAL);
    fld.metadata->SetAttribute(typeapi_create_consts::ATTR_STATIC);
    rec.fieldList.emplace_back(std::move(fld));

    ctxDataRecordCctor_.AddInstruction(pandasm::Create_LDAI(id));
    ctxDataRecordCctor_.AddInstruction(pandasm::Create_LDARR_OBJ(TYPEAPI_CTX_DATA_CCTOR_ARR_REG));
    ctxDataRecordCctor_.AddInstruction(pandasm::Create_CHECKCAST(type.GetPandasmName()));
    ctxDataRecordCctor_.AddInstruction(pandasm::Create_STSTATIC_OBJ(filedIdForIns));

    return filedIdForIns;
}

void TypeCreatorCtx::FlushTypeAPICtxDataRecordsToProgram()
{
    if (ctxDataRecord_.name.empty()) {
        return;
    }

    ctxDataRecordCctor_.AddInstruction(pandasm::Create_RETURN_VOID());

    auto name = ctxDataRecord_.name;
    prog_.recordTable.emplace(std::move(name), std::move(ctxDataRecord_));
    name = ctxDataRecordCctor_.name;
    prog_.functionTable.emplace(std::move(name), std::move(ctxDataRecordCctor_));
}

void TypeCreatorCtx::SaveObjects(EtsCoroutine *coro, VMHandle<EtsArray> &objects)
{
    auto arrayKlass = coro->GetPandaVM()->GetClassLinker()->GetClass(
        pandasm::Type {typeapi_create_consts::TYPE_OBJECT, 1, true}.GetDescriptor(true).c_str());
    auto arr = coretypes::Array::Create(arrayKlass->GetRuntimeClass(), 1, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    arr->Set<ObjectHeader *>(0, objects.GetPtr()->GetCoreType());
    initArrObject_ = arr;
    objects = VMHandle<EtsArray>(coro, initArrObject_);
}

EtsArray *TypeCreatorCtx::GetObjects([[maybe_unused]] EtsCoroutine *coro)
{
    // NOTE: this code is possible because that array is allocated as non-movable!
    // and is stored in vmhandle => can't be moved or deallcoated
    return reinterpret_cast<EtsArray *>(initArrObject_->Get<ObjectHeader *>(0));
}

void TypeCreatorCtx::InitializeCtxDataRecord(EtsCoroutine *coro)
{
    if (ctxDataRecordName_.empty()) {
        return;
    }
    auto name = pandasm::Type(ctxDataRecordName_, 0).GetDescriptor();
    auto klass = coro->GetPandaVM()->GetClassLinker()->GetClass(name.c_str());
    ASSERT(!klass->IsInitialized());
    auto linker = Runtime::GetCurrent()->GetClassLinker();
    [[maybe_unused]] auto result = linker->InitializeClass(coro, klass->GetRuntimeClass());
    ASSERT(result);
    ASSERT(klass->IsInitialized());
}

pandasm::Record &TypeCreatorCtx::GetTypeAPICtxDataRecord()
{
    if (!ctxDataRecord_.name.empty()) {
        return ctxDataRecord_;
    }
    static std::atomic_uint ctxDataNextName {};
    ctxDataRecordName_ = typeapi_create_consts::CREATOR_CTX_DATA_PREFIX;
    ctxDataRecordName_ += std::to_string(ctxDataNextName++);
    ctxDataRecord_.name = ctxDataRecordName_;

    AddRefTypeAsExternal(std::string {typeapi_create_consts::TYPE_OBJECT});
    AddRefTypeAsExternal(pandasm::Type {typeapi_create_consts::TYPE_OBJECT, 1}.GetName());

    ctxDataRecordCctor_.name = ctxDataRecord_.name;
    ctxDataRecordCctor_.name += '.';
    ctxDataRecordCctor_.name += panda_file::GetCctorName(SourceLanguage::ETS);
    ctxDataRecordCctor_.metadata->SetAttribute(typeapi_create_consts::ATTR_CCTOR);
    ctxDataRecordCctor_.metadata->SetAttribute(typeapi_create_consts::ATTR_STATIC);
    ctxDataRecordCctor_.regsNum = 1;
    ctxDataRecordCctor_.AddInstruction(pandasm::Create_MOVI_64(0, reinterpret_cast<EtsLong>(this)));
    ctxDataRecordCctor_.AddInstruction(
        pandasm::Create_CALL_SHORT(0, 0, std::string {typeapi_create_consts::FUNCTION_GET_OBJECTS_FOR_CCTOR}));
    ctxDataRecordCctor_.AddInstruction(pandasm::Create_STA_OBJ(TYPEAPI_CTX_DATA_CCTOR_ARR_REG));

    AddRefTypeAsExternal(std::string {typeapi_create_consts::TYPE_TYPE_CREATOR_CTX});
    pandasm::Function getObjectsArray {std::string {typeapi_create_consts::FUNCTION_GET_OBJECTS_FOR_CCTOR},
                                       SourceLanguage::ETS};
    getObjectsArray.metadata->SetAttribute(typeapi_create_consts::ATTR_EXTERNAL);
    getObjectsArray.returnType = pandasm::Type {typeapi_create_consts::TYPE_OBJECT, 1};
    getObjectsArray.params.emplace_back(
        pandasm::Type::FromPrimitiveId(ConvertEtsTypeToPandaType(EtsType::LONG).GetId()), SourceLanguage::ETS);
    auto getObjectsArrayName = getObjectsArray.name;
    prog_.functionTable.emplace(std::move(getObjectsArrayName), std::move(getObjectsArray));

    return ctxDataRecord_;
}

pandasm::Record &TypeCreatorCtx::AddRefTypeAsExternal(const std::string &name)
{
    pandasm::Record objectRec {name, SourceLanguage::ETS};
    objectRec.metadata->SetAttribute(typeapi_create_consts::ATTR_EXTERNAL);
    return prog_.recordTable.emplace(objectRec.name, std::move(objectRec)).first->second;
}

const std::pair<std::string, std::string> &TypeCreatorCtx::DeclarePrimitive(const std::string &primTypeName)
{
    if (auto found = primitiveTypesCtorDtor_.find(primTypeName); found != primitiveTypesCtorDtor_.end()) {
        return found->second;
    }

    auto primType = pandasm::Type {primTypeName, 0};

    std::string objectTypeName {typeapi_create_consts::TYPE_BOXED_PREFIX};
    if (primTypeName == "u1") {
        objectTypeName += "Boolean";
    } else if (primTypeName == "i8") {
        objectTypeName += "Byte";
    } else if (primTypeName == "i16") {
        objectTypeName += "Short";
    } else if (primTypeName == "i32") {
        objectTypeName += "Int";
    } else if (primTypeName == "i64") {
        objectTypeName += "Long";
    } else if (primTypeName == "u16") {
        objectTypeName += "Char";
    } else if (primTypeName == "void") {
        objectTypeName += "Void";
    } else if (primTypeName == "f32") {
        objectTypeName += "Float";
    } else if (primTypeName == "f64") {
        objectTypeName += "Double";
    } else {
        UNREACHABLE();
    }
    AddRefTypeAsExternal(objectTypeName);
    auto objectType = pandasm::Type {objectTypeName, 0};

    PandasmMethodCreator ctor {objectTypeName + "." + panda_file::GetCtorName(SourceLanguage::ETS), this};
    ctor.AddParameter(objectType);
    ctor.AddParameter(primType);
    ctor.GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_EXTERNAL);
    ctor.GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_CTOR);
    ctor.Create();

    PandasmMethodCreator unboxed {objectTypeName + ".unboxed", this};
    unboxed.AddParameter(objectType);
    unboxed.AddResult(primType);
    unboxed.GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_EXTERNAL);
    unboxed.Create();

    return primitiveTypesCtorDtor_
        .emplace(primTypeName, std::make_pair(ctor.GetFunctionName(), unboxed.GetFunctionName()))
        .first->second;
}

void LambdaTypeCreator::AddParameter(pandasm::Type param)
{
    ASSERT(!param.IsVoid());
    fn_.params.emplace_back(std::move(param), SourceLanguage::ETS);
}

void LambdaTypeCreator::AddResult(const pandasm::Type &type)
{
    fn_.returnType = type;
}

void LambdaTypeCreator::Create()
{
    ASSERT(!finished_);
    finished_ = true;
    // IMPORTANT: must be synchronized with
    // tools/es2panda/varbinder/ETSBinder.cpp
    // ETSBinder::FormLambdaName

    static constexpr size_t MAX_NUMBER_OF_PARAMS_FOR_FUNCTIONAL_INTERFACE = 16;

    if (fn_.params.size() > MAX_NUMBER_OF_PARAMS_FOR_FUNCTIONAL_INTERFACE) {
        GetCtx()->AddError("Function types with more than 16 parameters are not supported");
        return;
    }
    name_ += std::to_string(fn_.params.size());

    rec_.name = name_;
    fnName_ = fn_.name = name_ + ".invoke0";
    fn_.params.insert(fn_.params.begin(), pandasm::Function::Parameter(pandasm::Type(name_, 0), SourceLanguage::ETS));
    for (const auto &attr : typeapi_create_consts::ATTR_ABSTRACT_METHOD) {
        fn_.metadata->SetAttribute(attr);
    }
    fn_.metadata->SetAttributeValue(typeapi_create_consts::ATTR_ACCESS, typeapi_create_consts::ATTR_ACCESS_VAL_PUBLIC);
    GetCtx()->Program().recordTable.emplace(name_, std::move(rec_));
    GetCtx()->Program().functionTable.emplace(fnName_, std::move(fn_));
}

void PandasmMethodCreator::AddParameter(pandasm::Type param)
{
    name_ += ':';
    name_ += param.GetName();
    fn_.params.emplace_back(std::move(param), SourceLanguage::ETS);
}

void PandasmMethodCreator::AddResult(pandasm::Type type)
{
    if (fn_.metadata->IsCtor()) {
        fn_.returnType = pandasm::Type {"void", 0};
        return;
    }
    fn_.returnType = std::move(type);
}

void PandasmMethodCreator::Create()
{
    ASSERT(!finished_);
    finished_ = true;
    fn_.name = name_;
    auto ok = ctx_->Program().functionTable.emplace(name_, std::move(fn_)).second;
    if (!ok) {
        ctx_->AddError("duplicate function " + name_);
    }
}
}  // namespace ark::ets
