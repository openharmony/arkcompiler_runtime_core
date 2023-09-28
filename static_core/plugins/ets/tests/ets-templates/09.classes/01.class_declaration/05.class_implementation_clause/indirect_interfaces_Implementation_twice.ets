/*---
desc: Direct implement one interface two times.
assert: It is a compile-time error if the same interface is mentioned as a direct superinterface more than once in a single implements clause. This is true even if the interface is named in different ways.
tags: [compile-only, negative]
---*/

interface A { }

interface B extends A { }

class C implements A, B { }
