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


.. _Ambient Declarations:

环境声明
####################

.. meta:
    frontend_status: Done

*环境声明* 指定一个在别处声明、但可在当前上下文中使用的实体。

环境声明：

- 为在别处声明的实体提供类型信息；
- 不像常规声明那样引入新实体；
- 不能包含可执行代码，因此：

  - 环境变量、常量和枚举都没有初始化器；
  - 环境函数、方法和构造器都没有函数体。

.. index::
   ambient declaration
   declaration
   module
   entity
   executable code
   initializer
   initialization
   ambient function
   ambient method
   ambient constructor
   function
   method
   constructor
   function body
   method body
   constructor body


*环境声明* 的语法如下：

.. code-block:: abnf

    ambientDeclaration:
        'declare'
        ( ambientConstantOrVariableDeclaration
        | ambientFunctionDeclaration
        | explicitFunctionOverload
        | ambientClassDeclaration
        | ambientInterfaceDeclaration
        | ambientEnumDeclaration
        | ambientNamespaceDeclaration
        | ambientAnnotationDeclaration
        | ambientAccessorDeclaration
        | typeAlias
        )
        ;

如果在已经属于环境上下文的位置使用修饰符 ``declare``，则会发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    declare namespace A{
        declare function foo(): void // Compile-time error
    }

.. index::
   syntax
   ambient declaration
   enumeration type declaration
   context
   modifier declare
   declare
   declared type
   prefix
   const keyword
   compatibility
   ambient

单独的 *环境声明* 并不能保证在别处为同名实体声明的非环境声明与之完全一致。
从其本质看，任意 *环境声明* 只是一项要求，即由 :ref:`Build System` 提供合适的非环境声明。

|

.. _Ambient Constant or Variable Declarations:

环境常量或变量声明
*********************************************************************************

.. meta:
    frontend_status: Done

*环境* 常量或变量声明的语法如下：

.. code-block:: abnf

    ambientConstantOrVariableDeclaration:
        'const'|'let' ambientConstantOrVariableList ';'
        ;

    ambientConstantOrVariableList:
        ambientConstantOrVariable (',' ambientConstantOrVariable)*
        ;

    ambientConstantOrVariable:
        identifier ':' type
        ;

.. index::
   ambient constant
   ambient variable
   constant declaration
   variable declaration
   declaration

*环境常量* 与 *环境变量声明* 必须具有显式类型注解，并且不得带有初始化器。否则会发生
:index:`compile-time error`：


.. code-block:: typescript
   :linenos:

    declare let v1: number // OK
    declare let v2 = 1     // Compile-time error, ambient variable must have no initializer

    declare const c1: number // OK
    declare const c2 = 1     // Compile-time error, ambient constant must have no initializer

|

.. _Ambient Function Declarations:

环境函数声明
********************************************************************************

.. meta:
    frontend_status: Done

*环境函数声明* 的语法如下：

.. code-block:: abnf

    ambientFunctionDeclaration:
        'function' identifier typeParameters? signature
        ;

如果环境函数声明具有以下任一情况，则会发生 :index:`compile-time error`：

- 未指定显式返回类型；
- 某个参数带有默认值；
- 带有函数体；或
- 指定了修饰符 ``async``。

下例如此说明：

.. code-block:: typescript
   :linenos:

    declare function ok1(x: number): void // OK
    declare function bad1(x: number) // Compile-time error, no return specified

    declare function ok2(x?: string): void // OK, optional parameters can be used
    declare function bad2(y: number = 1): void // Compile-time error, parameter
                                               // has default value

    declare function bad3(): void {} // Compile-time error, function body provided

    declare async function bad4(): void // Compile-time error, async modifier is used


.. index::
   syntax
   ambient function declaration
   return type
   function body
   parameter
   optional parameter
   default value
   modifier async
   async modifier
   function body
   ambient context

|

.. _Ambient Overload Function Declarations:

环境重载函数声明
********************************************************************************

.. meta:
    frontend_status: None

*环境重载函数声明* 的语法与 :ref:`Explicit Function Overload` 完全相同。此类声明的语义也由相同规则定义。


.. code-block:: typescript
   :linenos:

   // Top-level functions are overloaded
   declare function foo1(p: string): void
   declare function foo2(p: number): void
   declare overload foo {foo1, foo2}

   // Namespace functions are overloaded
   declare namespace N {
      function foo1(p: string): void
      function foo2(p: number): void
      overload foo {foo1, foo2}
   }

   // All calls are valid
   foo("a string")
   foo(5)
   N.foo("a string")
   N.foo(5)

