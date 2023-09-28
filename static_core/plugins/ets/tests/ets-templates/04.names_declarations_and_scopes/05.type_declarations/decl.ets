{% for t in cases %}
/*---
desc: >-
    Type declaration is either interface declaration, class declaration or
    enum declaration. See corresponding chapters.

    Note: See sections: 9.Classes, 10.Interfaces, 11.Enums for full-scale tests on
    each type.
---*/

{{t.defs}}

function main(): void {
  let v: {{t.type}} = {{t.val|safe}};
  assert {{t.assert|safe}};
}
{% endfor %}
