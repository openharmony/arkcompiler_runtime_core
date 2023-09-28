{% for i in logical_ops %}

/*---
desc: Bitwise logical operations (bool)
params: {{i.xorigin}} {{i.op}} {{i.yorigin}} == {{i.dest}}
---*/

function main(): void {
  let x: boolean = {{i.xorigin}};
  let y: boolean = {{i.yorigin}};
  let dest: boolean = {{i.dest}};
  assert((x {{i.op}} y) == dest);
}

{% endfor %}
