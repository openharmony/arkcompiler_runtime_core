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


.. _Annotations:

注解
###########

.. meta:
    frontend_status: Done

*注解* 是一种特殊语言元素，它通过添加元数据来改变其所应用声明的语义。

下例展示了注解的声明与使用：

.. code-block:: typescript
   :linenos:

    // Annotation declaration:
    @interface ClassAuthor {
        authorName: string
    }

    // Annotation use:
    @ClassAuthor({authorName: "Bob"})
    class MyClass {/*body*/}

上述示例中的注解 *ClassAuthor* 为类声明添加了元数据。

注解必须紧挨着其所应用的声明之前放置。注解也可以像上例一样包含实参。

要使用注解，注解名必须带有字符 ``@`` 前缀（例如 ``@MyAnno``）。字符 ``@`` 与名称之间不允许出现空白符或换行符：

.. index::
   annotation
   semantics
   language element
   metadata
   declaration
   class declaration
   prefix
   space
   white space
   line separator
   argument
   name

.. code-block::
   :linenos:

    ClassAuthor({authorName: "Bob"}) // Compile-time error, no '@'
    @ ClassAuthor({authorName: "Bob"}) // Compile-time error, space is forbidden

如果注解名称在使用位置不可访问（见 :ref:`Accessible`），则会发生 :index:`compile-time error`。
注解声明可以被导出并在其他模块中使用。

多个注解可以应用于同一个声明：

.. code-block:: typescript
   :linenos:

    @MyAnno()
    @ClassAuthor({authorName: "John Smith"})
    class MyClass {/*body*/}

.. index::
   annotation
   access
   accessibility
   annotation declaration
   declaration

|

.. _Declaring Annotations:

声明注解
********************************************************************************

.. meta:
    frontend_status: Done

声明 *注解* 与声明接口类似，只是关键字 ``interface`` 需要带前缀字符 ``'@'``。

*注解声明* 的语法如下：

.. code-block:: abnf

    annotationDeclaration:
        '@interface' identifier '{' annotationField* '}'
        ;
    annotationField:
        identifier ':' type constInitializer?
        ;
    constInitializer:
        '=' constantExpression
        ;

与其他已声明实体一样，注解也可以使用关键字 ``export`` 导出。

注解字段中的 *type* 受到限制（见 :ref:`Types of Annotation Fields`）。

*注解字段* 的默认值可以通过把 *initializer* 写成 *常量表达式* 来指定。
如果该表达式的值不能在编译期求出，则会发生 :index:`compile-time error`。

.. index::
   annotation
   declaration
   interface
   interface keyword
   prefix
   export keyword
   syntax
   annotation declaration
   annotation field
   declared entity
   constant expression
   compile time
   initializer
   expression
   value
   type

*注解* 必须定义在顶层。否则会发生 :index:`compile-time error`。

*注解* 不能被扩展，因为不支持继承。

*注解* 的名称不能与其他实体的名称相同：

.. code-block:: typescript
   :linenos:

    @interface Position {/*properties*/}

    class Position {/*body*/} // Compile-time error, duplicate identifier

注解声明不会定义任何类型。不能将类型别名应用到注解，也不能将注解用作接口：

.. code-block:: typescript
   :linenos:

    @interface Position {}
    type Pos = Position // Compile-time error

    class A implements Position {} // Compile-time error

.. index::
   annotation
   type alias
   inheritance
   annotation declaration
   interface
   entity
   type

|

.. _Types of Annotation Fields:

注解字段的类型
================================================================================

.. meta:
    frontend_status: Done

*注解字段类型* 只能从下列类型中选择：

- :ref:`Numeric Types`；
- ``boolean`` 类型（见 :ref:`Type boolean`）；
- :ref:`Type string`；
- 枚举类型（见 :ref:`Enumerations`）；
- 上述类型的数组（例如 ``string[]``），包括数组的数组（例如 ``string[][]``）。

