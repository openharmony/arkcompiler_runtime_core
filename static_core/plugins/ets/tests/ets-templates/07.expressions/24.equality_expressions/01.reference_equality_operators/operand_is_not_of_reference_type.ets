{% for eq in reference_eq %}
/*---
desc: A compile-time error occurs if any operand is not of reference type.
tags: [compile-only, negative]
params: {{eq.left}} {{eq.op}} {{eq.right}}
---*/


function main(): void {
  let a: int = 1;
  let b: String = "Hello";
  let c: String = "Hello";
  assert {{eq.left}} {{eq.op}} {{eq.right}};
}

{% endfor %}



