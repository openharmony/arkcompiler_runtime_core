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

.. _Support for GUI Programming:

Support for GUI Programming
###########################

This section describes the built-in mechanisms that |LANG| provides to create
graphical user interface (GUI) programs. The section is based on the ArkUI
Declarative Core Language Specification available at
<https://gitee.com/arkui-finland/arkui-edsl-core-spec/blob/master/arkui-core-spec.md>.

ArkUI provides a set of extensions of the standard to declaratively describe
the programs’ GUI, and the interaction between the GUI components. |LANG| aims
to adopt ArkUI syntax and semantics as long as they do not contradict |LANG|.

This section is actively under development, and some of its parts can be still
underspecified. In such cases please refer to the original specification.

.. index::
   GUI programming

|

.. _GUI Structs:

GUI Structs
***********

.. meta:
    frontend_status: Partly
    
*GUI structs* are used to define UI components. From the language
perspective, a GUI struct is a restricted form of a non-primitive type
that is designed to define GUI expressively and efficiently.

Each GUI struct is required to implement its *builder* (i.e., the method
responsible for the visual rendering of components).

.. index::
    GUI struct
    struct type
    builder
    implementation
    visual rendering
    method

.. code-block:: abnf

    guiStructDeclaration:
        guiEntryAnnotation? guiComponentAnnotation 'struct' identifier
        guiStructBody
        ;

    guiStructBody:
        '{'
        guiStructBodyDeclaration*
        guiMainComponentBuilderDeclaration
        guiStructBodyDeclaration*
        '}'
        ;

    guiStructBodyDeclaration:
        guiAccessModifier?
        (
        | guiStructFieldDeclaration
        | guiLifeCycleCallbackDeclaration
        | guiCustomComponentBuilderDeclaration
        | classFieldDeclaration
        | classMethodDeclaration
        )
        ;

    guiAccessModifier:
        'private'
        ;

    guiStructFieldDeclaration:
        guiStructFieldAnnotationDeclaration
        variableDeclaration
        ;

    guiStructFieldAnnotationDeclaration:
        guiBuilderParamAnnotation
        | ( guiDataSynchronizationAnnotation guiWatchAnnotation? )
        ;

    guiDataSynchronizationAnnotation:
        guiConsumeAnnotation
        | guiLinkAnnotation
        | guiLocalStorageLinkAnnotation
        | guiLocalStoragePropAnnotation
        | guiObjectLinkAnnotation
        | guiPropAnnotation
        | guiProvideAnnotation
        | guiStateAnnotation
        | guiStorageLinkAnnotation
        | guiStoragePropAnnotation
        ;

    guiMainComponentBuilderDeclaration:
        guiAccessModifier?
        'build'
        '(' ')' (':' 'void')? block
        ;

    guiCustomComponentBuilderDeclaration:
        guiBuilderAnnotation
        guiAccessModifier?
        identifier
        '(' ')' (':' 'void')? block
        ;

    guiLifeCycleCallbackDeclaration:
        guiAccessModifier?
        ( 'aboutToAppear' | 'aboutToDisappear' )
        '(' ')' ':' 'void' block
        ;

|

.. _Builder Function Syntax Conventions:

Builder Function Syntax Conventions
***********************************

The following syntax conventions apply to any builder function (component’s
main builder, component’s custom builder, or stand-alone global custom builder):

-  The required result of ``C({...})`` for any predefined or custom component
   *C* is to initialize and render the component with the data from the
   ``{...}`` block. Specific semantics depends on the implementation. For
   illustrative purposes, it can be expressed as ``(new C({...})).build()``,
   where the object literal ``{...}`` is handled as an initializer of the
   component’s fields.

-  The required result of ``C() {...}`` for any predefined or custom component
   *C* is to initialize and render the component by passing the data
   from the ``{...}`` block to the component’s builder function. Specific
   semantics depends on the implementation. For illustrative purposes, it can
   be expressed as ``new C().build({...})``, where the ``{...}`` block is
   handled as a lambda to be passed as an argument to the builder.

.. index::
   builder
   builder function
   initializer
   initialization
   object literal
   literal
   implementation
   rendering
   argument

|

.. _Builder Function Restrictions:

Builder Function Restrictions
*****************************

Restrictions apply to any builder function (component’s main builder,
component’s custom builder, or stand-alone global custom builder). The
following is not allowed:

-  Declaring local variables;

-  Constructing new objects; and

-  Function calls, except the following:

    -  Calling builders by name.

    -  Calling builders by the reference stored in the
       ``@BuilderParam``-annotated struct field.

    -  Calling a predefined builder ``ForEach`` for iterative rendering.

    -  Calling a function that does not mutate the program state (note that all
       logging functions mutate the state, and thus are prohibited).

    - Using conditional ``if ... else`` syntax.

