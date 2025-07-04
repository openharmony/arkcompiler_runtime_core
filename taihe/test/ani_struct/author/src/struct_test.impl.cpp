/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "struct_test.impl.hpp"
#include "stdexcept"
#include "struct_test.proj.hpp"
#include "taihe/runtime.hpp"

using namespace taihe;

namespace {
    // To be implemented.
    
    class StructDImpl {
    public:
        ::struct_test::DBase5 structD = {1, 2, 3, 4, "5"};
        StructDImpl() {}
    
        void SetStructD(::struct_test::DBase5 const &d)
        {
            this->structD = d;
        }
    
        ::struct_test::DBase5 GetStructD()
        {
            return structD;
        }
    };
    
    class StructEImpl {
    public:
        ::struct_test::EBigint structE = {true, 5.17, 18.00, 2025};
        StructEImpl() {}
    
        void SetStructE(::struct_test::EBigint const &e)
        {
            this->structE = e;
        }
    
        ::struct_test::EBigint GetStructE()
        {
            return structE;
        }
    };
    
    class StructFImpl {
    public:
        ::struct_test::FUnion structF;
        StructFImpl()
            : structF({
                taihe::optional<taihe::string>(std::in_place_t{}, "optval"),
                ::struct_test::UnionF::make_sValue("unionval"),
                taihe::map<taihe::string, taihe::string>()
            })
        {
            this->structF.param3.emplace("rsK", "rsV");
        }
    
        void SetStructF(::struct_test::FUnion const &f)
        {
            this->structF = f;
        }
    
        ::struct_test::FUnion GetStructF()
        {
            return structF;
        }
    };
    
    class StructGImpl {
    public:
        ::struct_test::GArray gArray = {{1, 2, 3, 4, 5}, {1, 2, 3, 4, 5}};
        StructGImpl() {}
    
        void SetGArray(::struct_test::GArray const &sa)
        {
            this->gArray = sa;
        }
    
        ::struct_test::GArray GetGArray()
        {
            return gArray;
        }
    };
    
    class SDB5ReadonlyImpl {
    public:
        ::struct_test::DB5Readonly db5 = {1, 2, 3, 4, "5"};
        SDB5ReadonlyImpl() {}
    
        void SetDB5Readonly(::struct_test::DB5Readonly const &d)
        {
            this->db5 = d;
        }
    
        ::struct_test::DB5Readonly GetDB5Readonly()
        {
            return db5;
        }
    };
    
    class SEBReadonlyImpl {
    public:
        ::struct_test::EBigintReadonly ebr = {true, 5.17, 18.00, 2025};
        SEBReadonlyImpl() {}
    
        void SetSEBReadonly(::struct_test::EBigintReadonly const &e)
        {
            this->ebr = e;
        }
    
        ::struct_test::EBigintReadonly GetSEBReadonly()
        {
            return ebr;
        }
    };
    
    ::struct_test::StructD GetStructD()
    {
        return taihe::make_holder<StructDImpl, ::struct_test::StructD>();
    }
    
    ::struct_test::StructE GetStructE()
    {
        return taihe::make_holder<StructEImpl, ::struct_test::StructE>();
    }
    
    ::struct_test::StructF GetStructF()
    {
        return taihe::make_holder<StructFImpl, ::struct_test::StructF>();
    }
    
    ::struct_test::StructG GetStructG()
    {
        return taihe::make_holder<StructGImpl, ::struct_test::StructG>();
    }
    
    ::struct_test::Canvas addNewCanvas(::taihe::string_view name)
    {
        return {{255, 0, 255}, name};
    }
    
    ::struct_test::SDB5Readonly GetSDB5Readonly()
    {
        return taihe::make_holder<SDB5ReadonlyImpl, ::struct_test::SDB5Readonly>();
    }
    
    ::struct_test::SEBReadonly GetSEBReadonly()
    {
        return taihe::make_holder<SEBReadonlyImpl, ::struct_test::SEBReadonly>();
    }
    
    ::struct_test::Draw AddNewDraw(::taihe::string_view drawName)
    {
        return {{255, 0, 255}, drawName};
        ;
    }
    }  // namespace
    
    TH_EXPORT_CPP_API_GetStructD(GetStructD);
    TH_EXPORT_CPP_API_GetStructE(GetStructE);
    TH_EXPORT_CPP_API_GetStructF(GetStructF);
    TH_EXPORT_CPP_API_GetStructG(GetStructG);
    TH_EXPORT_CPP_API_addNewCanvas(addNewCanvas);
    TH_EXPORT_CPP_API_GetSDB5Readonly(GetSDB5Readonly);
    TH_EXPORT_CPP_API_GetSEBReadonly(GetSEBReadonly);
    TH_EXPORT_CPP_API_AddNewDraw(AddNewDraw);