/**
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

#ifndef LIBABCKIT_STD_METADATA_INSPECT_IMPL_H
#define LIBABCKIT_STD_METADATA_INSPECT_IMPL_H

#include "libabckit/include/c/metadata_core.h"
#include <cstdint>
#include <variant>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

namespace panda::pandasm {
struct Program;
struct Function;
struct Record;
struct LiteralArray;
}  // namespace panda::pandasm
namespace ark::pandasm {
struct Function;
struct Program;
struct Record;
struct LiteralArray;
}  // namespace ark::pandasm

namespace libabckit {

enum class Mode {
    DYNAMIC,
    STATIC,
};

struct pandasm_Literal;
struct pandasm_Value;

}  // namespace libabckit

/* ====================
 * Internal implementations for all of the Abckit* structures
 */

/*
 * NOTE: after v1 releases, move this enum to the user headers, add static types
 * NOTE: come up with a better name
 */
enum AbckitImportExportDescriptorKind {
    UNTYPED,  // Applies to: JavaScript
};

enum AbckitDynamicExportKind {
    ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT,
    ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT,
    ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT,
};

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
using AbckitValueImplT = std::unique_ptr<libabckit::pandasm_Value, void (*)(libabckit::pandasm_Value *)>;
struct AbckitValue {
    AbckitFile *file = nullptr;
    /*
     * Underlying implementation
     */
    AbckitValueImplT val;
    AbckitValue(AbckitFile *file, AbckitValueImplT val) : file(file), val(std::move(val)) {}
};

struct AbckitArktsAnnotationInterface {
    /*
     * Points to the pandasm::Record that stores annotation definition.
     */
    std::variant<panda::pandasm::Record *, ark::pandasm::Record *> impl;

    ark::pandasm::Record *GetStaticImpl()
    {
        return std::get<ark::pandasm::Record *>(impl);
    }

    panda::pandasm::Record *GetDynamicImpl()
    {
        return std::get<panda::pandasm::Record *>(impl);
    }

    AbckitCoreAnnotationInterface *core = nullptr;
};

struct AbckitCoreAnnotationInterface {
    /*
     * To refer to the properties of the origin module.
     */
    AbckitCoreModule *owningModule = nullptr;

    /*
     * Contains annotation interface fields
     */
    std::vector<std::unique_ptr<AbckitCoreAnnotationInterfaceField>> fields;

    std::variant<std::unique_ptr<AbckitArktsAnnotationInterface>> impl;
    AbckitArktsAnnotationInterface *GetArkTSImpl()
    {
        return std::get<std::unique_ptr<AbckitArktsAnnotationInterface>>(impl).get();
    }
};

struct AbckitArktsAnnotationInterfaceField {
    AbckitCoreAnnotationInterfaceField *core = nullptr;
};

struct AbckitCoreAnnotationInterfaceField {
    /*
     * To refer to the properties of the origin annotation interface.
     */
    AbckitCoreAnnotationInterface *ai = nullptr;

    /*
     * Field name
     */
    AbckitString *name = nullptr;

    /*
     * Field type
     */
    AbckitType *type = nullptr;

    /*
     * Field value
     */
    AbckitValue *value = nullptr;

    std::variant<std::unique_ptr<AbckitArktsAnnotationInterfaceField>> impl;
    AbckitArktsAnnotationInterfaceField *GetArkTSImpl()
    {
        return std::get<std::unique_ptr<AbckitArktsAnnotationInterfaceField>>(impl).get();
    }
};

struct AbckitArktsAnnotationElement {
    AbckitCoreAnnotationElement *core = nullptr;
};

struct AbckitCoreAnnotationElement {
    /*
     * To refer to the properties of the origin annotation.
     */
    AbckitCoreAnnotation *ann = nullptr;

    /*
     * Name of annotation element
     */
    AbckitString *name = nullptr;

    /*
     * Value stored in annotation
     */
    AbckitValue *value = nullptr;

    std::variant<std::unique_ptr<AbckitArktsAnnotationElement>> impl;
    AbckitArktsAnnotationElement *GetArkTSImpl()
    {
        return std::get<std::unique_ptr<AbckitArktsAnnotationElement>>(impl).get();
    }
};

struct AbckitArktsAnnotation {
    AbckitCoreAnnotation *core = nullptr;
};

struct AbckitCoreAnnotation {
    /*
     * To refer to the properties of the annotation interface
     */
    AbckitCoreAnnotationInterface *ai = nullptr;