.. index::
    builder
    builder function
    builder function restriction
    local variable
    construction
    conditional rendering
    iterative rendering
    rendering
    function call
    predefined builder
    builder call
    global custom builder

|

.. _Additional Features:

Additional Features
*******************

|

.. _Methods Returning this:

Methods Returning ``this``
==========================

.. meta:
    frontend_status: Done

A return type of an instance method of a class or a struct can be ``this``.
It means that the return type is the class or struct type the method belongs to.
 
The extended grammar for a method signature (see :ref:`Signatures`) is as
follows:

.. code-block:: abnf

    returnType:
        ':' (type | 'this')
        ;


The only result that is allowed to be returned from such a method is ``this``:

.. code-block:: typescript
   :linenos:

    class C {
        foo(): this {
            return this
        }
    }


The return type of an overridden method in a subclass must also be ``this``:

.. code-block:: typescript
   :linenos:

    class D extends C {
        foo(): this {
            return this
        }
    }

    let x = new C().foo() // type of 'x' is 'C'
    let y = new D().foo() // type of 'y' is 'D'

Otherwise, a :index:`compile-time error` occurs.

|

.. _Unary operator $$:

Unary operator '$$'
===================

.. meta:
    frontend_status: None

A prefix unary operator '``$$``' is used to pass primitive types by reference.
It is added to |LANG| to support the legacy ArkUI code.
As the use of this operator is deprecated, it is to be removed in the future
versions of the language.

The operator '$$' can be followed by an identifier. The code ``$$this.a`` is
considered to be the same as ``$$ this.a`` and ``$$(this.a)``.


|

.. _Example of GUI Programming:

Example of GUI Programming
**************************

