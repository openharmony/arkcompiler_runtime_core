/*---
desc: Circularly declared interfaces.
assert: If circularly declared interfaces are detected at run time, as interfaces are loaded, then a ClassCircularityError is thrown.
tags: [compile-only, negative]
---*/

interface A extends D {}

interface B extends A {}

interface C extends B {}

interface D extends C {}