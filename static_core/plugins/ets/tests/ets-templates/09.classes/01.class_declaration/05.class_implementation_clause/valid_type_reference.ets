/*---
desc: Implement interface.
assert: Each typeReference must name an accessible interface type (link to access control), or a compile-time error occurs.
tags: [compile-only]
---*/

class A implements C, D { }

interface C { }
interface D { }