    /*
     * To refer to the properties of the owner.
     */
    std::variant<AbckitCoreClass *, AbckitCoreFunction *> owner;

    /*
     * Name of the annotation
     */
    AbckitString *name = nullptr;

    /*
     * Annotation elements
     */
    std::vector<std::unique_ptr<AbckitCoreAnnotationElement>> elements;

    std::variant<std::unique_ptr<AbckitArktsAnnotation>> impl;
    AbckitArktsAnnotation *GetArkTSImpl()
    {
        return std::get<std::unique_ptr<AbckitArktsAnnotation>>(impl).get();
    }
};

struct AbckitArktsClassPayload {
    /*
     * In Arkts1 class is defined by it's constructor.
     * In Arkts2 class is defined by corresponding pandasm::Record.
     */
    std::variant<panda::pandasm::Function *, ark::pandasm::Record *> cl;

    ark::pandasm::Record *GetStaticClass()
    {
        return std::get<ark::pandasm::Record *>(cl);
    }
    panda::pandasm::Function *GetDynamicClass()
    {
        return std::get<panda::pandasm::Function *>(cl);
    }
};

struct AbckitArktsClass {
    AbckitArktsClassPayload impl;
    AbckitCoreClass *core = nullptr;

    explicit AbckitArktsClass(panda::pandasm::Function *function)
    {
        impl.cl = reinterpret_cast<panda::pandasm::Function *>(function);
    };

    explicit AbckitArktsClass(ark::pandasm::Record *record)
    {
        impl.cl = reinterpret_cast<ark::pandasm::Record *>(record);
    };
};

struct AbckitJsClass {
    panda::pandasm::Function *impl;
    AbckitCoreClass *core = nullptr;

    explicit AbckitJsClass(panda::pandasm::Function *function)
    {
        impl = reinterpret_cast<panda::pandasm::Function *>(function);
    };
};

struct AbckitCoreClass {
    /*
     * To refer to the properties of the origin module.
     */
    AbckitCoreModule *owningModule = nullptr;

    /*
     * To refer to the properties of the parent namepsace.
     */
    AbckitCoreNamespace *parentNamespace = nullptr;

    /*
     * To refer to the properties of the parent function.
     */
    AbckitCoreFunction *parentFunction = nullptr;

    /*
     * To store class methods.
     */
    std::vector<std::unique_ptr<AbckitCoreFunction>> methods {};

    /*
     * To store links to the wrapped annotations.
     */
    std::vector<std::unique_ptr<AbckitCoreAnnotation>> annotations;

    /*
     * Language-dependent implementation to store class data.
     */
    std::variant<std::unique_ptr<AbckitJsClass>, std::unique_ptr<AbckitArktsClass>> impl;
    AbckitArktsClass *GetArkTSImpl()
    {
        return std::get<std::unique_ptr<AbckitArktsClass>>(impl).get();
    }
    AbckitJsClass *GetJsImpl()
    {
        return std::get<std::unique_ptr<AbckitJsClass>>(impl).get();
    }

    AbckitCoreClass(AbckitCoreModule *module, AbckitJsClass klass)
    {
        klass.core = this;
        impl = std::make_unique<AbckitJsClass>(klass);
        owningModule = module;
    }
    AbckitCoreClass(AbckitCoreModule *module, AbckitArktsClass klass)
    {
        klass.core = this;
        impl = std::make_unique<AbckitArktsClass>(klass);
        owningModule = module;
    }
};

struct AbckitJsFunction {
    AbckitCoreFunction *core = nullptr;
    panda::pandasm::Function *impl = nullptr;
};

struct AbckitArktsFunction {
    std::variant<panda::pandasm::Function *, ark::pandasm::Function *> impl;

    ark::pandasm::Function *GetStaticImpl()
    {
        return std::get<ark::pandasm::Function *>(impl);
    }
    panda::pandasm::Function *GetDynamicImpl()
    {
        return std::get<panda::pandasm::Function *>(impl);
    }

    AbckitCoreFunction *core = nullptr;
};

struct AbckitCoreFunction {
    /*
     * To refer to the properties of the origin module.
     */
    AbckitCoreModule *owningModule = nullptr;

    /*
     * To refer to the properties of the parent namepsace.
     */
    AbckitCoreNamespace *parentNamespace = nullptr;

    /*
     * To be able to refer to the class where method is defined.
     * For global functions the rules are as follows:
     * - Dynamic: pandasm::Function with js file name.
     * - Static: pandasm::Record with name `L/.../ETSGLOBAL`.
     */
    AbckitCoreClass *parentClass = nullptr;

