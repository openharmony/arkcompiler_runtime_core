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

#ifndef CPP_ABCKIT_DECLARATIONS_H
#define CPP_ABCKIT_DECLARATIONS_H

namespace abckit {

// ISA API's
class DynamicIsa;
class StaticIsa;

// IR core interfaces
class Graph;
class BasicBlock;
class Instruction;

// Language-independent entities interfaces
class File;
class Type;
class Value;
class Literal;
class LiteralArray;

// Core declarations
namespace core {
class Module;     // Check
class Namespace;  // Check
class Class;      // Check
class Function;   // Check
class Field;
class Annotation;                // Check
class AnnotationElement;         // Check
class AnnotationInterface;       // Check
class AnnotationInterfaceField;  // Check
class ImportDescriptor;
class ExportDescriptor;
}  // namespace core

// ArkTS extension declarations
namespace arkts {
class Module;
class Namespace;
class Class;
class Function;
class Field;
class Annotation;
class AnnotationElement;
class AnnotationInterface;
class AnnotationInterfaceField;
class ImportDescriptor;
class ExportDescriptor;
}  // namespace arkts

}  // namespace abckit

#endif  // CPP_ABCKIT_DECLARATIONS_H
