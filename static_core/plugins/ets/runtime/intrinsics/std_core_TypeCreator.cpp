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

#include "assembler/assembly-emitter.h"
#include "assembler/assembly-field.h"
#include "assembler/assembly-function.h"
#include "assembler/assembly-ins.h"
#include "assembler/assembly-parser.h"
#include "assembler/assembly-program.h"
#include "assembler/assembly-record.h"
#include "assembler/assembly-type.h"
#include "ins_create_api.h"

#include "ets_coroutine.h"

#include "runtime/include/runtime.h"
#include "runtime/mem/vm_handle.h"
#include "source_lang_enum.h"
#include "types/ets_array.h"
#include "types/ets_class.h"
#include "types/ets_field.h"
#include "types/ets_method.h"
#include "types/ets_primitives.h"
#include "types/ets_string.h"
#include "types/ets_typeapi_create_panda_constants.h"
#include "types/ets_void.h"
#include "types/ets_type.h"
#include "types/ets_typeapi.h"
#include "types/ets_typeapi_create.h"

namespace panda::ets::intrinsics {
namespace {

EtsString *ErrorFromCtx(TypeCreatorCtx *ctx)
{
    if (auto err = ctx->GetError(); err) {
        EtsString::CreateFromMUtf8(err.value().data(), err.value().size());
    }
    return nullptr;
}

pandasm::Type GetPandasmTypeFromDescriptor(TypeCreatorCtx *ctx, std::string_view descriptor)
{
    auto ret = pandasm::Type::FromDescriptor(descriptor);

    if (ret.IsObject()) {
        auto pandasm_name = ret.GetPandasmName();
        ctx->AddRefTypeAsExternal(pandasm_name);
        return pandasm::Type {std::move(pandasm_name), ret.GetRank()};
    }
    return ret;
}

pandasm::Type EtsTypeToPandasm(EtsType type)
{
    ASSERT(type != EtsType::OBJECT);
    return pandasm::Type::FromPrimitiveId(ConvertEtsTypeToPandaType(type).GetId());
}

pandasm::Ins GetReturnStatement(EtsType type)
{
    switch (type) {
        case EtsType::BOOLEAN:
        case EtsType::BYTE:
        case EtsType::CHAR:
        case EtsType::SHORT:
        case EtsType::INT:
        case EtsType::FLOAT:
            return pandasm::Create_RETURN();
        case EtsType::LONG:
        case EtsType::DOUBLE:
            return pandasm::Create_RETURN_64();
        case EtsType::VOID:
            return pandasm::Create_RETURN_VOID();
        case EtsType::OBJECT:
            return pandasm::Create_RETURN_OBJ();
        case EtsType::UNKNOWN:
        default:
            UNREACHABLE();
    }
}

PandasmMethodCreator CreateCopiedMethod(TypeCreatorCtx *ctx, const std::string &prefix, EtsMethod *method)
{
    PandasmMethodCreator fn {prefix + method->GetName(), ctx};

    size_t ref_num = 0;

    for (size_t arg_num = 0; arg_num < method->GetNumArgs(); arg_num++) {
        auto ets_type = method->GetArgType(arg_num);
        if (ets_type == EtsType::OBJECT) {
            fn.AddParameter(GetPandasmTypeFromDescriptor(ctx, method->GetRefArgType(ref_num++)));
        } else {
            fn.AddParameter(EtsTypeToPandasm(ets_type));
        }
    }
    auto ets_ret_type = method->GetReturnValueType();
    if (ets_ret_type == EtsType::OBJECT) {
        fn.AddResult(GetPandasmTypeFromDescriptor(ctx, method->GetReturnTypeDescriptor()));
    } else {
        fn.AddResult(EtsTypeToPandasm(ets_ret_type));
    }

    return fn;
}

void SetAccessFlags(pandasm::ItemMetadata *meta, EtsTypeAPIAccessModifier mod)
{
    switch (mod) {
        case EtsTypeAPIAccessModifier::PUBLIC:
            meta->SetAttributeValue(typeapi_create_consts::ATTR_ACCESS, typeapi_create_consts::ATTR_ACCESS_VAL_PUBLIC);
            break;
        case EtsTypeAPIAccessModifier::PRIVATE:
            meta->SetAttributeValue(typeapi_create_consts::ATTR_ACCESS, typeapi_create_consts::ATTR_ACCESS_VAL_PRIVATE);
            break;
        case EtsTypeAPIAccessModifier::PROTECTED:
            meta->SetAttributeValue(typeapi_create_consts::ATTR_ACCESS,
                                    typeapi_create_consts::ATTR_ACCESS_VAL_PROTECTED);
            break;
    }
}

bool HasFeatureAttribute(EtsInt where, EtsTypeAPIAttributes attr)
{
    static_assert(sizeof(EtsInt) == sizeof(uint32_t));
    return (bit_cast<uint32_t>(static_cast<EtsInt>(attr)) & bit_cast<uint32_t>(where)) != 0;
}

template <typename T>
T *PtrFromLong(EtsLong ptr)
{
    return reinterpret_cast<T *>(ptr);
}

template <typename T>
EtsLong PtrToLong(T *ptr)
{
    return reinterpret_cast<EtsLong>(ptr);
}

}  // namespace

extern "C" {

EtsLong TypeAPITypeCreatorCtxCreate()
{
    auto ret = Runtime::GetCurrent()->GetInternalAllocator()->New<TypeCreatorCtx>();
    return reinterpret_cast<ssize_t>(ret);
}

EtsVoid *TypeAPITypeCreatorCtxDestroy(EtsLong ctx)
{
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(PtrFromLong<TypeCreatorCtx>(ctx));
    return EtsVoid::GetInstance();
}

EtsString *TypeAPITypeCreatorCtxCommit(EtsLong ctx_ptr, EtsArray *objects)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsArray> objects_handle {coroutine, objects->GetCoreType()};

    auto ctx = PtrFromLong<TypeCreatorCtx>(ctx_ptr);

    ctx->FlushTypeAPICtxDataRecordsToProgram();

    panda_file::MemoryWriter writer;
    auto status = pandasm::AsmEmitter::Emit(&writer, ctx->Program());
    if (!status) {
        return EtsString::CreateFromMUtf8(("can't write file in memory" + pandasm::AsmEmitter::GetLastError()).c_str());
    }

    // debug
    // panda_file::FileWriter {"/tmp/test.abc"}.WriteBytes(writer.GetData());

    auto linker = Runtime::GetCurrent()->GetClassLinker();
    auto &data = writer.GetData();
    auto file = panda_file::OpenPandaFileFromMemory(data.data(), data.size());

    ctx->SaveObjects(coroutine, objects_handle);

    linker->AddPandaFile(std::move(file));

    ctx->InitializeCtxDataRecord(coroutine);
    return nullptr;
}

EtsLong TypeAPITypeCreatorCtxClassCreate(EtsLong ctx_ptr, EtsString *name, EtsInt attrs)
{
    auto ctx = PtrFromLong<TypeCreatorCtx>(ctx_ptr);
    pandasm::Record rec {std::string {name->GetMutf8()}, SourceLanguage::ETS};
    rec.conflict = true;

    if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::FINAL)) {
        rec.metadata->SetAttribute(typeapi_create_consts::ATTR_FINAL);
    }

    auto name_str = rec.name;
    auto [iter, ok] = ctx->Program().record_table.emplace(name_str, std::move(rec));
    if (!ok) {
        ctx->AddError("duplicate class " + name_str);
        return 0;
    }
    return PtrToLong(ctx->Alloc<ClassCreator>(&iter->second, ctx));
}

