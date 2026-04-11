..
    Copyright (c) 2021-2026 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


.. _RT Runtime Name:

运行时名称
********************************************************************************

|LANG| 运行时（例如标准库反射和类加载 API）以及构建期工具（构建系统、编译器、字节码操作工具）都使用 *运行时名称* 来处理模块、类和其他实体。*运行时名称* 的语法如下：

.. code-block:: abnf

    RuntimeName:
        PrimitiveType
        | ArrayType
        | RefType
        | UnionType
        ;
    PrimitiveType:
        "void"
        | "u1"
        | "i8"
        | "u8"
        | "i16"
        | "u16"
        | "i32"
        | "u32"
        | "f32"
        | "f64"
        | "i64"
        | "u64"
        | "any"
        ;
    ArrayType:
        RuntimeName '[]'
        ;
    RefType:
        RefTypeName '[]'
        ;
    UnionType:
        '{U' RuntimeName ',' RuntimeName (',' RuntimeName)* '}'
        ;

``RefTypeName`` 是该类型的 :ref:`全限定名 <RT Fully Qualified Name>`。

**约束**：``UnionType`` 必须被规范化。规范化要求把所有 *组成类型* 的 ``RuntimeName`` 按字母顺序排序。