如果使用了其他任何类型作为 *注解字段* 的类型，则会发生 :index:`compile-time error`。

.. index::
   annotation field
   type for annotation field
   numeric type
   boolean type
   string type
   enumeration type
   array

|

.. _Using Annotations:

使用注解
********************************************************************************

.. meta:
    frontend_status: Done

将注解应用到声明并定义注解字段值时，使用如下语法：

.. code-block:: abnf

    annotationUsage:
        AnnotationUsageNoParentheses |
        annotationUsageWithParentheses
        ;

    annotationUsageNoParentheses:
        '@' qualifiedName
        ;

    annotationUsageWithParentheses:
        '@' qualifiedName annotationValues
        ;
    annotationValues:
        '(' (objectLiteral | constantExpression)? ')'
        ;

下例展示了注解声明：

.. code-block:: typescript
   :linenos:

    @interface ClassPreamble {
        authorName: string
        revision: number = 1
    }
    @interface MyAnno{}

一般情况下，注解字段值通过 *对象字面量* 设置。在一种特殊情况下，可通过表达式设置注解字段值
（见 :ref:`Using Single Field Annotations`）。

注解字段的值必须是：

- 如果字段不是数组类型，则必须是常量表达式；或者

- 必须是 :ref:`Array Literal`，且其中元素要么是常量表达式，
  要么在数组的数组情形下，是包含常量表达式的内层数组字面量。

否则会发生 :index:`compile-time error`。

.. index::
   annotation
   annotation declaration
   syntax
   declaration
   annotation field
   object literal
   value
   expression

下例展示了注解的使用。例中的注解应用于类声明：

.. code-block:: typescript
   :linenos:

    @ClassPreamble({authorName: "John", revision: 2})
    class C1 {/*body*/}

    @ClassPreamble({authorName: "Bob"}) // default value for revision = 1
    class C2 {/*body*/}

    @MyAnno()
    class C3 {/*body*/}

注解可以应用于以下位置：

- :ref:`Top-Level Declarations`；
- 类成员（见 :ref:`Class Members`），但不包括被重写的字段（见下例）；
- 接口成员（见 :ref:`Interface Members`）；
- 类型使用（见 :ref:`Using Types`）；
- 参数（见 :ref:`Parameter List` 和 :ref:`Optional Parameters`）；
- lambda 表达式（见 :ref:`Lambda Expressions` 和 :ref:`Lambda Expressions with Receiver`）；
- :ref:`Constant Or Variable Declarations`。

.. index::
   annotation
   declaration
   class declaration
   top-level declaration
   class
   type
   interface
   method
   parameter
   optional parameter
   lambda expression
   lambda expression with receiver
   function
   local declaration

否则会发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    function foo () @MyAnno() {} // wrong target for annotation


如果把注解应用到被重写字段（见 :ref:`Override Fields`），则会发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    class C {
        field: int = 1
    }
    class D extends C {
        @MyAnno() // Compile-time error
        field: int = 2
    }

不支持可重复注解，也就是说，同一个注解在同一个实体上最多只能应用一次：

.. code-block:: typescript
   :linenos:

    @ClassPreamble({authorName: "John"})
    @ClassPreamble({authorName: "Bob"}) // Compile-time error
    class C {/*body*/}

使用注解时，字段值的顺序无关紧要：

.. code-block:: typescript
   :linenos:

    @ClassPreamble({authorName: "John", revision: 2})
    // the same as:
    @ClassPreamble({revision: 2, authorName: "John"})

使用注解时，所有没有默认值的字段都必须列出。否则会发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    @ClassPreamble() // Compile-time error, authorName is not defined
    class C1 {/*body*/}

.. index::
   annotation
   repeatable annotation
   entity
   array literal
   array type
   value
   field

如果为注解定义了数组类型字段，则其值应使用数组字面量语法设置：

.. code-block:: typescript
   :linenos:

    @interface ClassPreamble {
        authorName: string
        revision: number = 1
        reviewers: string[]
    }

    @ClassPreamble(
        {authorName: "Alice",
        reviewers: ["Bob", "Clara"]}
    )
    class C3 {/*body*/}

