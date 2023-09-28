/*---
desc: Circularly declared classes.
assert: If circularly declared classes are detected at run time, as classes are loaded, then a ClassCircularityError is thrown.
tags: [compile-only, negative]
---*/

class A extends D {}

class B extends A {}

class C extends B {}

class D extends C {}
