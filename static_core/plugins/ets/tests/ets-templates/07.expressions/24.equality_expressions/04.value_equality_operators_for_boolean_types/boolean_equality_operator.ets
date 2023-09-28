{% for eq in boolean_eq %}

/*---
desc: Boolean equality
params: {{eq.left}} {{eq.op}} {{eq.right}} is {{eq.result}}
---*/

function main(): void {
  assert ({{eq.left}} {{eq.op}} {{eq.right}}) == {{eq.result}};
}

{% endfor %}