---
name: explore
description: Explore the ArkTS Playground codebase to understand architecture, find components, or trace data flow
---

Uses Explore agent to investigate the playground codebase structure, components, and relationships.

## Usage

```bash
/explore [query]
```

## Examples

### Find Redux State Management
```bash
/explore how does compilation result flow through Redux slices
```

### Locate Components
```bash
/explore where are the Monaco editor components and how are they configured
```

### Understand Data Flow
```bash
/explore trace the flow from compile button click to backend API response
```

### Feature Investigation
```bash
/explore how does the AST viewer component work
```

### Find All Related Files
```bash
/explore find all files related to syntax highlighting for ArkTS
```

## What It Does

1. Searches the codebase using multiple strategies:
   - File pattern matching (Glob)
   - Content searching (Grep)
   - File reading and analysis

2. Provides thorough exploration at specified level:
   - **quick**: Basic file locations and structure
   - **medium**: Moderate exploration (default, recommended)
   - **very thorough**: Comprehensive analysis across multiple locations

3. Returns structured findings:
   - File paths with line numbers
   - Code snippets showing relevant implementations
   - Relationships between components/modules
   - Architecture patterns used

## Best Practices

- Be specific in your query for better results
- Use "medium" thoroughness for most cases
- Great for onboarding to the 9 Redux slices architecture
- Helps understand the frontend → backend API integration
- Use before making changes to unfamiliar code

## Tips

- Start with broad queries, then narrow down
- Review the file paths and line numbers returned
- Follow the code snippets to understand patterns
- Combine with `/plan-feature` for implementation

## Related Skills

- `/plan-feature` - Use exploration to plan new features
- `/add-endpoint` - Explore existing endpoints before adding new ones
- `/playground-test` - Understand test structure before running tests