.. index::
   ambient overload function declaration
   ambient overload function
   explicit function overload
   semantics
   syntax

|

.. _Ambient Class Declarations:

环境类声明
********************************************************************************

.. meta:
    frontend_status: Done

*环境类声明* 的语法如下：

.. code-block:: abnf

    ambientClassDeclaration:
        'class'|'struct' identifier typeParameters?
        classExtendsClause? implementsClause?
        '{' ambientClassMember* '}'
        ;

    ambientClassMember:
        ambientAccessModifier?
        ( ambientFieldDeclaration
        | ambientConstructorDeclaration
        | ambientMethodDeclaration
        | explicitClassMethodOverload
        | ambientClassAccessorDeclaration
        | ambientIndexerDeclaration
        | ambientCallSignatureDeclaration
        | ambientIterableDeclaration
        )
        ;

    ambientAccessModifier:
        'public' | 'protected'
        ;

环境字段声明不带初始化器。

.. index::
   ambient field declaration
   ambient class declaration
   initializer
   syntax

*环境字段声明* 的语法如下：

.. code-block:: abnf

    ambientFieldDeclaration:
        ambientFieldModifier* identifier ':' type
        ;

    ambientFieldModifier:
        'static' | 'readonly'
        ;

环境构造器、方法和 accessor 声明都不带函数体。

它们的语法如下：


.. index::
   ambient field declaration
   ambient class declaration
   ambient constructor declaration
   ambient method declaration
   ambient accessor declaration
   initializer declaration
   syntax

.. code-block:: abnf

    ambientConstructorDeclaration:
        'constructor' parameters
        ;

    ambientMethodDeclaration:
        ambientMethodModifier* identifier signature
        ;

    ambientMethodModifier:
        'static'
        ;

    ambientClassAccessorDeclaration:
        ambientMethodModifier*
        ( 'get' identifier '(' ')' returnType
        | 'set' identifier '(' requiredParameter ')'
        )
        ;

环境方法可以像非环境方法一样重载，并采用相同的语法与语义（见 :ref:`Explicit Class Method Overload`）。

.. code-block:: typescript
   :linenos:


   // Class methods are overloaded
   declare class A {
      foo1(p: string): void
      foo2(p: number): void
      overload foo {foo1, foo2}
   }

   // All methods calls are valid
   function demo (a: A) {
      a.foo("a string")
      a.foo(5)
   }

.. index::
   ambient method
   overload
   non-ambient method
   syntax
   semantics
   method call
   class method

|

.. _Ambient Indexer:

环境索引器
================================================================================

.. meta:
    frontend_status: Done

*环境索引器声明* 指定环境上下文中类实例的索引方式。该特性为兼容 |TS| 而提供：

*环境索引器声明* 的语法如下：

.. code-block:: abnf

    ambientIndexerDeclaration:
        'readonly'? '[' identifier ':' type ']' returnType
        ;
.. index::
   ambient indexer
   ambient indexer declaration
   indexing
   class instance
   ambient context
   syntax
   compatibility

*环境索引器声明* 的使用如下例所示：

.. code-block:: typescript
   :linenos:

    declare class C {
        [index: number]: number
    }
    declare class D {
        [index: int]: C
    }
    declare class E {
        [index: string]: string
    }

适用以下限制：

- 环境类声明中只允许一个 *环境索引器声明*。

- *环境索引器声明* 仅在环境上下文中受支持。
  如果写在 |LANG| 中，则环境类实现必须符合 :ref:`Indexable Types`。

.. index::
   ambient indexer declaration
   restriction
   ambient class declaration
   ambient context
   ambient class
   implementation
   indexable type

|

.. _Ambient Call Signature:

环境调用签名
================================================================================

.. meta:
    frontend_status: Done

*环境调用签名* 声明用于在环境上下文中指定 *可调用类型*。该特性为兼容 |TS| 而提供：

*环境调用签名声明* 的语法如下：

.. code-block:: abnf

    ambientCallSignatureDeclaration:
        signature
        ;

.. code-block:: typescript
   :linenos:

    declare class C {
        (someArg: number): boolean
        (someArg: string): boolean
        ...
    }

*环境类签名声明* 仅在环境上下文中受支持。如果写在 |LANG| 中，则环境类实现必须符合 :ref:`Callable Types with $_invoke Method`。

在环境类声明中，只要多个 *环境调用签名* 彼此可区分（见 :ref:`Declaration Distinguishable by Signatures`），
就允许同时存在多个。多个不同环境调用签名如下例所示：

