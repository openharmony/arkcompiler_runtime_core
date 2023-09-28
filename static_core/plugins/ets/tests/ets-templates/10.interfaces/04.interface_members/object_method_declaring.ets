/*---
desc: Declared not open Object's methods in interface.
assert: It is a compile-time error if the interface explicitly declares such a method m in the case where m is not declared to be open in Object.
tags: [compile-only, negative]
---*/

interface A {
  getClass(): Class;
}

class B implements A {

}