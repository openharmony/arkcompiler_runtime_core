{% for case in cases %}
/*---
desc: |-
  22 specific conversions on primitive types are called the narrowing primitive conversions:
    -	short to byte or char
    -	char to byte or short
    -	int to byte, short, or char
    -	long to byte, short, char, or int
    -	float to byte, short, char, int, or long
    -	double to byte, short, char, int, long, or float
---*/

function main(): int {
  {%- for t in case['types'] %}
  let src_{{loop.index}}: {{t.stype}} = {{t.src|safe}};
  if ((src_{{loop.index}} as {{t.dtype}}) != ({{t.dst|safe}}) ) {
      return 1;
  }
  {% endfor %}
  return 0;
}
{% endfor %}