如果不需要设置注解属性，则可以省略注解名后的括号：

.. code-block:: typescript
   :linenos:

    @MyAnno
    class C4 {/*body*/}

.. index::
   field
   array type
   annotation
   syntax
   array literal
   parentheses
   annotation name

|

.. _Using Single Field Annotations:

使用单字段注解
================================================================================

.. meta:
    frontend_status: Done

如果注解声明只定义了一个字段，那么可以使用简写记法，用单个表达式代替对象字面量：

.. code-block:: typescript
   :linenos:

    @interface deprecated{
        fromVersion: string
    }

    @deprecated("5.18")
    function foo() {}

    @deprecated({fromVersion: "5.18"})
    function goo() {}

简写记法与对象字面量记法的行为完全相同。

.. index::
   field annotation
   annotation declaration
   field
   notation
   expression
   object literal

|

.. _Exporting and Importing Annotations:

导出与导入注解
********************************************************************************

.. meta:
    frontend_status: Done

注解可以被导出和导入。不过，只支持部分形式的导出与导入指令。

要导出的注解声明必须按如下方式标记关键字 ``export``：

.. code-block:: typescript
   :linenos:

    // a.ets
    export @interface MyAnno {}

如果注解作为已导入模块的一部分被导入，则通过其限定名访问该注解：

.. code-block:: typescript
   :linenos:

    // b.ets
    import * as ns from "./a"

    @ns.MyAnno
    class C {/*body*/}

也允许使用非限定导入：

.. index::
   export
   import
   annotation
   annotation declaration
   export keyword
   import directive
   export directive
   imported module
   qualified name
   access
   unqualified import

.. code-block:: typescript
   :linenos:

    // b.ets
    import { MyAnno } from "./a"

    @MyAnno
    class C {/*body*/}

注解声明不会定义类型。使用 ``export type`` 或 ``import type`` 记法导出或导入注解，
会导致 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    import type { MyAnno } from "./a" // Compile-time error


如果在以下情形中使用注解，也会发生 :index:`compile-time error`：

- Export default，
- Import default，
- Rename in export，以及
- Rename in import。

.. index::
   annotation
   export type
   import type
   import annotation
   export annotation
   annotation
   notation
   type
   notation
   import annotation
   export default
   import default
   renaming

.. code-block:: typescript
   :linenos:

    import {MyAnno as Anno} from "./a" // Compile-time error

|

.. _Ambient Annotations:

环境注解
********************************************************************************

.. meta:
    frontend_status: Done

*环境注解* 的语法如下：

.. code-block:: abnf

    ambientAnnotationDeclaration:
        'declare' annotationDeclaration
        ;

这种声明不会引入新的注解，而是提供使用该注解所需的类型信息。注解本身必须在其他地方定义。
如果程序中使用的环境注解没有对应的实际注解，则会发生 :index:`runtime error`。

环境注解与实现它的注解必须完全一致，包括字段初始化：

.. index::
   syntax
   ambient annotation
   declaration
   annotation
   type
   runtime error
   field initialization
   initialization

.. code-block:: typescript
   :linenos:

    // a.d.ets
    export declare @interface NameAnno{name: string = ""}

    // a.ets
    export @interface NameAnno{name: string = ""} // OK

下面示例中的代码不正确，因为环境声明与注解声明并不一致：

.. code-block:: typescript
   :linenos:

    // a.d.ets
    export declare @interface VersionAnno{version: number} // initialization is missing

    // a.ets
    export @interface VersionAnno{version: number = 1}

环境声明可以像普通注解一样被导入并使用：

.. code-block:: typescript
   :linenos:

    // a.d.ets
    export declare @interface MyAnno {}

    // b.ets
    import { MyAnno } from "./a"

    @MyAnno
    class C {/*body*/}

