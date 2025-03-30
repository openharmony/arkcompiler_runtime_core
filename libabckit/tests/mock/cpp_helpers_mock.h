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

#ifndef ABCKIT_CPP_HELPERS_MOCK
#define ABCKIT_CPP_HELPERS_MOCK

#include <string>
#include "../../include/cpp/abckit_cpp.h"
#include "check_mock.h"
#include "include/cpp/headers/value.h"
#include "include/cpp/headers/arkts/annotation.h"
#include "include/cpp/headers/arkts/annotation_interface.h"
#include "include/cpp/headers/arkts/annotation_interface_field.h"
#include "include/cpp/headers/arkts/class.h"
#include "include/cpp/headers/arkts/function.h"
#include "include/cpp/headers/core/annotation_element.h"
#include "include/cpp/headers/core/annotation_interface_field.h"
#include "mock/mock_values.h"

#include <gtest/gtest.h>

namespace abckit::mock::helpers {

// NOLINTBEGIN(performance-unnecessary-value-param)

inline abckit::core::Module GetMockCoreModule(const abckit::File &file)
{
    std::vector<abckit::core::Module> modules;

    file.EnumerateModules([&modules](abckit::core::Module md) -> bool {
        modules.push_back(md);
        return true;
    });

    EXPECT_TRUE(CheckMockedApi("FileEnumerateModules"));

    return modules.front();
}

inline abckit::arkts::Module GetMockArktsModule(const abckit::File &file)
{
    auto m = abckit::arkts::Module(GetMockCoreModule(file));
    EXPECT_TRUE(CheckMockedApi("CoreModuleToArktsModule"));
    return m;
}

inline abckit::core::Namespace GetMockCoreNamespace(const abckit::File &file)
{
    std::vector<abckit::core::Namespace> namespaces;
    abckit::core::Module mod = GetMockCoreModule(file);

    mod.EnumerateNamespaces([&namespaces](abckit::core::Namespace n) -> bool {
        namespaces.push_back(n);
        return true;
    });

    EXPECT_TRUE(CheckMockedApi("ModuleEnumerateNamespaces"));
    return namespaces.front();
}

inline abckit::arkts::Namespace GetMockArktsNamespace(const abckit::File &file)
{
    auto n = abckit::arkts::Namespace(GetMockCoreNamespace(file));
    EXPECT_TRUE(CheckMockedApi("CoreNamespaceToArktsNamespace"));
    return n;
}

inline abckit::core::Class GetMockCoreClass(const abckit::File &file)
{
    abckit::core::Module cmd = GetMockCoreModule(file);

    std::vector<abckit::core::Class> classes;

    cmd.EnumerateClasses([&classes](abckit::core::Class cls) -> bool {
        classes.push_back(cls);
        return true;
    });

    EXPECT_TRUE(CheckMockedApi("ModuleEnumerateClasses"));

    return classes.front();
}

inline abckit::arkts::Class GetMockArktsClass(const abckit::File &file)
{
    auto c = abckit::arkts::Class(GetMockCoreClass(file));
    EXPECT_TRUE(CheckMockedApi("CoreClassToArktsClass"));
    return c;
}

inline abckit::core::Function GetMockCoreFunction(const abckit::File &file)
{
    abckit::core::Class cls = GetMockCoreClass(file);
    std::vector<abckit::core::Function> funcs;

    cls.EnumerateMethods([&](abckit::core::Function md) -> bool {
        funcs.push_back(md);
        return true;
    });

    EXPECT_TRUE(CheckMockedApi("ClassEnumerateMethods"));

    return funcs.front();
}

inline abckit::arkts::Function GetMockArktsFunction(const abckit::File &file)
{
    auto f = abckit::arkts::Function(GetMockCoreFunction(file));
    EXPECT_TRUE(CheckMockedApi("CoreFunctionToArktsFunction"));
    return f;
}

inline abckit::core::Annotation GetMockCoreAnnotation(const abckit::File &file)
{
    abckit::core::Function func = GetMockCoreFunction(file);
    std::vector<abckit::core::Annotation> anns;

    func.EnumerateAnnotations([&](abckit::core::Annotation an) -> bool {
        anns.push_back(an);
        return true;
    });

    EXPECT_TRUE(CheckMockedApi("FunctionEnumerateAnnotations"));

    return anns.front();
}

inline abckit::arkts::Annotation GetMockArktsAnnotation(const abckit::File &file)
{
    auto a = abckit::arkts::Annotation(GetMockCoreAnnotation(file));
    EXPECT_TRUE(CheckMockedApi("CoreAnnotationToArktsAnnotation"));
    return a;
}

inline abckit::Graph GetMockGraph(const abckit::File &file)
{
    abckit::core::Function func = GetMockCoreFunction(file);
    abckit::Graph graph = func.CreateGraph();
    EXPECT_TRUE(CheckMockedApi("CreateGraphFromFunction"));
    // Utilization of NRVO, no dtor
    return graph;
}

inline abckit::BasicBlock GetMockBasicBlock(const abckit::File &file)
{
    abckit::BasicBlock bb = GetMockGraph(file).GetStartBb();
    EXPECT_TRUE(CheckMockedApi("DestroyGraph"));
    EXPECT_TRUE(CheckMockedApi("GgetStartBasicBlock"));
    // NOLINTNEXTLINE(clang-analyzer-core.StackAddressEscape)
    return bb;
}

inline abckit::Instruction GetMockInstruction(const abckit::File &file)
{
    abckit::BasicBlock bb = GetMockBasicBlock(file);
    abckit::Instruction instr = bb.GetFirstInst();
    EXPECT_TRUE(CheckMockedApi("BBGetFirstInst"));
    return instr;
}

inline abckit::core::ImportDescriptor GetMockCoreImportDescriptor(const abckit::File &file)
{
    abckit::core::ImportDescriptor id = GetMockGraph(file).DynIsa().GetImportDescriptor(GetMockInstruction(file));
    EXPECT_TRUE(CheckMockedApi("DestroyGraph"));
    EXPECT_TRUE(CheckMockedApi("IgetImportDescriptor"));
    return id;
}

inline abckit::arkts::ImportDescriptor GetMockArktsImportDescriptor(const abckit::File &file)
{
    auto id = abckit::arkts::ImportDescriptor(GetMockCoreImportDescriptor(file));
    EXPECT_TRUE(CheckMockedApi("CoreImportDescriptorToArktsImportDescriptor"));
    return id;
}

inline abckit::core::ExportDescriptor GetMockCoreExportDescriptor(const abckit::File &file)
{
    abckit::core::ExportDescriptor ed = GetMockGraph(file).DynIsa().GetExportDescriptor(GetMockInstruction(file));
    EXPECT_TRUE(CheckMockedApi("DestroyGraph"));
    EXPECT_TRUE(CheckMockedApi("IgetExportDescriptor"));
    return ed;
}

inline abckit::arkts::ExportDescriptor GetMockArktsExportDescriptor(const abckit::File &file)
{
    auto ed = abckit::arkts::ExportDescriptor(GetMockCoreExportDescriptor(file));
    EXPECT_TRUE(CheckMockedApi("CoreExportDescriptorToArktsExportDescriptor"));
    return ed;
}

inline abckit::LiteralArray GetMockLiteralArray(const abckit::File &file)
{
    abckit::LiteralArray litarr = GetMockInstruction(file).GetLiteralArray();
    EXPECT_TRUE(CheckMockedApi("IgetLiteralArray"));
    return litarr;
}

inline abckit::Type GetMockType(const abckit::File &file)
{
    abckit::Type t = file.CreateType(DEFAULT_TYPE_ID);
    EXPECT_TRUE(CheckMockedApi("CreateType"));
    return t;
}

inline abckit::Value GetMockValueString(const abckit::File &file)
{
    abckit::Value val = file.CreateValueString(DEFAULT_CONST_CHAR);
    EXPECT_TRUE(CheckMockedApi("CreateValueString"));
    return val;
}

inline abckit::Value GetMockValueDouble(const abckit::File &file)
{
    abckit::Value val = file.CreateValueDouble(DEFAULT_DOUBLE);
    EXPECT_TRUE(CheckMockedApi("CreateValueDouble"));
    return val;
}

inline abckit::Value GetMockValueU1(const abckit::File &file)
{
    abckit::Value val = file.CreateValueU1(DEFAULT_BOOL);
    EXPECT_TRUE(CheckMockedApi("CreateValueU1"));
    return val;
}

inline abckit::Value GetMockValueLiteralArray(const abckit::File &file)
{
    abckit::Value v = file.CreateValueU1(DEFAULT_BOOL);
    std::vector<abckit::Value> arr = {v};
    abckit::Value litArr = file.CreateLiteralArrayValue({arr}, 1);
    EXPECT_TRUE(CheckMockedApi("CreateLiteralArrayValue"));
    return litArr;
}

inline abckit::Literal GetMockLiteral(const abckit::File &file)
{
    abckit::Literal lit = file.CreateLiteralBool(DEFAULT_BOOL);
    EXPECT_TRUE(CheckMockedApi("CreateLiteralBool"));
    return lit;
}

inline abckit::Literal GetMockLiteralLiteralArray(const abckit::File &file)
{
    abckit::LiteralArray litArr = GetMockLiteralArray(file);
    abckit::Literal arr = file.CreateLiteralLiteralArray(litArr);
    EXPECT_TRUE(CheckMockedApi("CreateLiteralLiteralArray"));
    return arr;
}

inline abckit::Literal GetMockLiteralBool(const abckit::File &file)
{
    abckit::Literal lit = file.CreateLiteralBool(DEFAULT_BOOL);
    EXPECT_TRUE(CheckMockedApi("CreateLiteralBool"));
    return lit;
}

inline abckit::Literal GetMockLiteralU8(const abckit::File &file)
{
    abckit::Literal lit = file.CreateLiteralU8(DEFAULT_U8);
    EXPECT_TRUE(CheckMockedApi("CreateLiteralU8"));
    return lit;
}

inline abckit::Literal GetMockLiteralU16(const abckit::File &file)
{
    abckit::Literal lit = file.CreateLiteralU16(DEFAULT_U16);
    EXPECT_TRUE(CheckMockedApi("CreateLiteralU16"));
    return lit;
}

inline abckit::Literal GetMockLiteralU32(const abckit::File &file)
{
    abckit::Literal lit = file.CreateLiteralU32(DEFAULT_U32);
    EXPECT_TRUE(CheckMockedApi("CreateLiteralU32"));
    return lit;
}

inline abckit::Literal GetMockLiteralU64(const abckit::File &file)
{
    abckit::Literal lit = file.CreateLiteralU64(DEFAULT_U64);
    EXPECT_TRUE(CheckMockedApi("CreateLiteralU64"));
    return lit;
}

inline abckit::Literal GetMockLiteralFloat(const abckit::File &file)
{
    abckit::Literal lit = file.CreateLiteralFloat(DEFAULT_FLOAT);
    EXPECT_TRUE(CheckMockedApi("CreateLiteralFloat"));
    return lit;
}

inline abckit::Literal GetMockLiteralDouble(const abckit::File &file)
{
    abckit::Literal lit = file.CreateLiteralDouble(DEFAULT_DOUBLE);
    EXPECT_TRUE(CheckMockedApi("CreateLiteralDouble"));
    return lit;
}

inline abckit::Literal GetMockLiteralString(const abckit::File &file)
{
    abckit::Literal lit = file.CreateLiteralString(DEFAULT_CONST_CHAR);
    EXPECT_TRUE(CheckMockedApi("CreateLiteralString"));
    return lit;
}

inline abckit::Literal GetMockLiteralMethodAffiliate(const abckit::File &file)
{
    abckit::Literal lit = file.CreateLiteralMethodAffiliate(DEFAULT_U16);
    EXPECT_TRUE(CheckMockedApi("CreateLiteralMethodAffiliate"));
    return lit;
}

inline abckit::core::AnnotationElement GetMockCoreAnnotationElement(const abckit::File &file)
{
    abckit::core::Annotation ann = GetMockCoreAnnotation(file);
    std::vector<abckit::core::AnnotationElement> annElems;

    ann.EnumerateElements([&](abckit::core::AnnotationElement elem) -> bool {
        annElems.push_back(elem);
        return true;
    });

    EXPECT_TRUE(CheckMockedApi("AnnotationEnumerateElements"));

    return annElems.front();
}

inline abckit::arkts::AnnotationElement GetMockArktsAnnotationElement(const abckit::File &file)
{
    auto ae = abckit::arkts::AnnotationElement(GetMockCoreAnnotationElement(file));
    EXPECT_TRUE(CheckMockedApi("CoreAnnotationElementToArktsAnnotationElement"));
    return ae;
}

inline abckit::core::AnnotationInterface GetMockCoreAnnotationInterface(const abckit::File &file)
{
    abckit::core::Annotation ann = GetMockCoreAnnotation(file);
    abckit::core::AnnotationInterface ai = ann.GetInterface();
    EXPECT_TRUE(CheckMockedApi("AnnotationGetInterface"));

    return ai;
}

inline abckit::arkts::AnnotationInterface GetMockArktsAnnotationInterface(const abckit::File &file)
{
    auto ai = abckit::arkts::AnnotationInterface(GetMockCoreAnnotationInterface(file));
    EXPECT_TRUE(CheckMockedApi("CoreAnnotationInterfaceToArktsAnnotationInterface"));
    return ai;
}

inline abckit::core::AnnotationInterfaceField GetMockCoreAnnotationInterfaceField(const abckit::File &file)
{
    abckit::core::AnnotationInterface ai = GetMockCoreAnnotationInterface(file);
    std::vector<abckit::core::AnnotationInterfaceField> fields;
    ai.EnumerateFields([&](abckit::core::AnnotationInterfaceField field) -> bool {
        fields.push_back(field);
        return true;
    });
    EXPECT_TRUE(CheckMockedApi("AnnotationInterfaceEnumerateFields"));

    return fields.front();
}

inline abckit::arkts::AnnotationInterfaceField GetMockArktsAnnotationInterfaceField(const abckit::File &file)
{
    auto aif = abckit::arkts::AnnotationInterfaceField(GetMockCoreAnnotationInterfaceField(file));
    EXPECT_TRUE(CheckMockedApi("CoreAnnotationInterfaceFieldToArktsAnnotationInterfaceField"));
    return aif;
}

// NOLINTEND(performance-unnecessary-value-param)

}  // namespace abckit::mock::helpers

#endif  // ABCKIT_CPP_HELPERS_MOCK