EtsLong TypeAPITypeCreatorCtxInterfaceCreate(EtsLong ctx_ptr, EtsString *name)
{
    auto ctx = PtrFromLong<TypeCreatorCtx>(ctx_ptr);
    pandasm::Record rec {std::string {name->GetMutf8()}, SourceLanguage::ETS};
    rec.conflict = true;
    auto name_str = rec.name;
    for (const auto &attr : typeapi_create_consts::ATTR_INTERFACE) {
        rec.metadata->SetAttribute(attr);
    }
    auto [iter, ok] = ctx->Program().record_table.emplace(name_str, std::move(rec));
    if (!ok) {
        ctx->AddError("duplicate class " + name_str);
        return 0;
    }
    return PtrToLong(ctx->Alloc<InterfaceCreator>(&iter->second, ctx));
}

EtsString *TypeAPITypeCreatorCtxGetError(EtsLong ctx_ptr)
{
    auto ctx = PtrFromLong<TypeCreatorCtx>(ctx_ptr);
    return ErrorFromCtx(ctx);
}

EtsObject *TypeAPITypeCreatorCtxGetObjectsArrayForCCtor(EtsLong ctx_ptr)
{
    auto ctx = PtrFromLong<TypeCreatorCtx>(ctx_ptr);
    auto coro = EtsCoroutine::GetCurrent();
    return ctx->GetObjects(coro)->AsObject();
}