如果在 ``*.d.ets`` 文件中的环境声明上应用了注解（见下例），那么必须手动把该注解应用到实现声明上，
因为该注解不会自动应用到实现该环境声明的声明上：

.. code-block:: typescript
   :linenos:

    // a.d.ets
    export declare @interface MyAnno {}

    @MyAnno
    declare class C {}

.. index::
   annotation declaration
   initialization
   import
   annotation
   ambient declaration
   declaration
   implementation

|

.. _Standard Annotations:

标准注解
********************************************************************************

.. meta:
    frontend_status: Done

*标准注解* 是指在 :ref:`Standard Library` 中定义，或在编译器中隐式定义（*built-in annotation*）的注解。
*标准注解* 通常为编译器所知。它会修改其所应用声明的语义。

用于注解另一个注解声明的注解称为 *元注解*。

.. index::
   standard annotation
   annotation
   standard annotation
   compiler
   built-in annotation
   call
   semantics
   declaration
   meta-annotation

|

.. _Retention Annotation:

Retention 注解
================================================================================

.. meta:
    frontend_status: Done

``@Retention`` 是一种标准 *元注解*，用于注解另一个注解的声明。
如果在其他位置使用它，则会发生 :index:`compile-time error`。

该注解只有一个名为 ``policy`` 的字段，类型为 ``string``。其典型用法如下：

.. code-block:: typescript
   :linenos:

    @Retention({policy: "RUNTIME"})
    @interface MyAnno {} // this annotation uses "RUNTIME" policy

    @MyAnno //
    class C {}

.. index::
   meta-annotation
   retention annotation
   standard annotation
   annotation
   declaration
   declaration annotation
   field
   string type

该字段的值决定了注解会在什么阶段被使用，并在使用后丢弃。
保留策略分为三种：

- ``"SOURCE"``

  使用 ``"SOURCE"`` 策略的注解会在编译期处理，并在编译后被丢弃；

- ``"BYTECODE"``

  使用 ``"BYTECODE"`` 策略的注解所指定的元数据会保存到字节码文件中，但会在运行时被丢弃。

- ``"RUNTIME"``

  使用 ``"RUNTIME"`` 策略的注解所指定的元数据会保存到字节码文件中、在运行时保留，并且可在运行时访问。

默认保留策略是 ``"BYTECODE"``。

如果将其他任何字符串字面量作为 ``policy`` 字段的值，则会发生 :index:`compile-time error`。

由于 ``@Retention`` 只有一个字段，因此可以像下面这样使用简写记法
（见 :ref:`Using Single Field Annotations`）：

.. code-block:: typescript
   :linenos:

    @Retention("SOURCE")
    @interface Author {name: string} // this annotation uses "SOURCE" policy

.. index::
   source
   runtime
   value
   field
   compile time
   bytecode
   metadata
   annotation
   policy
   bytecode file
   string literal
   notation

如果某个注解采用 ``"BYTECODE"`` 或 ``"RUNTIME"`` 策略，而不是 ``"SOURCE"`` 策略，
则其字段值可以在执行期间读取；详见 :ref:`Runtime Access to Annotations`。

|

.. _Target Annotation:

Target 注解
================================================================================

.. meta:
    frontend_status: None


``@Target`` 是一种标准 *元注解*，用于注解另一个注解的声明。
如果在其他位置使用 ``@Target``，则会发生 :index:`compile-time error`。

``@Target`` 指定所声明注解可被使用的一组源代码上下文。
这些上下文通过 :ref:`Standard Library` 中定义的 ``AnnotationTargets`` 值集合来指定。

注解 ``@Target`` 有一个名为 ``targets`` 的字段，其类型为 ``AnnotationTargets[]``。
其典型用法如下：

.. index::
   target annotation
   annotation
   meta-annotation
   declaration
   source code
   context
   value

.. code-block:: typescript
   :linenos:

    // short form:
    @Target(["FUNCTION", "CLASS_METHOD"])
    @interface SpecialCall {/*some fields*/}

    // long form:
    @Target({targets: ["PARAMETER"]})
    @interface SpecialParameter {/*some fields*/}

