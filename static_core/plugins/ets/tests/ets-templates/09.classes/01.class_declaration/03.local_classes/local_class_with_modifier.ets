{% for m in modifiers %}


/*---
desc: Declaring local class in function body with {{m}} modifier.
assert: A compile-time error occurs if a local class or interface declaration has a local class or interface declaration has the modifier static, sealed, or non-sealed.
params: {{m}}
tags: [compile-only, negative]
---*/

function bar(): void {
  {{m}} class B {}
}

{% endfor %}