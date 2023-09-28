/*---
desc: Class extends itself.
assert: It is a compile-time error if a class depends on itself.
tags: [compile-only, negative]
---*/

class A extends A {

}
