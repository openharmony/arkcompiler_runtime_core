/*---
desc: A class inherits fields with the same name from its direct superclass and superinterfaces.
assert: It is possible for a class to inherit more than one field with the same name, either from its superclass and superinterfaces or from its superinterfaces alone. Such a situation does not in itself cause a compile-time error.
tags: [compile-only]
---*/

class A {
  x: int;
}

interface C {
  x: int;
}

interface D {
    x: boolean;
}

class B extends A implements C, D{

}
