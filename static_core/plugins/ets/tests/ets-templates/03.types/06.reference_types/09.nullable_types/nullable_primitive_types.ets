{% for t in types %}

/*---
desc: null in primitive type {{t}} 
params: {{t}}
tags: [negative, compile-only]
---*/

function main(): void {
  let x: {{t}} = null;
}

{% endfor %}