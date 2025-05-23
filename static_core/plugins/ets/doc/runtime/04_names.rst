..
    Copyright (c) 2021-2024 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

.. _Runtime Names:

Runtime Names
#############

|LANG| runtime (including internal ``ClassLinker`` APIs, standard library
reflection and class loading APIs) and build-time (build system, frontend,
bytecode manipulation tools) both work with modules, classes and other entities
through **runtime names**.

A runtime name is formed according to the following rules:

- |LANG| compiler associates a **name** with a **group** of source code files
  compiled within a single compilation configuration file
  (``build-config.json``).

- |LANG| compiler limits relative imports to be in the **same group** of
  source code files.

- |LANG| compiler obtains the **current group name** from the current
  compilation configuration file.

- If the imported file is from the **external group** of source code files,
  the external group name is obtained from the corresponding external
  compilation configuration file.

.. code-block:: json
    :linenos:

    /* In compilation configuration file: */
    {
        "name": "@app",
        "paths": {
            "@ohos/apix": "../../somewhere/within/sdk",
            "@lib": "../../some/path"
        }
    }

.. code-block:: typescript
    :linenos:

    import { A } from "@ohos/apix/fs"           // "fs" is associated with the name "@ohos/apix"
    import { B } from "@lib/components/buttons" // "components/buttons" is associated with name "@lib"
    import { C } from "../bar"                  // "bar" is associated with the name "@app"

- |LANG| compiler computes the **module name** for each file according to the
  **group name** and the relative path. ``@ohos/apix/fs`` will have module
  name ``@ohos.apix.fs``.

- Module itself uses the same module name as its runtime name.

- Entities inside the module are prefixed with the module name. As shown below,
  classes are named `@ohos.apix.fs.A` and so on.

Modules within a well-formed build-system are guaranteed to not clash by runtime
name. Well-formed assumes parties do not assign same **group** name, ``.abc``
files are not patched.

.. _Comprehensive Runtime Names Example:

Comprehensive Runtime Names Example
===================================

Let's assume the following source code in ``appsrc/mod.ets``:

.. code-block:: typescript
    :linenos:

    export function foo() {
        return new F()
    }
    export let x001 = foo()
    export let x002 = foo()

    export namespace NS {
        let x1 = foo()
    }
    export namespace NS {
        class C {
            static x2 = foo()
        }
    }

    export class C {
        static x3 = foo()
    }

Runtime names are generated as follows (illustrated with pseudo-code):

.. code-block:: typescript
    :linenos:

    // Build system maps `appsrc` to `@app`
    class @app.mod.GLOBAL { // module top-level
        static x001: F
        static x002: F
        static foo() {
            return new F()
        }
        static {
            @app.mod.GLOBAL.x001 = @app.mod.GLOBAL.foo()
            @app.mod.GLOBAL.x002 = @app.mod.GLOBAL.foo()
        }
    }

    class @app.mod.NS { // merged namespace NS
        static x1: F
        static {
            @app.mod.NS.x1 = @app.mod.GLOBAL.foo()
        }
    }

    class @app.mod.NS.C { // class C inside namespace NS
        static x2: F
        static {
            @app.mod.NS.C.x2 = @app.mod.GLOBAL.foo()
        }
    }

    class @app.mod.C { // class C inside module
        static x3: F
        static {
            @app.mod.C.x3 = @app.mod.GLOBAL.foo()
        }
    }

.. _Name mangling for bytecode:

Name mangling
=============

We use the following easily readable name format to display all the mangled names in the bytecode:

.. code-block:: abnf

    mangledName:
        specialCharacter
        '-' elementType
        ('-' additionalInformation)?
        ('-' counter)?

where:

- **specialCharacter** is the ``%%`` symbol
- **elementType** is a predefined set of names to identify the entity
- **additionalInformation** is an optional part a developer can use to provide further information about the entity, if necessary
- **counter** is an optional counter, if needed

The following table lists all the currently used **elementType** values:

.. table::
    :widths: 25, 25, 25, 25

    ==================== ==================== ==================== ====================
    ``annotation``       ``get``              ``lit_array_prop``   ``property``        
    -------------------- -------------------- -------------------- --------------------
    ``async``            ``lambda_invoke``    ``local_class``      ``set``             
    -------------------- -------------------- -------------------- --------------------
    ``array_type``       ``lambda_obj``       ``module_init``      ``union_prop``      
    -------------------- -------------------- -------------------- --------------------
    ``enum``             ``lit_array``        ``partial``                              
    ==================== ==================== ==================== ====================


