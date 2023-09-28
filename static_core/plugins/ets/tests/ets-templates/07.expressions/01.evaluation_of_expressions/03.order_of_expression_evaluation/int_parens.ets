{% for ia in int_assoc %}

/*---
desc: Integer addition and multiplication are associative
---*/

function main(): void {
  assert {{ia.left}} == {{ia.right}};
}

{% endfor %}