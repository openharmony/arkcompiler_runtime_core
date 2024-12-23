/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef CPP_ABCKIT_ARKTS_MODULE_IMPL_H
#define CPP_ABCKIT_ARKTS_MODULE_IMPL_H

#include "./module.h"
#include "./import_descriptor.h"
#include "../core/import_descriptor.h"
#include "./annotation_interface.h"
#include "../core/annotation_interface.h"

// NOLINTBEGIN(performance-unnecessary-value-param)
namespace abckit::arkts {

inline AbckitArktsModule *Module::TargetCast() const
{
    auto ret = GetApiConfig()->cArktsIapi_->coreModuleToArktsModule(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline Module::Module(const core::Module &coreOther) : core::Module(coreOther), targetChecker_(this) {}

inline arkts::ImportDescriptor Module::AddImportFromArktsV1ToArktsV1(Module imported, std::string_view name,
                                                                     std::string_view alias) const
{
    const AbckitArktsImportFromDynamicModuleCreateParams params {name.data(), alias.data()};
    AbckitArktsImportDescriptor *arktsid =
        GetApiConfig()->cArktsMapi_->moduleAddImportFromArktsV1ToArktsV1(TargetCast(), imported.TargetCast(), &params);
    CheckError(GetApiConfig());
    AbckitCoreImportDescriptor *coreid =
        GetApiConfig()->cArktsIapi_->arktsImportDescriptorToCoreImportDescriptor(arktsid);
    CheckError(GetApiConfig());
    return arkts::ImportDescriptor(core::ImportDescriptor(coreid, GetApiConfig(), GetResource()));
}

inline arkts::ExportDescriptor Module::AddExportFromArktsV1ToArktsV1(arkts::Module exported, std::string_view name,
                                                                     std::string_view alias) const
{
    const AbckitArktsDynamicModuleExportCreateParams params {name.data(), alias.data()};
    auto arktsExported = exported.TargetCast();
    auto arktsEd =
        GetApiConfig()->cArktsMapi_->moduleAddExportFromArktsV1ToArktsV1(TargetCast(), arktsExported, &params);
    CheckError(GetApiConfig());
    auto coreEd = GetApiConfig()->cArktsIapi_->arktsExportDescriptorToCoreExportDescriptor(arktsEd);
    CheckError(GetApiConfig());
    return ExportDescriptor(core::ExportDescriptor(coreEd, GetApiConfig(), GetResource()));
}

inline Module Module::RemoveImport(arkts::ImportDescriptor id) const
{
    auto arktsId = id.TargetCast();
    auto arktsMod = TargetCast();
    GetApiConfig()->cArktsMapi_->moduleRemoveImport(arktsMod, arktsId);
    CheckError(GetApiConfig());
    return *this;
}

inline Module Module::RemoveExport(arkts::ExportDescriptor desc) const
{
    auto arktsId = desc.TargetCast();
    auto arktsMod = TargetCast();
    GetApiConfig()->cArktsMapi_->moduleRemoveExport(arktsMod, arktsId);
    CheckError(GetApiConfig());
    return *this;
}

inline arkts::AnnotationInterface Module::AddAnnotationInterface(std::string_view name) const
{
    const AbckitArktsAnnotationInterfaceCreateParams params {name.data()};
    auto arktsMod = TargetCast();
    auto arktsAi = GetApiConfig()->cArktsMapi_->moduleAddAnnotationInterface(arktsMod, &params);
    CheckError(GetApiConfig());
    auto coreAi = GetApiConfig()->cArktsIapi_->arktsAnnotationInterfaceToCoreAnnotationInterface(arktsAi);
    CheckError(GetApiConfig());
    return AnnotationInterface(core::AnnotationInterface(coreAi, GetApiConfig(), GetResource()));
}

}  // namespace abckit::arkts
// NOLINTEND(performance-unnecessary-value-param)

#endif  // CPP_ABCKIT_ARKTS_MODULE_IMPL_H