    /*
     * To be able to refer to the class where method is defined.
     */
    AbckitCoreFunction *parentFunction = nullptr;

    /*
     * To store links to the wrapped annotations.
     */
    std::vector<std::unique_ptr<AbckitCoreAnnotation>> annotations;

    std::vector<std::unique_ptr<AbckitCoreFunction>> nestedFunction;
    std::vector<std::unique_ptr<AbckitCoreClass>> nestedClasses;

    bool isAnonymous = false;

    std::variant<std::unique_ptr<AbckitJsFunction>, std::unique_ptr<AbckitArktsFunction>> impl;

    AbckitArktsFunction *GetArkTSImpl()
    {
        return std::get<std::unique_ptr<AbckitArktsFunction>>(impl).get();
    }
    AbckitJsFunction *GetJsImpl()
    {
        return std::get<std::unique_ptr<AbckitJsFunction>>(impl).get();
    }
};

struct AbckitArktsNamespace {
    AbckitCoreNamespace *core = nullptr;
    /*
     * To store links to the wrapped methods.
     */
    std::unique_ptr<AbckitCoreFunction> f;
};

struct AbckitCoreNamespace {
    explicit AbckitCoreNamespace(AbckitCoreModule *owningModule) : owningModule(owningModule) {}

    /*
     * To refer to the properties of the origin module.
     */
    AbckitCoreModule *owningModule = nullptr;

    /*
     * To be able to refer to the namespace where method is defined.
     */
    AbckitCoreNamespace *parentNamespace = nullptr;

    /*
     * To store links to the wrapped methods.
     */
    std::vector<std::unique_ptr<AbckitCoreFunction>> functions;

    /*
     * To store links to the wrapped classes.
     */
    std::vector<std::unique_ptr<AbckitCoreClass>> classes;
    /*
     * To store links to the wrapped namespaces.
     */
    std::vector<std::unique_ptr<AbckitCoreNamespace>> namespaces;

    std::variant<std::unique_ptr<AbckitArktsNamespace>> impl;

    AbckitArktsNamespace *GetArkTSImpl()
    {
        return std::get<std::unique_ptr<AbckitArktsNamespace>>(impl).get();
    }
};

struct AbckitModulePayloadDyn {
    /*
     * In JS module is defined by corresponding pandasm::Record and AbckitLiteralArray.
     */
    const panda::pandasm::Record *record = nullptr;
    AbckitLiteralArray *moduleLiteralArray = nullptr;
    AbckitLiteralArray *scopeNamesLiteralArray = nullptr;
    bool absPaths = false;
    size_t moduleRequestsOffset = 0;
    size_t regularImportsOffset = 0;
    size_t namespaceImportsOffset = 0;
    size_t localExportsOffset = 0;
    size_t indirectExportsOffset = 0;
    size_t starExportsOffset = 0;
};

struct AbckitModulePayload {
    /*
     * Some data structure for STS which should store module's name and other data.
     */
    std::variant<AbckitModulePayloadDyn> impl;
    /*
     * Implementation for JS.
     */
    AbckitModulePayloadDyn &GetDynModule()
    {
        return std::get<AbckitModulePayloadDyn>(impl);
    }
};

struct AbckitArktsModule {
    AbckitModulePayload impl;
    AbckitCoreModule *core = nullptr;
};

struct AbckitJsModule {
    AbckitModulePayloadDyn impl {};
    AbckitCoreModule *core = nullptr;
};

struct AbckitCoreModule {
    /*
     * To refer to the properties of the original `.abc` file, such as is it dynamic, etc.
     */
    AbckitFile *file = nullptr;

    /*
     * Stores module's dependencies.
     * NOTE: For JS index here must match the index in `module_requests`.
     */
    std::vector<AbckitCoreModule *> md;

    /*
     * Stores module's imports.
     * NOTE: For JS will need to perform linear search to find needed import,
     * because namespace imports and regular imports are stored together here.
     */
    std::vector<std::unique_ptr<AbckitCoreImportDescriptor>> id;

    /*
     * Stores module's exports.
     * NOTE: For JS will need to perform linear search to find needed import,
     * because local exports, indirect exports and star exports are stored together here.
     */
    std::vector<std::unique_ptr<AbckitCoreExportDescriptor>> ed;

    /*
     * Tables to store and find wrapped entities by their name.
     */
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreClass>> ct;
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreAnnotationInterface>> at;
    std::vector<std::unique_ptr<AbckitCoreNamespace>> namespaces;

