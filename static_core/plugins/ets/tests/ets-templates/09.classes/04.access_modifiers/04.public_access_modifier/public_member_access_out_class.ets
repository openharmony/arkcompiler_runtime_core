{% for member in members %}
/*---
desc: Check member with public access modifier out class.
assert: Member or constructor with public modifier can be accessed everywhere.
params: "{{member.body}}"
---*/

public class A {
  public {{member.body}}
}

function main(): void {
  let instance: A = new A();

  instance.{{member.call}};
}

{% endfor %}