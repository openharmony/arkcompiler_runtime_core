/*---
desc: Override Object's methods in interface which is not abstract.
assert: It is a compile-time error if the interface explicitly declares a method with a signature that is override-equivalent (link to signatures) to a public method of Object, but which has a different return type, or an incompatible throws clause, or is not abstract.
tags: [compile-only, negative]
---*/

interface A {
  toString(): String {
    return "sd";
  };
}

class B implements A {
  
}