EtsString *TypeAPITypeCreatorCtxClassSetBase(EtsLong class_ptr, EtsString *base_td)
{
    auto creator = PtrFromLong<ClassCreator>(class_ptr);
    auto par = GetPandasmTypeFromDescriptor(creator->GetCtx(), base_td->GetMutf8());
    if (par.GetRank() != 0) {
        return EtsString::CreateFromMUtf8("can't have array base");
    }
    creator->GetRec()->metadata->SetAttributeValue(typeapi_create_consts::ATTR_EXTENDS, par.GetName());
    return nullptr;
}

EtsString *TypeAPITypeCreatorCtxInterfaceAddBase(EtsLong iface_ptr, EtsString *base_td)
{
    auto creator = PtrFromLong<InterfaceCreator>(iface_ptr);
    auto par = GetPandasmTypeFromDescriptor(creator->GetCtx(), base_td->GetMutf8());
    if (par.GetRank() != 0) {
        return EtsString::CreateFromMUtf8("can't have array base");
    }
    creator->GetRec()->metadata->SetAttributeValue(typeapi_create_consts::ATTR_IMPLEMENTS, par.GetName());
    return nullptr;
}

EtsString *TypeAPITypeCreatorCtxGetTypeDescFromPointer(EtsLong nptr)
{
    auto creator = PtrFromLong<TypeCreator>(nptr);
    auto type = creator->GetType();
    return EtsString::CreateFromMUtf8(type.GetDescriptor(!type.IsPrimitive()).c_str());
}

EtsString *TypeAPITypeCreatorCtxMethodAddParam(EtsLong method_ptr, EtsString *param_td,
                                               [[maybe_unused]] EtsString *name, [[maybe_unused]] EtsInt attrs)
{
    // TODO(kprokopenko): dump meta info
    auto m = PtrFromLong<PandasmMethodCreator>(method_ptr);
    auto type = GetPandasmTypeFromDescriptor(m->Ctx(), param_td->GetMutf8());
    m->AddParameter(std::move(type));
    return nullptr;
}

EtsLong TypeAPITypeCreatorCtxMethodCreate(EtsLong containing_type_ptr, EtsString *name, EtsInt attrs)
{
    auto klass = PtrFromLong<ClassCreator>(containing_type_ptr);
    auto name_str = klass->GetRec()->name;
    name_str += ".";
    if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::CONSTRUCTOR)) {
        name_str += panda_file::GetCtorName(SourceLanguage::ETS);
    } else {
        if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::GETTER)) {
            name_str += GETTER_BEGIN;
        } else if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::SETTER)) {
            name_str += SETTER_BEGIN;
        }
        name_str += name->GetMutf8();
    }
    auto ret = klass->GetCtx()->Alloc<PandasmMethodCreator>(std::move(name_str), klass->GetCtx());
    if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::STATIC)) {
        ret->GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_STATIC);
    } else {
        ret->GetFn().params.emplace_back(pandasm::Type {klass->GetRec()->name, 0}, SourceLanguage::ETS);
    }
    if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::ABSTRACT)) {
        ret->GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_NOIMPL);
    }
    if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::CONSTRUCTOR)) {
        ret->GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_CTOR);
    }
    return PtrToLong(ret);
}

EtsString *TypeAPITypeCreatorCtxMethodAddAccessMod(EtsLong method_ptr, EtsInt access)
{
    auto m = PtrFromLong<PandasmMethodCreator>(method_ptr);
    SetAccessFlags(m->GetFn().metadata.get(), static_cast<EtsTypeAPIAccessModifier>(access));
    return ErrorFromCtx(m->Ctx());
}

EtsString *TypeAPITypeCreatorCtxMethodAdd(EtsLong method_ptr)
{
    auto m = PtrFromLong<PandasmMethodCreator>(method_ptr);
    m->Create();
    return ErrorFromCtx(m->Ctx());
}

