{% for eq in value_eq %}

/*---
desc: Value equality operators can compare two operands convertible to numeric type, of type boolean or Boolean or of a reference type.
params: {{eq.left}} {{eq.op}} {{eq.right}} is {{eq.result}}
---*/

class A {
  x: int;
}

class B {
  y: int;
}

function main(): void {
  let a1: A = {x: 1};
  let a2: A = {x: 1};
  let a3: A = {x: 2};
  let b1: B = {y: 1};

  assert ({{eq.left}} {{eq.op}} {{eq.right}}) == {{eq.result}};
}

{% endfor %}