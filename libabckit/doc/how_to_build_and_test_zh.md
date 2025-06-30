# 安装依赖项：

确保安装以下组件：
- repo
- clang++-14
- clang-format-14
- clang-tidy-14
- doxygen
- graphviz
- Git LFS

# 如何下载

```sh
repo init -u https://gitee.com/ark-standalone-build/manifest.git -b master
repo sync -c -j8
repo forall -c 'git lfs pull'
./prebuilts_download.sh
```

# 如何构建和测试

## 构建 Linux 平台的 AbcKit

```sh
# debug模式
./ark.py x64.debug abckit_packages --gn-args="is_standard_system=true abckit_enable=true"
# release模式
./ark.py x64.release abckit_packages --gn-args="is_standard_system=true abckit_enable=true"
```

## 构建 Windows 平台的 AbcKit
```sh
# debug模式
./ark.py mingw_x86_64.debug abckit_packages --gn-args="is_standard_system=true abckit_enable=true"
# release模式
./ark.py mingw_x86_64.release abckit_packages --gn-args="is_standard_system=true abckit_enable=true"
```

## 构建产物的位置

生成的 Abckit 二进制文件和 libabckit.so/libabckit.dll 位于 `out/${target}/arkcompiler/runtime_core/libabckit` 目录。

生成的二进制文件和库依赖于以下目录中的库：
- `out/${target}/arkcompiler/runtime_core/`
- `out/${target}/arkcompiler/ets_runtime/`
- `out/${target}/thirdparty/icu/`
- `out/${target}/thirdparty/zlib/`

注意：根据编译方式将 ${target} 替换为：x64.debug、x64.release、mingw\_x86\_64.debug 或 mingw\_x86\_64.release。

## 运行单元测试

```sh
# debug模式
./ark.py x64.debug abckit_tests --gn-args="is_standard_system=true abckit_enable=true"
# release模式
./ark.py x64.release abckit_tests --gn-args="is_standard_system=true abckit_enable=true"
```

## 使用 Sanitizer 运行单元测试

```sh
# debug模式
./ark.py x64.debug abckit_tests --gn-args="is_standard_system=true abckit_enable=true libabckit_with_sanitizers=true"
# release模式
./ark.py x64.release abckit_tests --gn-args="is_standard_system=true abckit_enable=true libabckit_with_sanitizers=true"
```

# 如何使用 AbcKit

要求：
- AbcKit 适用于 `release` 模式的方舟字节码文件
- 目前 AbcKit 支持从 ArkTS 或 JS 编译的方舟字节码文件
- 目前 AbcKit 不支持 CommonJS 模块

AbcKit 提供 C API 和 C++ API 来读取和修改方舟字节码文件。

用户可以使用 AbcKit API 来实现 Abckit 插件。

有两种使用 Abckit 的方式：可执行二进制文件和动态库。

(1) 可执行二进制文件

实现以下函数：

```cpp
extern "C" int Entry(AbckitFile *file)
{
    // 用户可以在这里使用 AbcKit API。
    return 0;
}
```

将代码编译为动态库（例如 transformer.so）作为 abckit 插件。

运行以下命令来执行 abckit 插件：

```sh
./abckit --plugin-path transformer.so --input-file /path/to/input.abc --output-file /path/to/output.abc
```

(2) 动态库

用户可以加载 libabckit.so/libabckit.dll 然后使用 AbcKit API。

# 全量测试

删除 out 目录，构建 AbcKit，在debug和release模式下执行`self-check.sh` 脚本。该脚本会进行代码格式化、代码检查、单元测试和压力测试。

```sh
./arkcompiler/runtime_core/libabckit/scripts/self-check.sh --dir=/path/to/standalone/root
```

# 代码覆盖率

删除 out 目录，构建 AbcKit，在debug模式下执行 `self-check.sh` 脚本。该脚本会执行单元测试和压力测试，并收集代码覆盖率。

```sh
./arkcompiler/runtime_core/libabckit/scripts/self-check.sh --dir=/path/to/standalone/root --coverage
```
