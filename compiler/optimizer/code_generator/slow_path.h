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

#ifndef PANDA_SLOW_PATH_H
#define PANDA_SLOW_PATH_H

#include "optimizer/ir/graph.h"
#include "optimizer/code_generator/encode.h"

namespace panda::compiler {
class Codegen;

class SlowPathBase {
public:
    using EntrypointId = RuntimeInterface::EntrypointId;

    explicit SlowPathBase(LabelHolder::LabelId label) : SlowPathBase(label, nullptr) {}
    SlowPathBase(LabelHolder::LabelId label, Inst *inst)
        : label_(label), label_back_(LabelHolder::INVALID_LABEL), inst_(inst)
    {
    }
    virtual ~SlowPathBase() = default;

    NO_COPY_SEMANTIC(SlowPathBase);
    NO_MOVE_SEMANTIC(SlowPathBase);

    Inst *GetInst() const
    {
        return inst_;
    }

    auto &GetLabel() const
    {
        return label_;
    }

    void BindBackLabel(Encoder *encoder)
    {
        if (!encoder->IsLabelValid(label_back_)) {
            label_back_ = encoder->CreateLabel();
        }
        encoder->BindLabel(label_back_);
    }

    void CreateBackLabel(Encoder *encoder)
    {
        label_back_ = encoder->CreateLabel();
    }

    LabelHolder::LabelId GetBackLabel()
    {
        return label_back_;
    }

    void Generate(Codegen *codegen);

    virtual void GenerateImpl(Codegen *codegen) = 0;

private:
    LabelHolder::LabelId label_ {};
    LabelHolder::LabelId label_back_ {};
    Inst *inst_ {nullptr};

#ifndef NDEBUG
    // Intended to check that slow path is generated only once.
    bool generated_ {false};
#endif
};

class SlowPathIntrinsic : public SlowPathBase {
public:
    using SlowPathBase::SlowPathBase;

    SlowPathIntrinsic(LabelHolder::LabelId label, Inst *inst) : SlowPathBase(label, inst) {}
    ~SlowPathIntrinsic() override = default;

    NO_COPY_SEMANTIC(SlowPathIntrinsic);
    NO_MOVE_SEMANTIC(SlowPathIntrinsic);

    void GenerateImpl(Codegen *codegen) override;
};

class SlowPathEntrypoint : public SlowPathBase {
public:
    explicit SlowPathEntrypoint(LabelHolder::LabelId label) : SlowPathBase(label) {}
    SlowPathEntrypoint(LabelHolder::LabelId label, Inst *inst, EntrypointId eid)
        : SlowPathBase(label, inst), entrypoint_(eid)
    {
    }
    ~SlowPathEntrypoint() override = default;

    void GenerateImpl(Codegen *codegen) override;

    EntrypointId GetEntrypoint()
    {
        return entrypoint_;
    }

    NO_COPY_SEMANTIC(SlowPathEntrypoint);
    NO_MOVE_SEMANTIC(SlowPathEntrypoint);

private:
    bool GenerateThrowOutOfBoundsException(Codegen *codegen);
    bool GenerateInitializeClass(Codegen *codegen);
    bool GenerateIsInstance(Codegen *codegen);
    bool GenerateCheckCast(Codegen *codegen);
    bool GenerateDeoptimize(Codegen *codegen);
    bool GenerateByEntry(Codegen *codegen);
    bool GenerateCreateObject(Codegen *codegen);

private:
    EntrypointId entrypoint_ {EntrypointId::COUNT};
};

class SlowPathImplicitNullCheck : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;

    void GenerateImpl(Codegen *codegen) override;
};

class SlowPathShared : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;

    void GenerateImpl(Codegen *codegen) override;
    void SetTmpReg(Reg reg)
    {
        tmp_reg_ = reg;
    }
    Reg GetTmpReg() const
    {
        return tmp_reg_;
    }

private:
    Reg tmp_reg_ {INVALID_REGISTER};
};

class SlowPathResolveStringAot : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;

    void GenerateImpl(Codegen *codegen) override;

    void SetDstReg(Reg reg)
    {
        dst_reg_ = reg;
    }

    void SetAddrReg(Reg reg)
    {
        addr_reg_ = reg;
    }

    void SetStringId(uint32_t string_id)
    {
        string_id_ = string_id;
    }

    void SetMethod(void *method)
    {
        method_ = method;
    }

private:
    Reg dst_reg_ {INVALID_REGISTER};
    Reg addr_reg_ {INVALID_REGISTER};
    uint32_t string_id_ {0};
    void *method_ {nullptr};
};

class SlowPathCheck : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;
};

class SlowPathCheckCast : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;
    void GenerateImpl(Codegen *codegen) override;
    void SetClassReg(Reg reg)
    {
        class_reg_ = reg;
    }

private:
    Reg class_reg_ {INVALID_REGISTER};
};

class SlowPathAbstract : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;
    void GenerateImpl(Codegen *codegen) override;
    void SetMethodReg(Reg reg)
    {
        method_reg_ = reg;
    }

private:
    Reg method_reg_ {INVALID_REGISTER};
};

class SlowPathRefCheck : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;

    void GenerateImpl(Codegen *codegen) override;

    void SetRegs(Reg array_reg, Reg ref_reg)
    {
        array_reg_ = array_reg;
        ref_reg_ = ref_reg;
    }

private:
    Reg array_reg_ {INVALID_REGISTER};
    Reg ref_reg_ {INVALID_REGISTER};
};

class SlowPathUnresolved : public SlowPathEntrypoint {
public:
    using SlowPathEntrypoint::SlowPathEntrypoint;

    void GenerateImpl(Codegen *codegen) override;

    void SetUnresolvedType(void *method, uint32_t type_id)
    {
        method_ = method;
        type_id_ = type_id;
    }

    void SetSlotAddr(uintptr_t addr)
    {
        slot_addr_ = addr;
    }

    void SetDstReg(Reg dst_reg)
    {
        dst_reg_ = dst_reg;
    }

    void SetArgReg(Reg arg_reg)
    {
        arg_reg_ = arg_reg;
    }

private:
    Reg dst_reg_ {INVALID_REGISTER};
    Reg arg_reg_ {INVALID_REGISTER};
    void *method_ {nullptr};
    uint32_t type_id_ {0};
    uintptr_t slot_addr_ {0};
};
}  // namespace panda::compiler

#endif  // PANDA_SLOW_PATH_H