All the currently used mangled names in the bytecode can be found in this table:

.. table::
    :widths: 35, 65

    ========================================== =========================================================
    **Entity type**                            **Mangled name**                                     
    ========================================== =========================================================
    ``annotation``                             ``%%-annotation-annotation_name``                    
    ------------------------------------------ ---------------------------------------------------------
    ``async``                                  ``%%-async-function_name``                       
    ------------------------------------------ ---------------------------------------------------------
    ``array_type``                             ``%%-array_type-base_name-classProp_name``           
    ------------------------------------------ ---------------------------------------------------------
    ``enum``                                   ``%%-enum-enum_name``                                
    ------------------------------------------ ---------------------------------------------------------
    ``get``                                    ``%%-get-member_name``                               
    ------------------------------------------ ---------------------------------------------------------
    ``lambda_invoke``                          ``%%-lambda_invoke-counter``                         
    ------------------------------------------ ---------------------------------------------------------
    ``lambda_obj``                             ``%%-lambda_obj-lambda_invoke-counter``
    ------------------------------------------ ---------------------------------------------------------
    ``lit_array``                              ``%%-lit_array-base_name``                           
    ------------------------------------------ ---------------------------------------------------------
    ``lit_array_prop``                         ``%%-lit_array_prop-base_name-field_name``           
    ------------------------------------------ ---------------------------------------------------------
    ``local_class``                            ``%%-local_class-class_name-counter``                
    ------------------------------------------ ---------------------------------------------------------
    ``module_init``                            ``%%init``
    ------------------------------------------ ---------------------------------------------------------
    ``partial``                                ``%%-partial-class_name``                            
    ------------------------------------------ ---------------------------------------------------------
    ``property``                               ``%%-property-member_name``                            
    ------------------------------------------ ---------------------------------------------------------
    ``set``                                    ``%%-set-member_name``                               
    ------------------------------------------ ---------------------------------------------------------
    ``union_prop``                             ``%%-union_prop-prop_type-prop_name``                
    ========================================== =========================================================

Simple examples to show from which STS code what mangled name is created:

.. table::
    :widths: 45, 55

    +----------------------------------------------------+-------------------------------------------------------+
    | **Annotation**                                     | **Mangled name**                                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     @interface ExampleAnnotation {}                |     %%-annotation-ExampleAnnotation                   |
    +----------------------------------------------------+-------------------------------------------------------+

.. table::
    :widths: 60, 40

    +----------------------------------------------------+-------------------------------------------------------+
    | **Async function**                                 | **Mangled name**                                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     async function exampleFunc() {}                |     the original name will not change                 |
    |                                                    |                                                       |
    |     //The generated pair of the original.          |     %%-async-exampleFunc                              |
    |     public static %%-async-exampleFunc(): Object {}|                                                       |
    +----------------------------------------------------+-------------------------------------------------------+

.. table::
    :widths: 45, 55

    +----------------------------------------------------+-------------------------------------------------------+
    | **Enum**                                           | **Mangled name**                                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     enum exampleEnum {Red, Blue, Green}            |     %%-enum-exampleEnum                               |
    +----------------------------------------------------+-------------------------------------------------------+

.. table::
    :widths: 45, 55

    +----------------------------------------------------+-------------------------------------------------------+
    | **Get (getter)**                                   | **Mangled name**                                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     interface ExampleInterface {                   |     %%-get-testMember                                 |
    |         get testMember(): String                   |                                                       |
    |     }                                              |                                                       |
    +----------------------------------------------------+-------------------------------------------------------+

.. table::
    :widths: 40, 60

    +----------------------------------------------------+-------------------------------------------------------+
    | **Lambda object**                                  | **Mangled name**                                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     let exampleFunc = (): void => {}               |     %%-lambda_obj-lambda_invoke-0                     |
    +----------------------------------------------------+-------------------------------------------------------+

