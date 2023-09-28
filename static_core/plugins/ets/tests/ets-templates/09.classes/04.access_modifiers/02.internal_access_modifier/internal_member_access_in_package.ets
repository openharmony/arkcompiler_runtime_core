{% for member in members %}
/*---
desc: Check members with internal access modifier in package.
assert: Member or constructor with private modifier can be accessed only eithin their declaring class.
params: "{{member.body}}"
---*/

package Test;

class A {
  internal {{member.body}}
}

function main(): void {
  let instance: A = new A();

  instance.{{member.call}};
}

{% endfor %}