    /*
     * Only stores top level functions.
     */
    std::vector<std::unique_ptr<AbckitCoreFunction>> functions;

    /*
     * Current module's name.
     */
    AbckitString *moduleName = nullptr;

    /*
     * Indicator whether current module is local or external.
     */
    bool isExternal = false;

    /*
     * Target language of current module
     */
    AbckitTarget target = ABCKIT_TARGET_UNKNOWN;

    /*
     * Language-dependent implementation to store module data.
     */
    std::variant<std::unique_ptr<AbckitJsModule>, std::unique_ptr<AbckitArktsModule>> impl;
    AbckitArktsModule *GetArkTSImpl()
    {
        return std::get<std::unique_ptr<AbckitArktsModule>>(impl).get();
    }
    AbckitJsModule *GetJsImpl()
    {
        return std::get<std::unique_ptr<AbckitJsModule>>(impl).get();
    }

    void InsertClass(const std::string &name, std::unique_ptr<AbckitCoreClass> &&klass)
    {
        ct.emplace(name, std::move(klass));
    }
};

struct AbckitString {
    std::string_view impl;
};

using AbckitLiteralImplT = std::unique_ptr<libabckit::pandasm_Literal, void (*)(libabckit::pandasm_Literal *)>;
struct AbckitLiteral {
    AbckitLiteral(AbckitFile *file, AbckitLiteralImplT val) : file(file), val(std::move(val)) {}
    AbckitFile *file;
    AbckitLiteralImplT val;
};

struct AbckitLiteralArray {
    AbckitFile *file = nullptr;
    std::variant<ark::pandasm::LiteralArray *, panda::pandasm::LiteralArray *> impl;

    AbckitLiteralArray() = default;

    AbckitLiteralArray(AbckitFile *f, ark::pandasm::LiteralArray *laImpl)
    {
        impl = laImpl;
        file = f;
    }

    AbckitLiteralArray(AbckitFile *f, panda::pandasm::LiteralArray *laImpl)
    {
        impl = laImpl;
        file = f;
    }

    ark::pandasm::LiteralArray *GetStaticImpl() const
    {
        return std::get<ark::pandasm::LiteralArray *>(impl);
    }
    panda::pandasm::LiteralArray *GetDynamicImpl() const
    {
        return std::get<panda::pandasm::LiteralArray *>(impl);
    }
};

struct AbckitType {
    AbckitTypeId id = ABCKIT_TYPE_ID_INVALID;
    size_t rank = 0;
    AbckitCoreClass *klass = nullptr;
};

struct AbckitFile {
    struct AbcKitLiterals {
        std::unordered_map<bool, std::unique_ptr<AbckitLiteral>> boolLits;
        std::unordered_map<uint8_t, std::unique_ptr<AbckitLiteral>> u8Lits;
        std::unordered_map<uint16_t, std::unique_ptr<AbckitLiteral>> u16Lits;
        std::unordered_map<uint16_t, std::unique_ptr<AbckitLiteral>> methodAffilateLits;
        std::unordered_map<uint32_t, std::unique_ptr<AbckitLiteral>> u32Lits;
        std::unordered_map<uint64_t, std::unique_ptr<AbckitLiteral>> u64Lits;
        std::unordered_map<float, std::unique_ptr<AbckitLiteral>> floatLits;
        std::unordered_map<double, std::unique_ptr<AbckitLiteral>> doubleLits;
        std::unordered_map<std::string, std::unique_ptr<AbckitLiteral>> litArrLits;
        std::unordered_map<std::string, std::unique_ptr<AbckitLiteral>> stringLits;
        std::unordered_map<std::string, std::unique_ptr<AbckitLiteral>> methodLits;
    };
    struct AbcKitValues {
        std::unordered_map<bool, std::unique_ptr<AbckitValue>> boolVals;
        std::unordered_map<double, std::unique_ptr<AbckitValue>> doubleVals;
        std::unordered_map<std::string, std::unique_ptr<AbckitValue>> stringVals;
        std::unordered_map<std::string, std::unique_ptr<AbckitValue>> litarrVals;
    };

    libabckit::Mode frontend = libabckit::Mode::DYNAMIC;

    /*
     * Table to store wrapped internal modules.
     */
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> localModules;

    /*
     * Table to store wrapped external modules.
     */
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> externalModules;

    /*
     * To store the original program and update it.
     */
    std::variant<panda::pandasm::Program *, ark::pandasm::Program *> program;

