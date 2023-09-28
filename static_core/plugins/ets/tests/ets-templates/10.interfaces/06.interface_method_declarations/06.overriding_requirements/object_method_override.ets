/*---
desc: Override Object's methods in interface which is not abstract.
assert: It is a compile-time error if a default method is override-equivalent with a non-private method of the class Object, because any class implementing the interface will inherit its own implementation of the method.
tags: [compile-only, negative]
---*/

interface A {
  toString(): String {
    return "sd";
  };
}

class B implements A {
  
}
