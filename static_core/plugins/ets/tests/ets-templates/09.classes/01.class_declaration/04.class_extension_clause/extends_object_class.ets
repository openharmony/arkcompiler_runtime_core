/*---
desc: Extend Object class.
assert: The extends clause must not appear in the definition of the class Object, or a compile-time error occurs, because it is the primordial class and has no direct superclass.
tags: [compile-only, negative]
---*/

class A {

}

final class Object extends A {
}
