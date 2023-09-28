{% for i in default_value %}

/*---
desc: Predefined class type {{i}} declaration.
params: {{i}}
---*/

let x: {{i}};
function main(): void {
  let y: {{i}};
}

{% endfor %}