    ark::pandasm::Program *GetStaticProgram()
    {
        return std::get<ark::pandasm::Program *>(program);
    }
    panda::pandasm::Program *GetDynamicProgram()
    {
        return std::get<panda::pandasm::Program *>(program);
    }

    AbcKitLiterals literals;
    std::unordered_map<size_t, std::unique_ptr<AbckitType>> types;
    AbcKitValues values;
    std::vector<std::unique_ptr<AbckitLiteralArray>> litarrs;

    /*
     * To store all program strings
     */
    std::unordered_map<std::string, std::unique_ptr<AbckitString>> strings;

    std::unordered_map<std::string, AbckitCoreFunction *> nameToFunction;

    /*
     * To store the .abc file version
     */
    AbckitFileVersion version = nullptr;

    void *internal = nullptr;
};

struct AbckitDynamicImportDescriptorPayload {
    /*
     * Indicator whether import is namespace or regular.
     */
    bool isRegularImport = false;
    /*
     * Offset in the corresponding `XXX_imports` in mainline.
     */
    uint32_t moduleRecordIndexOff = 0;
};

struct AbckitStaticImportDescriptorPayload {
    std::variant<AbckitCoreAnnotationInterface *, AbckitCoreClass *, AbckitCoreFunction *, AbckitCoreField *> impl;
    /*
     * Implementation for AbckitImportExportDescriptorKind::ANNOTATION.
     */
    AbckitCoreAnnotationInterface *GetAnnotationInterfacePayload()
    {
        return std::get<AbckitCoreAnnotationInterface *>(impl);
    }
    /*
     * Implementation for AbckitImportExportDescriptorKind::CLASS.
     */
    AbckitCoreClass *GetClassPayload()
    {
        return std::get<AbckitCoreClass *>(impl);
    }
    /*
     * Implementation for AbckitImportExportDescriptorKind::FUNCTION.
     */
    AbckitCoreFunction *GetFunctionPayload()
    {
        return std::get<AbckitCoreFunction *>(impl);
    }
    /*
     * Implementation for AbckitImportExportDescriptorKind::FIELD.
     */
    AbckitCoreField *GetFieldPayload()
    {
        return std::get<AbckitCoreField *>(impl);
    }
};

struct AbckitCoreImportDescriptorPayload {
    std::variant<AbckitDynamicImportDescriptorPayload, AbckitStaticImportDescriptorPayload> impl;
    /*
     * Implementation for AbckitImportExportDescriptorKind::UNTYPED.
     */
    AbckitDynamicImportDescriptorPayload &GetDynId()
    {
        return std::get<AbckitDynamicImportDescriptorPayload>(impl);
    }
    /*
     * Implementation for static kinds.
     */
    AbckitStaticImportDescriptorPayload GetStatId()
    {
        return std::get<AbckitStaticImportDescriptorPayload>(impl);
    }
};

struct AbckitArktsImportDescriptor {
    /*
     * Kind of the imported entity.
     * Use AbckitImportExportDescriptorKind::UNTYPED for imports from dynamic modules.
     * Other kinds are used for imports from static modules.
     */
    AbckitImportExportDescriptorKind kind = UNTYPED;
    /*
     * Data needed to work with the import.
     */
    AbckitCoreImportDescriptorPayload payload;
    AbckitCoreImportDescriptor *core = nullptr;
};

struct AbckitJsImportDescriptor {
    /*
     * Kind of the imported entity.
     * Use AbckitImportExportDescriptorKind::UNTYPED for imports from dynamic modules.
     * Other kinds are used for imports from static modules.
     */
    AbckitImportExportDescriptorKind kind = UNTYPED;
    /*
     * Data needed to work with the import.
     */
    AbckitCoreImportDescriptorPayload payload;
    AbckitCoreImportDescriptor *core = nullptr;
};

struct AbckitCoreImportDescriptor {
    /*
     * Back link to the importing module.
     */
    AbckitCoreModule *importingModule = nullptr;
    /*
     * Link to the module from which entity is imported.
     */
    AbckitCoreModule *importedModule = nullptr;

    std::variant<std::unique_ptr<AbckitJsImportDescriptor>, std::unique_ptr<AbckitArktsImportDescriptor>> impl;
    AbckitArktsImportDescriptor *GetArkTSImpl()
    {
        return std::get<std::unique_ptr<AbckitArktsImportDescriptor>>(impl).get();
    }
    AbckitJsImportDescriptor *GetJsImpl()
    {
        return std::get<std::unique_ptr<AbckitJsImportDescriptor>>(impl).get();
    }
};