.. code-block:: typescript
   :linenos:

    // ViewModel classes -----------------------

    let nextId : number = 0

    @Observed class ObservedArray<T> extends Array<T> {
        constructor(arr: T[]) {
            super(arr)
        }
    }

    @Observed class Address {
        street : string
        zip : number
        city : string

        constructor(street : string, zip: number, city : string) {
            this.street = street
            this.zip = zip
            this.city = city
        }
    }

    @Observed class Person {
        id_ : string
        name: string
        address : Address
        phones: ObservedArray<string>

        constructor(
            name: string,
            street : string,
            zip: number,
            city : string,
            phones: string[]
        ) {
            this.id_ = nextId as string
            nextId++
            this.name = name
            this.address = new Address(street, zip, city)
            this.phones = new ObservedArray<string>(phones)
        }
    }

    class AddressBook {
        me : Person
        contacts : ObservedArray<Person>

        constructor(me : Person, contacts : Person[]) {
            this.me = me
            this.contacts = new ObservedArray<Person>(contacts)
        }
    }

    // @Components -----------------------

    /* Renders the name of a Person object and 
       the first number in the phones ObservedArray<string>
       For also the phone number to update we need two 
       @ObjectLink here, person and phones, cannot use
       this.person.phones. Changes of inner Array not observed.
       onClick updates selectedPerson also in 
       AddressBookView, PersonEditView */
    @Component struct PersonView {

        @ObjectLink person : Person
        @ObjectLink phones : ObservedArray<string>

        @Link selectedPerson : Person

        build() {
            Flex({
                direction: FlexDirection.Row,
                justifyContent: FlexAlign.SpaceBetween })
            {
                Text(this.person.name)
                if (this.phones.length != 0) {
                    Text(this.phones[0])
                }
            }
            .height(55)
            .backgroundColor(
                this.selectedPerson.name == this.person.name ? "#ffa0a0" : "#ffffff"
            )
            .onClick(() => {
                this.selectedPerson = this.person
            })
        }
    }

    /* Renders all details
       @Prop get initialized from parent AddressBookView,
       TextInput onChange modifies local copies only on
       "Save Changes" copy all data from @Prop to @ObjectLink,
       syncs to selectedPerson in other @Components. */
    @Component struct PersonEditView {

        @Consume addrBook : AddressBook

        /* Person object and sub-objects owned by the parent Component */
        @Link selectedPerson: Person

        /* editing on local copy until save is handled */
        @Prop name: string = ""
        @Prop address : Address | null = null
        @Prop phones : ObservedArray<string> | null = null

        selectedPersonIndex() : number {
            return this.addrBook.contacts.findIndex(
                (person) => person.id_ == this.selectedPerson.id_
            )
        }

        build() {
            Column() {
                TextInput({ text: this.name})
                    .onChange((value) => {
                        this.name = value
                    })

                TextInput({text: this.address.street})
                    .onChange((value) => {
                        this.address.street = value
                    })

                TextInput({text: this.address.city})
                    .onChange((value) => {
                        this.address.city = value
                    })

                TextInput({text: this.address.zip.toString()})
                    .onChange((value) => {
                        const result = parseInt(value)
                        this.address.zip = isNaN(result) ? 0 : result
                    })

                if (this.phones.length > 0) {
                    ForEach(this.phones, (phone, index) => {
                        TextInput({text: phone})
                            .width(150)
                            .onChange((value) => {
                                console.log(index + ". " + value + " value has changed")
                                this.phones[index] = value
                            })
                    }, (phone, index) => index + "-" + phone)
                }

                Flex({
                    direction: FlexDirection.Row,
                    justifyContent: FlexAlign.SpaceBetween
                }) {
                    Text("Save Changes")
                        .onClick(() => {
                            // copy values from local copy to the provided ref
                            // to Person object owned by  parent Component.
                            // Avoid creating new Objects, modify properties of
                            // existing
                            this.selectedPerson.name           = this.name
                            this.selectedPerson.address.street = this.address.street
                            this.selectedPerson.address.city   = this.address.city
                            this.selectedPerson.address.zip    = this.address.zip
                            this.phones.forEach((phone : string, index : number) => {
                                this.selectedPerson.phones[index] = phone
                            })
                        })

                    if (this.selectedPersonIndex() != -1) {
                        Text("Delete Contact")
                            .onClick(() => {
                                let index = this.selectedPersonIndex()
                                console.log("delete contact at index " + index)

                                // delete found contact
                                this.addrBook.contacts.splice(index, 1)

                                // determine new selectedPerson
                                index = (index < this.addrBook.contacts.length)
                                    ? index
                                    : index - 1

                                // if no contact left, set me as selectedPerson
                                this.selectedPerson = (index >= 0)
                                    ? this.addrBook.contacts[index]
                                    : this.addrBook.me
                            })
                    }
                }
            }
        }
    }

    @Component struct AddressBookView {

        @ObjectLink me : Person
        @ObjectLink contacts : ObservedArray<Person>
        @State selectedPerson: Person | null = null

        aboutToAppear() {
            this.selectedPerson = this.me
        }

        build() {
            Flex({
                direction: FlexDirection.Column,
                justifyContent: FlexAlign.Start
            }) {
                Text("Me:")
                PersonView({
                    person: this.me,
                    phones: this.me.phones,
                    selectedPerson: this.$selectedPerson
                })

                Divider().height(8)

                Flex({
                    direction: FlexDirection.Row,
                    justifyContent: FlexAlign.SpaceBetween
                }) {
                    Text("Contacts:")
                    Text("Add")
                        .onClick(() => {
                            this.selectedPerson = new Person ("", "", 0, "", ["+86"])
                            this.contacts.push(this.selectedPerson)
                        })
                }
                .height(50)

                ForEach(this.contacts,
                    contact => {
                        PersonView({
                            person: contact,
                            phones: contact.phones,
                            selectedPerson: this.$selectedPerson
                        })
                    }, contact => contact.id_
                )

                Divider().height(8)

                Text("Edit:")
                PersonEditView({
                    selectedPerson: this.$selectedPerson,
                    name: this.selectedPerson.name,
                    address: this.selectedPerson.address,
                    phones: this.selectedPerson.phones
                })
            }
            .borderStyle(BorderStyle.Solid)
            .borderWidth(5)
            .borderColor(0xAFEEEE)
            .borderRadius(5)
        }
    }

    @Entry
    @Component struct PageEntry {
        @Provide addrBook : AddressBook = new AddressBook(
            new Person(
                "Mighty Panda",
                "Wonder str., 8",
                888,
                "Shanghai",
                ["+8611122223333", "+8677788889999", "+8655566667777"]
            ),
            [
            new Person(
                "Curious Squirrel",
                "Wonder str., 8",
                888,
                "Hangzhou",
                ["+8611122223332", "+8677788889998", "+8655566667776"]
            ),
            new Person(
                "Wise Tiger",
                "Wonder str., 8",
                888,
                "Nanjing",
                ["+8610101010101", "+8620202020202", "+8630303030303"]
            ),
            new Person(
                "Mysterious Dragon",
                "Wonder str., 8",
                888,
                "Suzhou",
                ["+8610000000000", "+8680000000000"]
            ),
        ]);

        build() {
            AddressBookView({
                me: this.addrBook.me,
                contacts: this.addrBook.contacts,
                selectedPerson: this.addrBook.me
            })
        }
    }

.. raw:: pdf

   PageBreak
