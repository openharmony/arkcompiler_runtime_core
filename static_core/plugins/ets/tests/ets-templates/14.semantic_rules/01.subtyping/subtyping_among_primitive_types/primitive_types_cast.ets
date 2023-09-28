{% for t in subtypes %}

/*---
desc: Subtyping relation among the primitive types
params: {{t.type}} >> {{t.cast}}
---*/

function main(): void {
  let instance: {{t.cast}} = 5 as {{t.type}};
}

{% endfor %}