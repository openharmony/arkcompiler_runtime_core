### Issue

### 描述/Description

### 原因/Reason

### 修改方案/Scheme


### **[测试项](https://gitee.com/openharmony/arkcompiler_runtime_core/wikis)**

#### 当前GGW中未添加的测试套(请在本地进行验证)
1.Fuzz编译测试
```
./build.sh --product-name rk3568 --build-target arkcompiler/runtime_core/tests/fuzztest:fuzztest
```

2."runtime_core/libpandabase/tests:host_unittest"
release模式：
```
./build.sh --product-name rk3568 --build-target runtime_core_host_unittest
```
debug模式：
```
./build.sh --product-name rk3568 --build-target runtime_core_host_unittest --gn-args is_debug=true
```

##### **一、Compile Target**
- [ ] 已通过
- [ ] 不涉及，无需验证


##### **二、Ark独立构建**
- [ ] 已通过
- [ ] 不涉及，无需验证

1. 编译(x64/arm64)
2. UT
3. test262
4. workload


##### **三、 Test262**
- [ ] 已通过
- [ ] 不涉及，无需验证

##### **四、 Runtime Core UT**
- [ ] 已通过
- [ ] 不涉及，无需验证

1. Host

2. Device


##### **五、Fuzz编译测试**
- [ ] 已通过
- [ ] 不涉及，无需验证


#####  **六、漏洞扫描UT**
- [ ] 已通过
- [ ] 不涉及，无需验证证


##### **七、Verifier测试**
- [ ] 已通过
- [ ] 不涉及，无需验证

1. UT
2. 测试脚本

##### **八、指令/abc格式修改自检，需联系下方邮箱，同步至相关领域**
**重要：涉及runtime_core仓abc2program、libpandafile、isa目录下的修改，必须提供一个编译helloworld项目的hap包给对应领域，并联系下方邮箱**
- [ ] 涉及，已同步
- [ ] 不涉及

**Email:** chenqiuyao@huawei.com