.. code-block:: typescript
   :linenos:

   // sdk_file.d.ets, declaration file
   export declare class C {
      (x: string): void
      (x: number): void
   }

   // sdk_file.ets, implementation file
   export class C {
      static $_invoke(x: string): void {
         console.log('string')
      }
      static $_invoke(x: number): void {
         console.log('number')
      }
   }

   // app.ets
   import { C } from './sdk_file'

   C(123)    // log: number
   C('abc')  // log: string

.. index::
   ambient call signature declaration
   ambient call signature
   callable type
   ambient context
   compatibility
   syntax
   restriction
   ambient class declaration

|

.. _Ambient Iterable:

环境可迭代声明
================================================================================

.. meta:
    frontend_status: Done

*环境可迭代声明* 表示某个类实例在环境上下文中是可迭代的。该特性为兼容 |TS| 而提供：

*环境可迭代声明* 的语法如下：

.. code-block:: abnf

    ambientIterableDeclaration:
        '[Symbol.iterator]' '(' ')' returnType
        ;


适用以下限制：

- *returnType* 必须是实现了 :ref:`Standard Library` 中定义的 ``Iterator`` 接口的类型；
- 环境类声明中只允许一个 *环境可迭代声明*。

.. code-block:: typescript
   :linenos:

    declare class C {
        [Symbol.iterator] (): CIterator
    }

.. note::
   *环境可迭代声明* 仅在环境上下文中受支持。
   如果写在 |LANG| 中，则环境类实现必须符合 :ref:`Iterable Types`。

.. index::
   ambient iterable
   ambient iterable declaration
   class instance
   ambient context
   iterable class instance
   ambient context
   compatibility
   syntax
   return type
   restriction
   implementation
   interface
   ambient class
   implementation

|

.. _Ambient Interface Declarations:

环境接口声明
********************************************************************************

.. meta:
    frontend_status: Done

*环境接口声明* 的语法如下：

.. code-block:: abnf

    ambientInterfaceDeclaration:
        'interface' identifier typeParameters?
        interfaceExtendsClause?
        '{' ambientInterfaceMember* '}'
        ;

    ambientInterfaceMember
        : interfaceProperty
        | ambientInterfaceMethodDeclaration
        | ambientIndexerDeclaration
        | ambientIterableDeclaration
        ;

    ambientInterfaceMethodDeclaration:
        'default'? identifier signature
        ;

*环境接口* 可以像环境类一样包含附加成员（见 :ref:`Ambient Indexer` 和 :ref:`Ambient Iterable`）。

.. index::
   syntax
   ambient interface
   ambient interface declaration
   ambient class
   ambient indexer
   ambient iterable

如果接口方法声明带有关键字 ``default``，则对应的非环境接口必须按如下方式为该方法提供默认实现：

.. code-block:: typescript
   :linenos:

    declare interface I1 {
        default foo (): void // method foo will have the default implementation
    }
    class C1 implements I1 {} // Class C1 is valid as foo() has the default implementation

    interface I1 {
        // If such interface is used as I1 it will be runtime error as there is
        // no default implementation for foo()
        foo (): void
    }

    declare interface I2 {
        foo (): void // method foo has no default implementation
    }
    class C2 implements I2 {} // Class C2 is invalid as foo() has no implementation
    class C3 implements I2 { foo() {} } // Class C3 is valid as foo() has implementation


.. index::
   interface method
   default keyword
   non-ambient interface
   runtime error
   method
   ambient interface declaration
   ambient class
   default implementation

|

.. _Ambient Enumeration Declarations:

环境枚举声明
********************************************************************************

.. meta:
    frontend_status: None

*环境枚举声明* 的语法如下：

.. code-block:: abnf

    ambientEnumDeclaration
        : 'const'? 'enum' identifier enumBaseType? '{' ambientEnumMemberList? '}'
        ;

    ambientEnumMemberList:
        identifier (',' identifier)* ','?
        ;

如果 *枚举声明* 带有关键字 ``const`` 前缀，则会发生 :index:`compile-time error`。
这一限制是临时的，``const enum`` 的语义将在未来版本的 |LANG| 中提供。

枚举声明的任何成员都不能带有初始化器。否则会像下例所示发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    declare enum RGB {Red, Green, Blue} // OK

    declare enum Err1 { A = 5 }      // Compile-time error, initializer is present

|

.. _Ambient Namespace Declarations:

环境命名空间声明
********************************************************************************

.. meta:
    frontend_status: Done

