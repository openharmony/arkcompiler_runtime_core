# 方舟运行时公共组件<a name="ZH-CN_TOPIC_0000001138850082"></a>

- [方舟运行时公共组件<a name="ZH-CN_TOPIC_0000001138850082"></a>](#方舟运行时公共组件)
  - [简介<a name="section11660541593"></a>](#简介)
  - [目录<a name="section161941989596"></a>](#目录)
  - [编译构建](#编译构建)
  - [说明](#说明)
    - [使用说明<a name="section1312121216216"></a>](#使用说明)
      - [反汇编器工具概述](#反汇编器工具概述)
  - [相关仓<a name="section1371113476307"></a>](#相关仓)

## 简介<a name="section11660541593"></a>

方舟编译器运行时公共组件（ArkCompiler Runtime Core）是OpenHarmony中语言运行时的公共组件。主要由与语言无关的基础运行库组成，包含承载字节码以及执行字节码所需要相关信息的ArkCompiler File文件组件、支持运行时调试的Debugger Tooling工具组件、提供不同系统平台公共接口的ArkCompiler Base基础库组件、以及与语言无关的公共指令集体系结构ISA等。

**图1** 方舟编译器运行时公共组件架构图：

![方舟编译器运行时公共组件架构图](docs/images/runtime_core_arch.png)

更多信息请参考：[方舟运行时子系统](https://gitee.com/openharmony/docs/blob/master/zh-cn/readme/ARK-Runtime-Subsystem-zh.md)

## 目录<a name="section161941989596"></a>

```
/arkcompiler/runtime_core
├── assembler               # 汇编器，将文本格式的方舟字节码文件(*.pa)转换为二进制格式的字节码文件(*.abc)，具体格式见：docs/assembly_format.md和docs/file_format.md
├── bytecode_optimizer      # 字节码优化器，对方舟字节码进行进一步优化生成带优化的字节码文件(*.abc)
├── cmake                   # Cmake脚本，包含工具链文件和用于定义构建和测试目标的常用cmake函数
├── compiler                # 编译器，主要包含用于字节码优化的IR和passes等，更多信息可参：docs/ir_format.md
├── cross_values            # 交叉值生成器
├── disassembler            # 反汇编器，将二进制格式的方舟字节码文件(*.abc)转换为文本格式的方舟字节码文件(*.pa)
├── docs                    # 包含语言前端、方舟文件格式和运行时的设计文档
├── dprof                   # 用于ArkCompiler运行时收集profile数据
├── gn                      # GN模板和配置文件
├── irtoc                   # IR to code工具，将手工创建的IR编译为目标代码，具体可以参考：docs/irtoc.md
├── isa                     # 字节码ISA描述文件YAML，ruby脚本和模板
├── ldscripts               # 包含链接器脚本，用于在非PIE可执行文件中放置4GB以上的ELF section
├── libark_defect_scan_aux  # 提供二进制格式方舟字节码文件(*.abc)的漏洞扫描接口
├── libpandabase            # ArkCompiler运行时基本库，包含：日志、同步原语、公共数据结构等
├── libpandafile            # 二进制格式的方舟字节码文件(*.abc)源码仓
├── libziparchive           # 提供基于zlib库实现的读写压缩文件接口
├── panda                   # CLI工具，用于执行方舟字节码文件(*.abc)
├── pandastdlib             # 通过方舟汇编语言编写的标准库
├── platforms               # 与系统平台相关的基础接口实现
├── plugins                 # 部分与语言相关的插件
├── quickener               # 加速器
├── runtime                 # ArkCompiler运行时公共组件
├── scripts                 # CI脚本
├── templates               # ruby模板和脚本，处理包括：命令行选项、日志组件、错误消息、事件等
├── tests                   # 测试用例
└── verification            # 字节码验证器，具体可以参考：docs/bc_verification
```

## 编译构建
**Linux平台**
```
$ ./build.sh --product-name rk3568 --build-target arkcompiler/runtime_core:ark_host_linux_tools_packages
```
**Windows平台**
```
$ ./build.sh --product-name rk3568 --build-target arkcompiler/runtime_core:ark_host_windows_tools_packages
```
**Mac平台**
```
$ ./build.sh --product-name rk3568 --build-target arkcompiler/runtime_core:ark_host_mac_tools_packages
```

## 说明
### 使用说明<a name="section1312121216216"></a>

#### 反汇编器工具概述

工具名称为ark\_disasm，用于将二进制格式的方舟字节码文件转换为文本格式的方舟字节码文件。

命令行格式：

```
ark_disasm [选项] 输入文件 输出文件
```

| 选项 | 描述 |
|------------------------|----------------------------------|
| --debug                | 使能调试信息 |
| --debug-file           | 调试信息输出文件路径，默认为std::cout |
| --help                 | 帮助提示 |
| --quiet                | 使能所有"--skip-\*"选项 |
| --skip-string-literals | 跳过对字符串字面量的反汇编 |
| --verbose              | 增加输出文件的注释信息 |
| --version              | 打印版本信息，包括当前和所支持的最低的文件格式版本 |

输入文件：二进制格式的方舟字节码

输出文件：文本格式的方舟字节码


更多使用说明请参考：[方舟运行时使用指南](https://gitee.com/openharmony/arkcompiler_ets_runtime/blob/master/docs/README_zh.md)

## 相关仓<a name="section1371113476307"></a>

**[arkcompiler\_runtime\_core](https://gitee.com/openharmony/arkcompiler_runtime_core)**

[arkcompiler\_ets\_runtime](https://gitee.com/openharmony/arkcompiler_ets_runtime)

[arkcompiler\_ets\_frontend](https://gitee.com/openharmony/arkcompiler_ets_frontend)
