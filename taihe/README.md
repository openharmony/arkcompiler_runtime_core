# Taihe: 多语言系统接口编程模型

## Taihe 是什么

Taihe 提供了简单易用的 API 发布和消费机制。

对于 API 发布方，Taihe 可以轻松地描述要发布的接口。
```ts
// idl/integer.arithmetic.taihe
function divmod_i32(a: i32, b: i32): (i32, i32);
```

对于 API 的发布和消费方，Taihe 生成各语言的绑定，提供原生的开发体验。
```c++
// 发布 API 为 libinteger.so，源码位于 author/integer.arithmetic.impl.cpp
#include "integer.arithmetic.impl.hpp"

std::tuple<int32_t, int32_t> ohos_int_divmod(int32_t a, int32_t b) {
    return { a / b, a % b };
}

TH_EXPORT_CPP_API_divmod_i32(ohos_int_divmod)
```

Taihe 将 API 的发布方和消费方在二进制级别隔离，允许二者在闭源的情况下独立升级。
```c++
// 使用 libinteger.so 编写用户应用
#include "integer.arithmetic.abi.hpp"
#include <cstdio>

using namespace integer;

int main() {
  auto [quo, rem] = arithmetic::divmod_i32(a, b);
  printf("q=%d r=%d\n", quo, rem);
  return 0;
}
```

## 加入 Taihe 的开发

Ubuntu 环境：运行 `./scripts/install-ubuntu-deps` 来一键安装环境

开发和使用：参见 [文档索引](./docs/README.md)