EtsString *TypeAPITypeCreatorCtxMethodAddBodyFromMethod(EtsLong method_ptr, EtsString *method_desc)
{
    auto m = PtrFromLong<PandasmMethodCreator>(method_ptr);
    auto meth = EtsMethod::FromTypeDescriptor(method_desc->GetMutf8());
    auto ctx = m->Ctx();

    auto parent_method_class_name = GetPandasmTypeFromDescriptor(m->Ctx(), meth->GetClass()->GetDescriptor());

    // TODO(kprokopenko): implement type checking

    auto parent_method = CreateCopiedMethod(ctx, parent_method_class_name.GetName() + ".", meth);
    parent_method.GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_EXTERNAL);
    parent_method.Create();

    m->GetFn().AddInstruction(pandasm::Create_CALL_RANGE(0, parent_method.GetFunctionName()));
    m->GetFn().AddInstruction(GetReturnStatement(meth->GetReturnValueType()));

    return ErrorFromCtx(m->Ctx());
}

EtsString *TypeAPITypeCreatorCtxMethodAddBodyFromLambda(EtsLong method_ptr, EtsInt lambda_object_id,
                                                        EtsString *lambda_td)
{
    auto m = PtrFromLong<PandasmMethodCreator>(method_ptr);
    auto ctx = m->Ctx();

    auto coro = EtsCoroutine::GetCurrent();
    auto klass_td = lambda_td->GetMutf8();
    auto klass_name = GetPandasmTypeFromDescriptor(m->Ctx(), klass_td);
    auto klass = coro->GetPandaVM()->GetClassLinker()->GetClass(klass_td.c_str());
    ASSERT(klass->IsInitialized());
    auto meth = klass->GetMethod("invoke");
    if (meth == nullptr) {
        return EtsString::CreateFromMUtf8("method is absent");
    }

    auto fld = m->Ctx()->AddInitField(lambda_object_id, pandasm::Type {klass_name, 0});

    auto external_fn = CreateCopiedMethod(ctx, klass_name.GetName() + ".", meth);
    external_fn.GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_EXTERNAL);
    external_fn.Create();

    m->GetFn().regs_num = 1;
    m->GetFn().AddInstruction(pandasm::Create_LDSTATIC_OBJ(fld));
    m->GetFn().AddInstruction(pandasm::Create_STA_OBJ(0));
    if (EtsMethod::ToRuntimeMethod(meth)->IsFinal()) {
        m->GetFn().AddInstruction(pandasm::Create_CALL_RANGE(0, external_fn.GetFunctionName()));
    } else {
        m->GetFn().AddInstruction(pandasm::Create_CALL_VIRT_RANGE(0, external_fn.GetFunctionName()));
    }

    m->GetFn().AddInstruction(GetReturnStatement(meth->GetReturnValueType()));

    return ErrorFromCtx(m->Ctx());
}