命名空间用于在逻辑上对多个实体进行分组。|LANG| 支持 *环境命名空间*，以更好兼容 |TS|。
|TS| 常用环境命名空间来描述平台 API 或第三方库 API。

*环境命名空间声明* 的语法如下：

.. code-block:: abnf

    ambientNamespaceDeclaration:
        'namespace' identifier '{' ambientNamespaceElement* '}'
        ;

    ambientNamespaceElement:
        ambientNamespaceElementDeclaration | exportDirective
    ;

    ambientNamespaceElementDeclaration:
        'export'?
        ( ambientConstantOrVariableDeclaration
        | ambientFunctionDeclaration
        | explicitFunctionOverload
        | ambientClassDeclaration
        | ambientInterfaceDeclaration
        | ambientNamespaceDeclaration
        | ambientAccessorDeclaration
        | ambientEnumDeclaration
        | typeAlias
        )
        ;

只有被导出的实体才能在命名空间外部访问。

命名空间可以嵌套：

.. code-block:: typescript
   :linenos:

    declare namespace A {
        export namespace B {
            export function foo(): void;
        }
    }

命名空间不是对象，它仅仅是一个作用域，其中的实体只能通过限定名访问。

.. index::
   namespace
   ambient namespace
   ambient namespace declaration
   entity
   compatibility
   syntax
   platform API
   third-party library API
   ambient iterable declaration
   declared type
   access
   const keyword
   enumeration type declaration
   prefix
   declared type

如果从某个模块导入环境命名空间，则该环境命名空间中的全部声明在当前模块的所有声明和顶层语句中都是可访问的
（见 :ref:`Accessible`）。

.. code-block:: typescript
   :linenos:

    // File1.d.ets
    export declare namespace A { // namespace itself must be exported
        function foo(): void
        type X = Array<number>
    }

    // File2.ets
    import {A} from 'File1.d.ets'

    A.foo() // Valid function call, as 'foo' is accessible for top-level statements
    function foo () {
        A.foo() // Valid function call, as 'foo' is accessible here as well
    }
    class C {
        method () {
            A.foo() // Valid function call, as 'foo' is accessible here too
            let x: A.X = [] // Type A.X can be used
        }
    }

如果某个 *环境命名空间* 声明包含了一个引用到不属于该命名空间声明的 *exportDirective*，
则会发生 :index:`compile-time error`。

.. code-block:: typescript
   :linenos:

    export declare namespace A {
         export {foo} // Compile-time error, no 'foo' in namespace 'A'
    }
    function foo() {}

.. index::
   ambient namespace
   ambient namespace declaration
   accessible declaration
   access
   accessibility
   top-level statement
   module

|

.. _Implementing Ambient Namespace Declaration:

实现环境命名空间声明
================================================================================

.. meta:
    frontend_status: Done

如果某个 *环境命名空间* 由 |LANG| 实现，则必须在模块顶层声明一个同名命名空间
（见 :ref:`Namespace Declarations`）。嵌套命名空间（即嵌入到另一个命名空间中的命名空间）的所有名称，
都必须与环境上下文中的名称相同。


.. index::
   ambient namespace declaration
   ambient namespace
   entity
   implementation
   namespace declaration
   namespace name
   declaration
   top-level declaration
   module
   ambient context
   nested namespace
   embedded namespace

|

.. _Ambient Accessor Declarations:

环境 accessor 声明
********************************************************************************

.. meta:
    frontend_status: None

*环境 accessor 声明* 是 :ref:`Accessor Declarations` 的环境版本。其语法如下：

.. code-block:: abnf

    ambientAccessorDeclaration:
        ( 'get' identifier '(' receiverParameter? ')' returnType
        | 'set' identifier '(' (receiverParameter ',')? requiredParameter ')'
        )
        ;

如果环境 getter 声明未指定显式返回类型，则会发生编译时错误。

.. code-block:: typescript
   :linenos:

    declare get name(): string // OK
    declare get age() // Compile-time error, return type must be specified

详情见 :ref:`Accessor Declarations`。

|

.. _Ambient Type Alias Declarations:

环境类型别名声明
********************************************************************************

.. meta:
    frontend_status: None

*环境类型别名声明* 与类型别名声明具有相同语义（见 :ref:`Type Alias Declaration`）。
它不要求在其他位置提供任何非环境声明。

.. code-block:: typescript
   :linenos:

    declare type A = number
    declare function foo (p: A): A

    // No need to have 'type A = number' declared again

.. raw:: pdf

   PageBreak