.. table::
    :widths: 40, 60

    +----------------------------------------------------+-------------------------------------------------------+
    | **Local class**                                    | **Mangled name**                                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     function exampleFunc() {                       |     %%-local_class-LClass-0                           |
    |       class LClass {}                              |                                                       |
    |     }                                              |                                                       |
    +----------------------------------------------------+-------------------------------------------------------+

.. table::
    :widths: 60, 40

    +----------------------------------------------------+-------------------------------------------------------+
    | **Partial**                                        | **Mangled name**                                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     interface TestType {}                          |     %%-partial-TestType                               |
    |                                                    |                                                       |
    |     function exampleFunc(a0: Partial<TestType>) {} |                                                       |
    +----------------------------------------------------+-------------------------------------------------------+

.. table::
    :widths: 60, 40

    +----------------------------------------------------+-------------------------------------------------------+
    | **Inherited property access in class**             | **Mangled name**                                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     interface TestInterface {                      |     %%-property-testMember                            |
    |       testMember: String                           |                                                       |
    |     }                                              |                                                       |
    |                                                    |                                                       |
    |     class TestClass implements TestInterface {     |                                                       |
    |       testMember = "hello"                         |                                                       |
    |     }                                              |                                                       |
    +----------------------------------------------------+-------------------------------------------------------+

.. table::
    :widths: 45, 55

    +----------------------------------------------------+-------------------------------------------------------+
    | **Set (setter)**                                   | **Mangled name**                                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     interface ExampleInterface {                   |     %%-set-testMember                                 |
    |       set testMember(a0: String): void             |                                                       |
    |     }                                              |                                                       |
    +----------------------------------------------------+-------------------------------------------------------+

.. table::
    :widths: 35, 65

    +----------------------------------------------------+-------------------------------------------------------+
    | **Union property access**                          | **Mangled name**                                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     class A {                                      |     //it will only be generated when accessing        |
    |       testMember: Number = 9                       |     //the 'testMember' member in 'foo' through 'a0'   |
    |     }                                              |     %%-union_prop-std_core_Double-testMember          |
    |                                                    |                                                       |
    |     class B {                                      |                                                       |
    |       testMember: Number = 8                       |                                                       |
    |     }                                              |                                                       |
    |                                                    |                                                       |
    |     function foo(a0: A|B) {                        |                                                       |
    |       let p = a0.testMember                        |                                                       |
    |     }                                              |                                                       |
    +----------------------------------------------------+-------------------------------------------------------+

All the other entities in the bytecode (some has non-mangled names, others are not represented):

.. table::
    :widths: 35, 65

    ==================== ====================
    **Primitive types**  **Name in bytecode**
    ==================== ====================
    ``byte``             ``i8``              
    -------------------- --------------------
    ``short``            ``i16``             
    -------------------- --------------------
    ``int``              ``i32``             
    -------------------- --------------------
    ``long``             ``i64``             
    -------------------- --------------------
    ``float``            ``f32``             
    -------------------- --------------------
    ``double/number``    ``f64``             
    -------------------- --------------------
    ``boolean``          ``u1``              
    -------------------- --------------------
    ``char``             ``u16``             
    ==================== ====================


.. table::
    :widths: 35, 65

    ==================================== =====================================================================
    **Built-in reference types**         **Representation in the bytecode**                
    ==================================== =====================================================================
    ``object/Object``                    ``std.core.Object``                 
    ------------------------------------ ---------------------------------------------------------------------
    ``string/String``                    ``std.core.String``                 
    ------------------------------------ ---------------------------------------------------------------------
    ``simple array``                     ``typename_in_bytecode + "[" + "]"``
    ------------------------------------ ---------------------------------------------------------------------
    ``void``                             ``void``                            
    ------------------------------------ ---------------------------------------------------------------------
    ``null``                             ``std.core.Object``                 
    ------------------------------------ ---------------------------------------------------------------------
    ``undefined``                        ``std.core.Object``                 
    ------------------------------------ ---------------------------------------------------------------------
    ``boxed types``                      ``"std.core." + boxed_type_name``   
    ------------------------------------ ---------------------------------------------------------------------
    ``tuple``                            an array of the least upper bound type of the types used in the tuple
    ==================================== =====================================================================

