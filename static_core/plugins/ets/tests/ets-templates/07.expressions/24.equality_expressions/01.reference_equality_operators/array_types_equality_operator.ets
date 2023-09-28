{% for eq in reference_eq %}

/*---
desc: "The value equality operator a == b semantically equals a === null ? b === null : a.equals(b) if both of its operands are of reference types."
params: {{eq.left}} {{eq.op}} {{eq.right}} is {{eq.result}}
---*/


function main(): void {
  let a: int[] = [1, 2, 3];
  let b: int[] = [1, 2, 3];
  let c: int[] = [1, 2, 4];
  let d: int[] = [1, 2, 3, 4];
  let e: int[]|null = null;
  assert ({{eq.left}} {{eq.op}} {{eq.right}}) == {{eq.result}};
}

{% endfor %}