{% for ins in class_instance %}

/*--- 
desc: Class instance creation expression
params: {{ins.init}}
---*/

{{ins.decl}}

function main(): void {
  let x = {{ins.init}};
}

{% endfor %}