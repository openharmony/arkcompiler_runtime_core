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

#ifndef PANDA_PLUGINS_ETS_TYPEAPI_CREATE_H_
#define PANDA_PLUGINS_ETS_TYPEAPI_CREATE_H_

#include "assembly-function.h"
#include "assembly-program.h"

namespace panda {
template <typename T>
class VMHandle;
}  // namespace panda

namespace panda::ets {
class EtsCoroutine;
class EtsArray;
}  // namespace panda::ets

namespace panda::ets {
class TypeCreatorCtx {
public:
    pandasm::Program &Program()
    {
        return prog_;
    }

    template <typename T, typename... A>
    T *Alloc(A &&...a)
    {
        auto p = std::make_shared<T>(std::forward<A>(a)...);
        auto ret = p.get();
        datas_.push_back(std::move(p));
        return ret;
    }

    /**
     * Adds field with permanent data used in TypeAPI$CtxData$_
     * also adds intializer into TypeAPI$CtxData$_.cctor that loads that id
     * from
     * @param id index of filed in init array
     * @param type type of that field
     * @returns field id that can be passed into pandasm instructions to access it
     */
    std::string AddInitField(uint32_t id, const pandasm::Type &type);

    /**
     * Adds TypeAPI$CtxData$_ and TypeAPI$CtxData$_$init if records to program necessary
     * also emplaces necessary cctors
     */
    void FlushTypeAPICtxDataRecordsToProgram();

    /**
     * Initializes TypeAPI$CtxData$_$init with provided storage of runtime objects
     * which then is lazily used in TypeAPI$CtxData$_ cctor
     */
    void InitializeCtxDataRecord(EtsCoroutine *coro, VMHandle<EtsArray> &arr);

    /**
     * Creates TypeAPI$CtxData record if it wasn't created before
     * @returns TypeAPI$CtxData record
     */
    pandasm::Record &GetTypeAPICtxDataRecord();

    void AddRefTypeAsExternal(const std::string &name);

    /**
     * Lazily declares primitive reference wrapper
     * @returns pair of constructor and unwrapper
     */
    const std::pair<std::string, std::string> &DeclarePrimitive(const std::string &prim_type_name);

    void AddError(std::string_view err)
    {
        error_ += err;
    }

    std::optional<std::string_view> GetError() const
    {
        if (error_.empty()) {
            return std::nullopt;
        }
        return error_;
    }

private:
    pandasm::Program prog_;
    pandasm::Record ctx_data_record_ {"", panda_file::SourceLang::ETS};
    pandasm::Record ctx_data_init_record_ {"", panda_file::SourceLang::ETS};
    std::string ctx_data_init_record_name_;
    pandasm::Function init_fn_ {"", panda_file::SourceLang::ETS};
    std::map<std::string, std::pair<std::string, std::string>> primitive_types_ctor_dtor_;
    std::vector<std::shared_ptr<void>> datas_;
    std::string error_;
};

enum class TypeCreatorKind {
    CLASS,
    INTERFACE,
    LAMBDA_TYPE,
};

class TypeCreator {
public:
    virtual TypeCreatorKind GetKind() const = 0;
    virtual pandasm::Type GetType() const = 0;

    explicit TypeCreator(TypeCreatorCtx *ctx) : ctx_(ctx) {}

    TypeCreatorCtx *GetCtx() const
    {
        return ctx_;
    }

private:
    TypeCreatorCtx *ctx_;
};

class ClassCreator final : public TypeCreator {
public:
    explicit ClassCreator(pandasm::Record *rec, TypeCreatorCtx *ctx) : TypeCreator(ctx), rec_(rec) {}

    TypeCreatorKind GetKind() const override
    {
        return TypeCreatorKind::CLASS;
    }

    pandasm::Type GetType() const override
    {
        return pandasm::Type {rec_->name, 0, true};
    }

    pandasm::Record *GetRec() const
    {
        return rec_;
    }

private:
    pandasm::Record *rec_;
};

class InterfaceCreator final : public TypeCreator {
public:
    explicit InterfaceCreator(pandasm::Record *rec, TypeCreatorCtx *ctx) : TypeCreator(ctx), rec_(rec) {}

    TypeCreatorKind GetKind() const override
    {
        return TypeCreatorKind::INTERFACE;
    }

    pandasm::Type GetType() const override
    {
        return pandasm::Type {rec_->name, 0, true};
    }

    pandasm::Record *GetRec() const
    {
        return rec_;
    }

private:
    pandasm::Record *rec_;
};

class LambdaTypeCreator final : public TypeCreator {
public:
    explicit LambdaTypeCreator(TypeCreatorCtx *ctx) : TypeCreator(ctx)
    {
        rec_.metadata->SetAttribute("ets.abstract");
        rec_.metadata->SetAttribute("ets.interface");
        rec_.metadata->SetAttributeValue("access.record", "public");
    }

    TypeCreatorKind GetKind() const override
    {
        return TypeCreatorKind::LAMBDA_TYPE;
    }

    pandasm::Type GetType() const override
    {
        ASSERT(finished_);
        return pandasm::Type {name_, 0};
    }

    const std::string &GetFunctionName() const
    {
        ASSERT(finished_);
        return fn_name_;
    }

    void AddParameter(pandasm::Type param);

    void AddResult(const pandasm::Type &type);

    void Create();

private:
    std::string name_ = "FunctionalInterface";
    std::string fn_name_ = "invoke";
    pandasm::Record rec_ {name_, SourceLanguage::ETS};
    pandasm::Function fn_ {fn_name_, SourceLanguage::ETS};
    bool finished_ = false;
};

class PandasmMethodCreator {
public:
    PandasmMethodCreator(std::string name, TypeCreatorCtx *ctx)
        : ctx_(ctx), name_(name), fn_(std::move(name), SourceLanguage::ETS)
    {
    }

    void AddParameter(pandasm::Type param);

    void AddResult(pandasm::Type type);

    void Create();

    const std::string &GetFunctionName() const
    {
        ASSERT(finished_);
        return name_;
    }

    pandasm::Function &GetFn()
    {
        ASSERT(!finished_);
        return fn_;
    }

    TypeCreatorCtx *Ctx() const
    {
        return ctx_;
    }

private:
    TypeCreatorCtx *ctx_;
    std::string name_;
    pandasm::Function fn_;
    bool finished_ = false;
};

}  // namespace panda::ets

#endif
