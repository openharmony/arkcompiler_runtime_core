{% for i in default_value %}

/*---
desc: Predefined type array name {{i}} declaration.
params: {{i}}
---*/

let x: {{i}}[];
function main(): void {
  let y: {{i}}[];
}

{% endfor %}