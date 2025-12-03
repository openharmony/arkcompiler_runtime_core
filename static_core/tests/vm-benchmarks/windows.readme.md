
# Starting VMB on Windows host (Experimental)

### Build and Install Python Module

```shell
# 1) build package or download it from CI builds and skip to step 2
# inside this source directory
py -m pip install build
py -m build
py -m pip uninstall -y vmb  # optionally
# 2) install
py -m pip install .\dist\vmb-*-py3-none-any.whl
# 3) check if it works
vmb version
```

### Usage
```shell
# Run Node ts test
vmb all -p node_host [-v debug]  .\examples\benchmarks\ts\VoidBench.ts

# Run ArkJs test on Windows host
vmb all -p ark_js_vm_host [-v debug] \

# Run ArkJS on OHOS device

# (optionally) Set env var pointing to hdc binary if it is not in the PATH
$Env:HDC = "d:\hu\STUBS\hdc.bat"

vmb all -p ark_js_vm_ohos [-v debug] \
    [--device=DEVSERIAL] [--device-host=DEVICE_HOST] [--device-dir=DEVICE_DIR] \
    --es2abc-path=d:\hu\workload\1.1\tools\windows\es2abc.exe \
    --ark_js_vm-path=d:\hu\vm-exec-stub.bat .\examples\benchmarks\ts\VoidBench.ts
```