EtsString *TypeAPITypeCreatorCtxMethodAddBodyFromErasedLambda(EtsLong method_ptr, EtsInt lambda_id)
{
    auto m = PtrFromLong<PandasmMethodCreator>(method_ptr);

    m->Ctx()->AddRefTypeAsExternal(std::string {typeapi_create_consts::TYPE_OBJECT});

    LambdaTypeCreator lambda {m->Ctx()};
    lambda.AddParameter(pandasm::Type {typeapi_create_consts::TYPE_OBJECT, 0});
    lambda.AddParameter(pandasm::Type {typeapi_create_consts::TYPE_OBJECT, 1});
    lambda.AddResult(pandasm::Type {typeapi_create_consts::TYPE_OBJECT, 0});
    lambda.Create();

    auto saved_lambda_field = m->Ctx()->AddInitField(lambda_id, lambda.GetType());

    constexpr int TMP_REG = 0;
    constexpr int LMB_REG = 0;
    constexpr int RECV_REG = LMB_REG + 1;
    constexpr int ARR_REG = RECV_REG + 1;

    constexpr int ARGS_REG_START = ARR_REG + 1;

    auto &fn = m->GetFn();
    auto is_static = fn.metadata->GetAttribute(std::string {typeapi_create_consts::ATTR_STATIC});
    fn.regs_num = ARGS_REG_START;
    const auto &ret = fn.return_type;
    auto pars = fn.params.size();
    auto arr_len = pars - (is_static ? 0 : 1);
    fn.AddInstruction(pandasm::Create_MOVI(TMP_REG, arr_len));
    fn.AddInstruction(
        pandasm::Create_NEWARR(ARR_REG, TMP_REG, pandasm::Type {typeapi_create_consts::TYPE_OBJECT, 1}.GetName()));

    fn.AddInstruction(pandasm::Create_MOVI(TMP_REG, 0));

    if (is_static) {
        fn.AddInstruction(pandasm::Create_LDA_NULL());
        fn.AddInstruction(pandasm::Create_STA_OBJ(RECV_REG));
    }

    for (size_t i = 0; i < fn.params.size(); i++) {
        auto &par = fn.params[i];
        // adjust array store index
        if (i != 0 && (i != 1 || is_static)) {
            fn.AddInstruction(pandasm::Create_INCI(TMP_REG, 1));
        }
        if (par.type.IsObject()) {
            fn.AddInstruction(pandasm::Create_LDA_OBJ(ARGS_REG_START + i));
        } else {
            auto ctor = m->Ctx()->DeclarePrimitive(par.type.GetComponentName()).first;
            fn.AddInstruction(pandasm::Create_INITOBJ_SHORT(ARGS_REG_START + i, 0, ctor));
        }
        if (i == 0 && !is_static) {
            // set recv
            fn.AddInstruction(pandasm::Create_STA_OBJ(RECV_REG));
        } else {
            // set arg
            fn.AddInstruction(pandasm::Create_STARR_OBJ(ARR_REG, TMP_REG));
        }
    }

    fn.AddInstruction(pandasm::Create_LDSTATIC_OBJ(saved_lambda_field));
    fn.AddInstruction(pandasm::Create_STA_OBJ(LMB_REG));

    fn.AddInstruction(pandasm::Create_CALL_VIRT(LMB_REG, RECV_REG, ARR_REG, 0, lambda.GetFunctionName()));

    fn.AddInstruction(pandasm::Create_STA_OBJ(0));
    if (!fn.return_type.IsObject()) {
        auto destr = m->Ctx()->DeclarePrimitive(fn.return_type.GetComponentName()).second;
        fn.AddInstruction(pandasm::Create_CALL_SHORT(0, 0, destr));
    }

    if (fn.return_type.IsObject()) {
        fn.AddInstruction(pandasm::Create_CHECKCAST(ret.GetName()));
    }
    auto return_id = fn.return_type.GetId();
    fn.AddInstruction(GetReturnStatement(ConvertPandaTypeToEtsType(panda_file::Type {return_id})));

    return ErrorFromCtx(m->Ctx());
}

EtsString *TypeAPITypeCreatorCtxMethodAddBodyDefault(EtsLong method_ptr)
{
    auto m = PtrFromLong<PandasmMethodCreator>(method_ptr);
    auto &fn = m->GetFn();

    // call default constructor in case of constructor
    if (fn.metadata->IsCtor()) {
        auto self_name = m->GetFn().params.front().type.GetName();
        auto &record_table = m->Ctx()->Program().record_table;
        auto super_name = record_table.find(self_name)
                              ->second.metadata->GetAttributeValue(std::string {typeapi_create_consts::ATTR_EXTENDS})
                              .value();
        m->Ctx()->AddRefTypeAsExternal(super_name);
        PandasmMethodCreator super_ctor {super_name + "." + panda_file::GetCtorName(panda_file::SourceLang::ETS),
                                         m->Ctx()};
        super_ctor.AddParameter(pandasm::Type {super_name, 0, true});
        super_ctor.GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_EXTERNAL);
        super_ctor.Create();

        fn.AddInstruction(pandasm::Create_CALL_SHORT(0, 0, super_ctor.GetFunctionName()));
    }

    const auto &ret = fn.return_type;
    if (ret.IsVoid()) {
        fn.AddInstruction(pandasm::Create_RETURN_VOID());
    } else if (ret.IsObject()) {
        if (ret.GetDescriptor() == panda_file_items::class_descriptors::VOID) {
            fn.AddInstruction(pandasm::Create_LDSTATIC_OBJ(m->Ctx()->GetRefVoidInstanceName()));
        } else {
            fn.AddInstruction(pandasm::Create_LDA_NULL());
        }
        fn.AddInstruction(pandasm::Create_RETURN_OBJ());
        // return EtsString::CreateFromMUtf8("can't make default return for object type");
    } else if (ret.IsFloat32()) {
        fn.AddInstruction(pandasm::Create_FLDAI(0));
        fn.AddInstruction(pandasm::Create_RETURN());
    } else if (ret.IsFloat64()) {
        fn.AddInstruction(pandasm::Create_FLDAI_64(0));
        fn.AddInstruction(pandasm::Create_RETURN_64());
    } else if (ret.IsPrim64()) {
        fn.AddInstruction(pandasm::Create_LDAI_64(0));
        fn.AddInstruction(pandasm::Create_RETURN_64());
    } else {
        fn.AddInstruction(pandasm::Create_LDAI(0));
        fn.AddInstruction(pandasm::Create_RETURN());
    }

    return ErrorFromCtx(m->Ctx());
}

