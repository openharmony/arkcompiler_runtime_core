---
name: plan-feature
description: Plan implementation of new features with architecture exploration and design
---

Enters Plan mode to thoroughly explore the codebase and design implementation approach before coding.

## Usage

```bash
/plan-feature [feature description]
```

## Examples

### Add New Compiler Option
```bash
/plan-feature add a new compiler option for optimization level to the options panel
```

### Add New Output View
```bash
/plan-feature implement a new IR viewer tab in the output panel
```

### Refactor Redux State
```bash
/plan-feature refactor the logs slice to support multiple log streams
```

### Add API Endpoint
```bash
/plan-feature add a new backend endpoint for bytecode verification
```

### Implement Feature Flag
```bash
/plan-feature add feature flag system for experimental compiler features
```

## What It Does

1. **Exploration Phase**
   - Analyzes existing codebase patterns
   - Finds similar features to reference
   - Identifies all files that need changes
   - Understands data flow and architecture

2. **Design Phase**
   - Proposes implementation approach
   - Identifies potential issues/trade-offs
   - Considers impact on existing features
   - Asks clarifying questions if needed

3. **Planning Phase**
   - Creates step-by-step implementation plan
   - Lists all files to modify/create
   - Defines acceptance criteria
   - Presents plan for your approval

4. **Execution**
   - Waits for your approval
   - Implements according to plan
   - Tests the implementation

## When to Use

Use Plan mode for:
- ✅ New features affecting multiple files
- ✅ Architectural changes
- ✅ Redux slice modifications
- ✅ API endpoint additions
- ✅ Component refactoring
- ✅ When multiple valid approaches exist

**Skip Plan mode for:**
- ❌ Simple bug fixes (1-2 lines)
- ❌ Adding a single function
- ❌ Typo corrections
- ❌ Obvious tweaks

## Best Practices

1. **Be specific** about what you want
2. **Review the plan** carefully before approval
3. **Ask questions** if anything is unclear
4. **Consider trade-offs** in the approach
5. **Test after implementation**

## Example Output

After planning, you'll see:
```
## Implementation Plan: Add IR Viewer

### Files to Modify
- frontend/src/pages/irViewer/irViewer.tsx (create)
- frontend/src/store/slices/outputSlice.ts (modify)
- frontend/src/components/mosaic/mosaic.tsx (update)
- backend/src/arkts_playground/routers/compile_run.py (add endpoint)

### Approach
1. Add new tab to mosaic layout
2. Create IR slice to manage IR data state
3. Add /ir endpoint to backend
4. Wire up IR viewer component

### Trade-offs
- Creating separate slice vs. adding to existing outputSlice
- Using Monaco vs. custom viewer for IR display

Ready to proceed? [Approve/Modify/Decline]
```

## Related Skills

- `/add-endpoint` - For adding single API endpoints
- `/explore` - Use exploration to inform feature planning
- `/playground-test` - Plan tests as part of feature implementation