.. table::
    :widths: 35, 65

    ================================ ===============================================================
    **User defined reference types** **Representation in the bytecode**            
    ================================ ===============================================================
    ``function type``                ``std.core.Function + counter`` (counter: number of parameters)
    -------------------------------- ---------------------------------------------------------------
    ``union type``                   the least upper bound type of the types used in the union
    -------------------------------- ---------------------------------------------------------------
    ``type parameter``               ``std.core.Object`` (because of type erasure)           
    -------------------------------- ---------------------------------------------------------------
    ``literal type with string``     ``std.core.String``                                      
    -------------------------------- ---------------------------------------------------------------
    ``literal type with null``       ``std.core.Object``                                      
    -------------------------------- ---------------------------------------------------------------
    ``literal type with undefined``  ``std.core.Object``                                      
    -------------------------------- ---------------------------------------------------------------
    ``class/interface/function``     ``"." + class/interface/function_name``                  
    -------------------------------- ---------------------------------------------------------------
    ``namespace``                    ``"." + namespace_name``                  
    ================================ ===============================================================

.. table::
    :widths: 35, 65

    ================================ ===============================================================
    **Other utility types**          **Representation in the bytecode**                       
    ================================ ===============================================================
    ``Required``                     not represented in the bytecode                          
    -------------------------------- ---------------------------------------------------------------
    ``Readonly``                     not represented in the bytecode                          
    ================================ ===============================================================

Other examples related to non-mangled names:

.. table::
    :widths: 45, 55

    +----------------------------------------------------+-------------------------------------------------------+
    | **Class**                                          | **How it looks in the bytecode**                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     class TestClass {}                             |     .TestClass                                        |
    +----------------------------------------------------+-------------------------------------------------------+

.. table::
    :widths: 45, 55

    +----------------------------------------------------+-------------------------------------------------------+
    | **Function**                                       | **How it looks in the bytecode**                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     function testFunction() {}                     |     .testFunction                                     |
    +----------------------------------------------------+-------------------------------------------------------+

.. table::
    :widths: 45, 55

    +----------------------------------------------------+-------------------------------------------------------+
    | **Interface**                                      | **How it looks in the bytecode**                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     interface TestInterface {}                     |     .TestInterface                                    |
    +----------------------------------------------------+-------------------------------------------------------+

.. table::
    :widths: 45, 55

    +----------------------------------------------------+-------------------------------------------------------+
    | **Namespace**                                      | **How it looks in the bytecode**                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     namespace TestNamespace {}                     |     .TestNamespace                                    |
    +----------------------------------------------------+-------------------------------------------------------+

.. table::
    :widths: 45, 55

    +----------------------------------------------------+-------------------------------------------------------+
    | **Array**                                          | **How it looks in the bytecode**                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     let arr: int[] = [1, 2, 3]                     |     i32[]                                             |
    +----------------------------------------------------+-------------------------------------------------------+

.. table::
    :widths: 45, 55

    +----------------------------------------------------+-------------------------------------------------------+
    | **Union**                                          | **How it looks in the bytecode**                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     class A {}                                     |     Type of 'testVar' in the bytecode is 'A',         |
    |                                                    |     because 'A' is the LUB (least upper bound)        |
    |     class B extends A {}                           |     type of 'B' and 'C'. A union is represented       |
    |                                                    |     as the LUB of the types used in the union         |
    |     class C extends A {}                           |     type.                                             |
    |                                                    |                                                       |
    |     let testVar: A | B                             |                                                       |
    +----------------------------------------------------+-------------------------------------------------------+

.. table::
    :widths: 45, 55

    +----------------------------------------------------+-------------------------------------------------------+
    | **Tuple**                                          | **How it looks in the bytecode**                      |
    +====================================================+=======================================================+
    | .. code-block:: typescript                         | .. code-block:: typescript                            |
    |                                                    |                                                       |
    |     class A {}                                     |     Type of 'testTuple' in the bytecode is 'A[]',     |
    |                                                    |     because 'A' is the LUB (least upper bound)        |
    |     class B extends A {}                           |     type of 'B' and 'C'. A tuple is represented       |
    |                                                    |     as an array of the LUB type of the types used     |
    |     class C extends A {}                           |     in the tuple.                                     |
    |                                                    |                                                       |
    |     let testTuple: [B, C]                          |                                                       |
    +----------------------------------------------------+-------------------------------------------------------+
