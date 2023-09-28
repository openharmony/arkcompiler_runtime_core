{% for eq in string_eq %}

/*---
desc: String equality
params: |
  {{eq.left}} {{eq.op}} {{eq.right}} is {{eq.result}}
---*/

function main(): void {
  let a: String = {{eq.left}};
  let b: String = {{eq.right}};
  assert (a {{eq.op}} b) == {{eq.result}};
}

{% endfor %}