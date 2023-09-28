{% for i in default_value %}

/*---
desc: The numerical comparison operators with type {{i.type}}.
params: {{i.avalue}}, {{i.bvalue}}, {{i.operator}} {{i.result}}
---*/

function main(): void {
  let a: boolean = {{i.avalue}};
  let b: boolean = {{i.bvalue}};

  let x: boolean = {{i.avalue}} {{i.operator}} {{i.bvalue}};

  assert x == {{i.result}}
}

{% endfor %}