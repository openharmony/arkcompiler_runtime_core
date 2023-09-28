{% for statement in statements %}
/*---
desc: All statements in the module level must be enclosed in curly braces to form a block.
tags: [compile-only]
---*/

const x: int = 0;

function check(): boolean {
  return true;
}

function pass(): void {
  assert true;
}

{
  {{statement.block}}
}

{% endfor %}