如果注解 ``X`` 的声明上存在该注解，则编译器会检查 ``X`` 是否仅在指定上下文中使用。
否则会发生 :index:`compile-time error`。

如果注解 ``X`` 的声明上没有该注解，则 ``X`` 的使用不受限制。

``AnnotationTargets`` 类型包含以下目标所对应的字符串字面量：

.. index::
   annotation
   declaration
   compiler
   compiler check
   context
   restriction
   enumeration
   constant

- :ref:`Top-Level Declarations` 目标：

  - ``"CLASS"``；
  - ``"ENUMERATION"``；
  - ``"FUNCTION"``；
  - ``"FUNCTION_WITH_RECEIVER"``；
  - ``"INTERFACE"``；
  - ``"NAMESPACE"``；
  - ``"TYPE_ALIAS"``；
  - ``"VARIABLE"``；

- :ref:`Class Members` 目标：

  - ``"CLASS_FIELD"``；
  - ``"CLASS_METHOD"``；
  - ``"CLASS_GETTER"``；
  - ``"CLASS_SETTER"``；

- :ref:`Interface Members` 目标：

  - ``"INTERFACE_METHOD"``；
  - ``"INTERFACE_GETTER"``；
  - ``"INTERFACE_SETTER"``；

- 其他目标：

  - ``"LAMBDA"``，用于 :ref:`Lambda Expressions` 和 :ref:`Lambda Expressions with Receiver`；
  - ``"PARAMETER"``，用于函数、方法和 lambda 参数；
  - "STRUCT"（见 :ref:`Keyword struct and ArkUI`）；
  - "TYPE"（见 :ref:`Using Types`）。

.. index::
   class
   enumeration
   function
   interface
   namespace
   type alias
   variable
   class member
   function with receiver
   class field
   class method
   class getter
   class setter
   interface method
   interface getter
   interface setter
   target
   lambda
   parameter
   struct
   type
   annotation

如果在 ``@Target`` 注解中多次使用同一个值，则会发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    @Target(["CLASS", "INTERFACE", "CLASS"]) // Compile-time error
    @interface Anno {}

|

.. _Runtime Access to Annotations:

运行时访问注解
********************************************************************************

.. meta:
    frontend_status: None

对于采用 ``"BYTECODE"`` 或 ``"RUNTIME"`` ``保留策略``（见 :ref:`Retention Annotation`）的注解，
编译器会隐式声明一个与该注解同名的抽象类。该类的所有字段都声明为 ``readonly``。
如果字段是数组类型，那么该数组类型也为 ``readonly``。

.. index::
   runtime
   access
   annotation
   retention policy
   retention annotation
   bytecode
   readonly field
   array type

以下注解：

.. code-block:: typescript
   :linenos:

    @Retention("RUNTIME")
    @interface MyAnno {
        name: string
        attrs: number[]
    }

-- 会隐式声明如下抽象类：

.. code-block:: typescript
   :linenos:

    abstract class MyAnno {
        readonly name: string
        readonly attrs: readonly number[]
    }

该类的用法见下例：

.. code-block:: typescript
   :linenos:

    @MyAnno({name: "someName", attr: [1, 2]})
    class A {}

    let my: MyAnno = // call of reflection library to get instance of annotation for type A
    console.log(my.name) // output: someName

.. index::
   annotation
   abstract class
   declaration
   readonly name

.. note::
   - 对于采用 ``"SOURCE"`` 保留策略的注解，不会声明抽象类。

   - 获取隐式声明抽象类实例的唯一方式是调用反射库。
     如果出现以下情形，则会发生 :index:`compile-time error`：

     - 对该类使用 ``new`` 表达式（见 :ref:`New Expressions`）；

     - 在 :ref:`Class Extension Clause` 中使用该类。

.. raw:: pdf

   PageBreak
