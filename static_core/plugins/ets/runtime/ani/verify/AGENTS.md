# ANI Verification Module

## Introduction
The ANI Verification Module provides a security layer for the Ark Native Interface (ANI). It is designed to protect the ArkTS VM from malfunctions or crashes caused by incorrect usage of the ANI API by native code. 

Key features include:
- **Transparent Switching**: The interfaces in verify mode match those in release mode one-to-one. The runtime switches between modes by replacing the `c_api` function pointer table within the `__ani_vm` and `__ani_env` structures.
- **Validation**: When enabled, it validates argument types, monitors reference life-cycles, and ensures that method/field handles are valid before they are used.

## Activation
Verification mode is not enabled by default. It is activated by passing the `--verify:ani` option during VM creation. Since creating a VM typically requires multiple configurations (like boot files), it is recommended to manage options using a container like `std::vector`.

### Example: Activating with Multiple Options
```cpp
std::vector<ani_option> options;
options.push_back({"--verify:ani", nullptr});
// Add other options as needed
ani_options optionsPtr = {options.size(), options.data()};
ani_vm *vm = nullptr;
if (ANI_CreateVM(&optionsPtr, ANI_VERSION_1, &vm) != ANI_OK) {
    // Handle initialization error
}
```

## Directory Structure
- `types/`: Contains wrappers and descriptors for verified entities (methods, fields, references, etc.).
  - `venv.h/cpp`: Verified environment wrapper.
  - `vmethod.h/cpp`, `vfield.h/cpp`: Verified method and field handles.
  - `vref.h/cpp`: Verified reference tracking.
  - `vvm.h`: Verified VM wrapper.
  - `vresolver.h`: Verified resolver handle.
  - `internal_ref.h/cpp`: Internal reference representation for verification.
- `ani_verifier.h/cpp`: The main verifier engine that manages global verification data and state.
- `env_ani_verifier.h/cpp`: Logic for verifying ANI environment states.
- `verify_ani_checker.h/cpp`: Implementation of specific verification checks.
- `verify_ani_interaction_api.h/cpp`: The verified version of the interaction API (Object, Class, Method operations).
- `verify_ani_vm_api.h/cpp`: The verified version of the VM API.
- `verify_ani_cast_api.h`: Utilities for casting between standard ANI types and their verified counterparts.

## Testing Commands
Testing the verification module uses the same commands as the main ANI module. Ensure you are in the project's build output directory (e.g., `static_core/out`).

### Full Test Suite
To run all ANI-related tests (including those that exercise verification):
```bash
ninja ani_tests
```

### Specific Test Case
To run a specific test case:
```bash
ninja ani_test_example_calls_gtests
```

## Typical Development Guidelines
- **Adding Checks**: New verification logic should be integrated into `verify_ani_checker.cpp`.
- **API Consistency**: APIs in verify and release versions must **strictly match**. If a new API is added, it must be implemented in both release and verify locations (e.g., `verify_ani_interaction_api.cpp` and `ani_interaction_api.cpp`) and registered in their respective dispatch tables.
