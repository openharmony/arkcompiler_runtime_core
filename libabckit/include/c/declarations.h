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

#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct AbckitFile;
struct AbckitString;
struct AbckitType;
struct AbckitValue;
struct AbckitLiteral;
struct AbckitLiteralArray;

struct AbckitCoreModule;
struct AbckitCoreNamespace;
struct AbckitCoreClass;
struct AbckitCoreFunction;
struct AbckitCoreField;
struct AbckitCoreAnnotation;
struct AbckitCoreAnnotationElement;
struct AbckitCoreAnnotationInterface;
struct AbckitCoreAnnotationInterfaceField;
struct AbckitCoreImportDescriptor;
struct AbckitCoreExportDescriptor;

struct AbckitArktsModule;
struct AbckitArktsNamespace;
struct AbckitArktsClass;
struct AbckitArktsFunction;
struct AbckitArktsField;
struct AbckitArktsAnnotation;
struct AbckitArktsAnnotationElement;
struct AbckitArktsAnnotationInterface;
struct AbckitArktsAnnotationInterfaceField;
struct AbckitArktsImportDescriptor;
struct AbckitArktsExportDescriptor;

struct AbckitJsModule;
struct AbckitJsClass;
struct AbckitJsFunction;
struct AbckitJsImportDescriptor;
struct AbckitJsExportDescriptor;

struct AbckitGraph;
struct AbckitBasicBlock;
struct AbckitInst;

#ifndef __cplusplus
typedef uint8_t *AbckitFileVersion;

typedef struct AbckitFile AbckitFile;
typedef struct AbckitString AbckitString;
typedef struct AbckitType AbckitType;
typedef struct AbckitValue AbckitValue;
typedef struct AbckitLiteral AbckitLiteral;
typedef struct AbckitLiteralArray AbckitLiteralArray;

typedef struct AbckitCoreModule AbckitCoreModule;
typedef struct AbckitCoreNamespace AbckitCoreNamespace;
typedef struct AbckitCoreClass AbckitCoreClass;
typedef struct AbckitCoreFunction AbckitCoreFunction;
typedef struct AbckitCoreField AbckitCoreField;
typedef struct AbckitCoreAnnotation AbckitCoreAnnotation;
typedef struct AbckitCoreAnnotationElement AbckitCoreAnnotationElement;
typedef struct AbckitCoreAnnotationInterface AbckitCoreAnnotationInterface;
typedef struct AbckitCoreAnnotationInterfaceField AbckitCoreAnnotationInterfaceField;
typedef struct AbckitCoreImportDescriptor AbckitCoreImportDescriptor;
typedef struct AbckitCoreExportDescriptor AbckitCoreExportDescriptor;

typedef struct AbckitArktsModule AbckitArktsModule;
typedef struct AbckitArktsNamespace AbckitArktsNamespace;
typedef struct AbckitArktsClass AbckitArktsClass;
typedef struct AbckitArktsFunction AbckitArktsFunction;
typedef struct AbckitArktsField AbckitArktsField;
typedef struct AbckitArktsAnnotation AbckitArktsAnnotation;
typedef struct AbckitArktsAnnotationElement AbckitArktsAnnotationElement;
typedef struct AbckitArktsAnnotationInterface AbckitArktsAnnotationInterface;
typedef struct AbckitArktsAnnotationInterfaceField AbckitArktsAnnotationInterfaceField;
typedef struct AbckitArktsImportDescriptor AbckitArktsImportDescriptor;
typedef struct AbckitArktsExportDescriptor AbckitArktsExportDescriptor;

typedef struct AbckitJsModule AbckitJsModule;
typedef struct AbckitJsClass AbckitJsClass;
typedef struct AbckitJsFunction AbckitJsFunction;
typedef struct AbckitJsImportDescriptor AbckitJsImportDescriptor;
typedef struct AbckitJsExportDescriptor AbckitJsExportDescriptor;

typedef struct AbckitGraph AbckitGraph;
typedef struct AbckitBasicBlock AbckitBasicBlock;
typedef struct AbckitInst AbckitInst;
#else
using AbckitFileVersion = uint8_t *;
#endif

#ifdef __cplusplus
}
#endif

#endif