EtsString *TypeAPITypeCreatorCtxMethodAddResult(EtsLong method_ptr, EtsString *descriptor)
{
    auto m = PtrFromLong<PandasmMethodCreator>(method_ptr);
    m->AddResult(GetPandasmTypeFromDescriptor(m->Ctx(), descriptor->GetMutf8().c_str()));
    return ErrorFromCtx(m->Ctx());
}

EtsLong TypeAPITypeCreatorCtxLambdaTypeCreate(EtsLong ctx_ptr, [[maybe_unused]] EtsInt attrs)
{
    auto ctx = PtrFromLong<TypeCreatorCtx>(ctx_ptr);
    // TODO(kprokopenko): add attributes
    auto fn = ctx->Alloc<LambdaTypeCreator>(ctx);
    return PtrToLong(fn);
}

EtsString *TypeAPITypeCreatorCtxLambdaTypeAddParam(EtsLong ft_ptr, EtsString *td, [[maybe_unused]] EtsInt attrs)
{
    // TODO(kprokopenko): dump meta info
    auto creator = PtrFromLong<LambdaTypeCreator>(ft_ptr);
    creator->AddParameter(GetPandasmTypeFromDescriptor(creator->GetCtx(), td->GetMutf8()));
    return ErrorFromCtx(creator->GetCtx());
}

EtsString *TypeAPITypeCreatorCtxLambdaTypeAddResult(EtsLong ft_ptr, EtsString *td)
{
    auto creator = PtrFromLong<LambdaTypeCreator>(ft_ptr);
    creator->AddResult(GetPandasmTypeFromDescriptor(creator->GetCtx(), td->GetMutf8()));
    return ErrorFromCtx(creator->GetCtx());
}

EtsString *TypeAPITypeCreatorCtxLambdaTypeAdd(EtsLong ft_ptr)
{
    auto creator = PtrFromLong<LambdaTypeCreator>(ft_ptr);
    creator->Create();
    return ErrorFromCtx(creator->GetCtx());
}

EtsString *TypeAPITypeCreatorCtxClassAddIface(EtsLong class_ptr, EtsString *descr)
{
    auto creator = PtrFromLong<ClassCreator>(class_ptr);
    auto iface = GetPandasmTypeFromDescriptor(creator->GetCtx(), descr->GetMutf8());
    creator->GetRec()->metadata->SetAttributeValue(typeapi_create_consts::ATTR_IMPLEMENTS, iface.GetName());
    return ErrorFromCtx(creator->GetCtx());
}

EtsString *TypeAPITypeCreatorCtxClassAddField(EtsLong class_ptr, EtsString *name, EtsString *descr, EtsInt attrs,
                                              EtsInt access)
{
    auto klass = PtrFromLong<ClassCreator>(class_ptr);
    auto type = GetPandasmTypeFromDescriptor(klass->GetCtx(), descr->GetMutf8());
    pandasm::Field fld {SourceLanguage::ETS};
    if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::STATIC)) {
        fld.metadata->SetAttribute(typeapi_create_consts::ATTR_STATIC);
    }
    if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::READONLY)) {
        fld.metadata->SetAttribute(typeapi_create_consts::ATTR_FINAL);
    }
    fld.name = name->GetMutf8();
    fld.type = pandasm::Type(type, 0);
    SetAccessFlags(fld.metadata.get(), static_cast<EtsTypeAPIAccessModifier>(access));
    klass->GetRec()->field_list.emplace_back(std::move(fld));
    return ErrorFromCtx(klass->GetCtx());
}
}

}  // namespace panda::ets::intrinsics
