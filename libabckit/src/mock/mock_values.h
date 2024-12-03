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

#ifndef ABCKIT_MOCK_GLOBAL_VALUES
#define ABCKIT_MOCK_GLOBAL_VALUES

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_PATH "abckit.abc"
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_CONST_CHAR "abckit default const char*"
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_BOOL 0x1
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_U8 0x11
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_U16 0x1111
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_U32 0x11111111
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_I32 0x11111112
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_U64 0x1111111122222222
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_I64 0x1111111122222223
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_SIZE_T 0x1111111133333333
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_ENUM_ABCKIT_ISA_TYPE (enum AbckitInst())
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_ENUM_ISA_TYPE AbckitIsaType::ABCKIT_ISA_TYPE_UNSUPPORTED
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_ENUM_ISA_DYNAMIC_CONDITION_TYPE \
    AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_ENUM_ISA_API_DYNAMIC_OPCODE AbckitIsaApiDynamicOpcode::ABCKIT_ISA_API_DYNAMIC_OPCODE_INVALID
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_ENUM_ISA_STATIC_CONDITION_CODE \
    AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_ENUM_TYPE_ID AbckitTypeId::ABCKIT_TYPE_ID_INVALID
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_ENUM_ISA_API_STATIC_OPCODE AbckitIsaApiStaticOpcode::ABCKIT_ISA_API_STATIC_OPCODE_INVALID
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_TYPE_ID AbckitTypeId::ABCKIT_TYPE_ID_INVALID
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_TARGET AbckitTarget::ABCKIT_TARGET_UNKNOWN
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_LITERAL_TAG AbckitLiteralTag::ABCKIT_LITERAL_TAG_INVALID
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_FILE (reinterpret_cast<AbckitFile *>(0xdeadffff))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_GRAPH (reinterpret_cast<AbckitGraph *>(0xdeadeeee))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_VOID (reinterpret_cast<void *>(0xdead0001))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CB (reinterpret_cast<void *>(0xdead0002))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_MODULE (reinterpret_cast<AbckitCoreModule *>(0xdead0003))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_STRING (reinterpret_cast<AbckitString *>(0xdead0005))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_NAMESPACE (reinterpret_cast<AbckitCoreNamespace *>(0xdead0006))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_IMPORT_DESCRIPTOR (reinterpret_cast<AbckitCoreImportDescriptor *>(0xdead0006))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_EXPORT_DESCRIPTOR (reinterpret_cast<AbckitCoreExportDescriptor *>(0xdead0007))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_CLASS (reinterpret_cast<AbckitCoreClass *>(0xdead0008))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_ANNOTATION_INTERFACE (reinterpret_cast<AbckitCoreAnnotationInterface *>(0xdead0009))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_ANNOTATION_INTERFACE_FIELD (reinterpret_cast<AbckitCoreAnnotationInterfaceField *>(0xdead0010))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_VALUE (reinterpret_cast<AbckitValue *>(0xdead0011))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_FUNCTION (reinterpret_cast<AbckitCoreFunction *>(0xdead0012))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_ANNOTATION (reinterpret_cast<AbckitCoreAnnotation *>(0xdead0013))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CORE_ANNOTATION_ELEMENT (reinterpret_cast<AbckitCoreAnnotationElement *>(0xdead0014))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_DOUBLE (static_cast<double>(0xdead0016))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_LITERAL_ARRAY (reinterpret_cast<AbckitLiteralArray *>(0xdead0017))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_LITERAL (reinterpret_cast<AbckitLiteral *>(0xdead0018))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_FLOAT (static_cast<float>(0xdead0020))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_TYPE (reinterpret_cast<AbckitType *>(0xdead0021))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_FILE_VERSION (reinterpret_cast<AbckitFileVersion>(0xdead0022))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_BB (reinterpret_cast<AbckitBasicBlock *>(0xdead0023))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_INST (reinterpret_cast<AbckitInst *>(0xdead0024))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_ABCKIT_TYPE (reinterpret_cast<AbckitType *>(0xdead0026))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_MODULE (reinterpret_cast<AbckitArktsModule *>(0xdead00027))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_NAMESPACE (reinterpret_cast<AbckitArktsNamespace *>(0xdead00028))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_IMPORT_DESCRIPTOR (reinterpret_cast<AbckitArktsImportDescriptor *>(0xdead00029))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_FUNCTION (reinterpret_cast<AbckitArktsFunction *>(0xdead00030))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_EXPORT_DESCRIPTOR (reinterpret_cast<AbckitArktsExportDescriptor *>(0xdead00030))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_CLASS (reinterpret_cast<AbckitArktsClass *>(0xdead0031))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_ANNOTATION (reinterpret_cast<AbckitArktsAnnotation *>(0xdead0032))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_ANNOTATION_ELEMENT (reinterpret_cast<AbckitArktsAnnotationElement *>(0xdead0033))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_ANNOTATION_INTERFACE (reinterpret_cast<AbckitArktsAnnotationInterface *>(0xdead00034))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_ANNOTATION_INTERFACE_FIELD (reinterpret_cast<AbckitArktsAnnotationInterfaceField *>(0xdead0035))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_STRUCT_ARKTS_V1_EXTERNAL_MODULE_CREATE_PARAMS \
    (reinterpret_cast<const struct AbckitArktsV1ExternalModuleCreateParams *>(0xdead0039))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_STRUCT_ARKTS_IMPORT_FROM_DYNAMIC_CREATE_PARAMS \
    (reinterpret_cast<const struct AbckitArktsImportFromDynamicModuleCreateParams *>(0xdead0039))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_STRUCT_ARKTS_DYNAMIC_MODULE_EXPORT_CREATE_PARAMS \
    (reinterpret_cast<const struct AbckitArktsDynamicModuleExportCreateParams *>(0xdead0039))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_STRUCT_ARKTS_ANNOTATION_INTERFACE_CREATE_PARAMS \
    (reinterpret_cast<const struct AbckitArktsAnnotationInterfaceCreateParams *>(0xdead0039))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_STRUCT_ARKTS_ANNOTATION_CREATE_PARAMS \
    (reinterpret_cast<const struct AbckitArktsAnnotationCreateParams *>(0xdead0039))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_STRUCT_ARKTS_ANNOTATION_INTERFACE_FIELD_CREATE_PARAMS \
    (reinterpret_cast<const struct AbckitArktsAnnotationInterfaceFieldCreateParams *>(0xdead0039))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAULT_STRUCT_ARKTS_ANNOTATION_ELEMENT_CREATE_PARAMS \
    (reinterpret_cast<const struct AbckitArktsAnnotationElementCreateParams *>(0xdead0039))

#endif  // ABCKIT_MOCK_GLOBAL_VALUES
