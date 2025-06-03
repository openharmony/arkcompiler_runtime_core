### **Issue**
_Please add a link to an issue_

### **Reason**
_Please add reason of the changes_

### **Description**
_Please add detailed description_

### **Tests**
```
ninja all tests
```
**All required pre-merge tests passed. Results are available in the internal CI.**

### **[Test Items](https://gitee.com/openharmony/arkcompiler_runtime_core/wikis)**

##### **1. Runtime Core UT(Release/Debug)**
- [ ] Passed
- [ ] Not involved, no verification needed

1. Host

2. Device

##### **2. Verifier Test**
- [ ] Passed
- [ ] Not involved, no verification needed

##### **3. Fuzz Compilation Test**
- [ ] Passed
- [ ] Not involved, no verification needed

##### **4. 64-bit RK Compilation**
- [ ] Passed
- [ ] Not involved, no verification needed

##### **5. Compile Mac Platform SDK**
- [ ] Passed
- [ ] Not involved, no verification needed

##### **6. Command/abc format modification self-check, please contact the email below, synchronize to the relevant field**
**Important: For modifications involving the abc2program, libpandafile and isa directories in the runtime_core repository, you must provide a compiled hap package for the helloworld project to the corresponding domain and contact the email address below.**
- [ ] Involved, already synchronized
- [ ] Not involved

**Email:** wutao185@huawei.com

##### **7. Panda_Guard version number change self-check**
- [ ] Involved, **Please contact email:** wutao185@huawei.com
- [ ] Not involved

##### **8. Compatibility Testing (for instruction generation or file format changes)**
1. Minor Version Compatibility Test <!-- If the change causes new ABC files to fail on older images, a new version number must be added -->
- [ ] Version number added
- [ ] Passed
- [ ] Not applicable, no need to verify

2. Major Version Compatibility Test <!-- When `target-api-version` is configured, the generated ABC must run on the corresponding version -->
- [ ] Passed
- [ ] Not applicable, no need to verify

3. es2abc Version Compatibility Test <!-- ABC files compiled by the new es2abc from an old API version should still be recognized and processed by the old es2abc -->
**Note: If the PR involves changes to version control test cases, you must also check its impact on other branches and confirm whether updates are needed in those branches.**
- [ ] Affected — Changes impact API/bytecode version and need to be synced to other branches
- [ ] Affected — Changes impact API/bytecode version but do not need to be synced to other branches
- [ ] Not affected — Changes do not involve API/bytecode version