struct AbckitDynamicExportDescriptorPayload {
    /*
     * The kind of export. Used to determine where to look by the index.
     */
    AbckitDynamicExportKind kind = ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT;
    /*
     * For special StarExport case 'export * as <StarExport> from "..."'.
     * It converts to NamespaceImport + LocalExport:
     * import * as =ens{$i} from "...";
     * export =ens{$i} as <StarExport>;
     */
    bool hasServiceImport = false;
    /*
     * For special StarExport case 'export * as <StarExport> from "..."'.
     */
    size_t serviceNamespaceImportIdx = 0;
    /*
     * Offset in the corresponding `XXX_exports` (depends on the `kind`) in mainline.
     */
    uint32_t moduleRecordIndexOff = 0;
};

struct AbckitCoreExportDescriptorPayload {
    std::variant<AbckitDynamicExportDescriptorPayload, AbckitCoreAnnotationInterface *, AbckitCoreClass *,
                 AbckitCoreFunction *, AbckitCoreField *>
        impl;

    /*
     * Implementation for AbckitImportExportDescriptorKind::UNTYPED.
     */
    AbckitDynamicExportDescriptorPayload &GetDynamicPayload()
    {
        return std::get<AbckitDynamicExportDescriptorPayload>(impl);
    }
    /*
     * Payload for AbckitImportExportDescriptorKind::ANNOTATION.
     * Should point to the LOCAL entity.
     */
    AbckitCoreAnnotationInterface *GetAnnotationInterfacePayload()
    {
        return std::get<AbckitCoreAnnotationInterface *>(impl);
    }
    /*
     * Payload for AbckitImportExportDescriptorKind::CLASS.
     * Should point to the LOCAL entity.
     */
    AbckitCoreClass *GetClassPayload()
    {
        return std::get<AbckitCoreClass *>(impl);
    }
    /*
     * Payload for AbckitImportExportDescriptorKind::FUNCTION.
     * Should point to the LOCAL entity.
     */
    AbckitCoreFunction *GetFunctionPayload()
    {
        return std::get<AbckitCoreFunction *>(impl);
    }
    /*
     * Payload for AbckitImportExportDescriptorKind::FIELD.
     * Should point to the LOCAL entity.
     */
    AbckitCoreField *GetFieldPayload()
    {
        return std::get<AbckitCoreField *>(impl);
    }
};

struct AbckitArktsExportDescriptor {
    AbckitCoreExportDescriptor *core = nullptr;
    /*
     * Kind of the exported entity.
     * Use AbckitImportExportDescriptorKind::UNTYPED for exports from dynamic modules.
     * Other kinds are used for exports from static modules.
     * NOTE: Use same enum as import API here. May need to rename this enum
     * when implementing v2 API to something neutral, like AbckitDescriptorKind.
     */
    AbckitImportExportDescriptorKind kind = UNTYPED;
    /*
     * Data needed to work with the import.
     */
    AbckitCoreExportDescriptorPayload payload;
};

struct AbckitJsExportDescriptor {
    AbckitCoreExportDescriptor *core = nullptr;
    /*
     * Kind of the exported entity.
     * Use AbckitImportExportDescriptorKind::UNTYPED for exports from dynamic modules.
     * Other kinds are used for exports from static modules.
     * NOTE: Use same enum as import API here. May need to rename this enum
     * when implementing v2 API to something neutral, like AbckitDescriptorKind.
     */
    AbckitImportExportDescriptorKind kind = UNTYPED;
    /*
     * Data needed to work with the import.
     */
    AbckitCoreExportDescriptorPayload payload;
};

struct AbckitCoreExportDescriptor {
    /*
     * Back link to the exporting module.
     */
    AbckitCoreModule *exportingModule = nullptr;
    /*
     * Link to the exported module.
     */
    AbckitCoreModule *exportedModule = nullptr;

    std::variant<std::unique_ptr<AbckitJsExportDescriptor>, std::unique_ptr<AbckitArktsExportDescriptor>> impl;
    AbckitArktsExportDescriptor *GetArkTSImpl()
    {
        return std::get<std::unique_ptr<AbckitArktsExportDescriptor>>(impl).get();
    }
    AbckitJsExportDescriptor *GetJsImpl()
    {
        return std::get<std::unique_ptr<AbckitJsExportDescriptor>>(impl).get();
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

#endif  // LIBABCKIT_STD_METADATA_INSPECT_IMPL_H
