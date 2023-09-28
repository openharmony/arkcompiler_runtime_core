{% for i in logical_ops %}

/*---
desc: Bitwise logical operations (Boolean)
params: {{i.xorigin}} {{i.op}} {{i.yorigin}} == {{i.dest}}
---*/

function main(): void {
  let x: Boolean = {{i.xorigin}};
  let y: Boolean = {{i.yorigin}};
  let dest: Boolean = {{i.dest}};
  assert((x {{i.op}} y) == dest);
}

{% endfor %}
