/*---
desc: Interface extends itself.
assert: It is a compile-time error if an interface depends on itself.
tags: [compile-only, negative]
---*/

interface A extends A {

}