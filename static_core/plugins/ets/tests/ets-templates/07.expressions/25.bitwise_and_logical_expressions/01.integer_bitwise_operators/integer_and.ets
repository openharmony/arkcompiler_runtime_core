{% for i in integer_and %}

/*---
desc: Integer 'and' operation
params: {{i.xorigin}} {{i.op}} {{i.yorigin}} == {{i.dest}} ({{i.xbits}} {{i.op}} {{i.ybits}} == {{i.dbits}})
---*/

function main(): void {
  let x: byte = {{i.xorigin}};
  let y: byte = {{i.yorigin}};
  let dest: byte = {{i.dest}};
  assert((x {{i.op}} y) == dest);
}

{% endfor %}