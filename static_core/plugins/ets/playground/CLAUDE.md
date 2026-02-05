# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is the **ArkTS Playground** - a full-stack web application for compiling, executing, and analyzing ArkTS (TypeScript-based) code. It consists of:

- **Frontend**: React 18.3 + TypeScript SPA with Monaco Editor (frontend/)
- **Backend**: Python FastAPI service that wraps ArkTS compiler toolchain (backend/)

The playground allows developers to write ArkTS code, compile it, execute the bytecode, view AST/disassembly/IR dumps, and analyze logs. It's part of Huawei's Panda runtime ecosystem for HarmonyOS development.

**Tech Stack:**
- Frontend: React 18.3, TypeScript 4.9, Redux Toolkit, Monaco Editor, Blueprint.js, React Mosaic
- Backend: Python 3.10+, FastAPI, Pydantic, structlog, Prometheus metrics

## Repository Structure

```
playground/
├── frontend/          # React SPA (client-side)
│   ├── src/
│   │   ├── components/      # Reusable UI components
│   │   ├── pages/           # Main views (editor, AST, disasm, logs, etc.)
│   │   ├── store/           # Redux state management
│   │   │   ├── actions/     # Async thunks for API calls
│   │   │   ├── slices/      # Redux slices (9 slices)
│   │   │   └── selectors/   # Memoized selectors
│   │   ├── services/        # API layer (axios)
│   │   ├── models/          # TypeScript interfaces
│   │   └── utils/           # Helper functions
│   ├── public/
│   │   └── env.js           # Backend URL configuration
│   ├── package.json
│   └── Dockerfile
└── backend/           # Python FastAPI service
    ├── src/arkts_playground/
    │   ├── routers/         # API endpoints (compile_run, options, formatting)
    │   ├── models/          # Pydantic models
    │   ├── deps/            # Dependencies (runner wrapper)
    │   ├── config.py        # Configuration management
    │   ├── app.py           # FastAPI app factory
    │   ├── main.py          # Entry point
    │   ├── logs.py          # Structured logging
    │   └── metrics.py       # Prometheus metrics
    ├── tests/               # pytest test suite
    ├── config.yaml.sample   # Sample configuration
    ├── pyproject.toml       # Python package config
    ├── setup.py             # Package setup
    └── Dockerfile
```

## Common Commands

### Frontend Development

```bash
cd frontend

# Install dependencies
npm install

# Start dev server (http://localhost:3000)
npm start

# Run tests
npm test                  # Run once
npm run test:watch        # Watch mode with coverage

# Build for production
npm run build             # Outputs to build/
```

**Important**: If backend runs on a different port locally, edit `frontend/public/env.js`:
```js
window.runEnv = {
   apiUrl: 'http://localhost:8000/',  // Change backend URL here
};
```

### Backend Development

```bash
cd backend

# Install package (development mode)
pip install -e .

# Run server
arkts-playground -c /path/to/config.yaml

# Or directly with Python
python -m arkts_playground.main -c config.yaml
```

**Configuration**: Copy `config.yaml.sample` to `config.yaml` and adjust:
- `binary.build`: Path to ArkTS compiler binaries (default: `../../../../build/returntype`)
- `binary.icu_data`: Path to ICU data file
- `server.uvicorn_config`: Host/port (default: 0.0.0.0:8000)
- `server.cors_*`: CORS settings (pre-configured for local dev)
- `env`: Logging format (`dev` or `production`)

### Testing

**Frontend:**
- 31 Jest test files covering components, services, Redux logic
- Framework: Jest + React Testing Library
- Mocks: Blueprint.js and Monaco Editor mocked in `__mocks__/`

**Backend:**
```bash
cd backend

# Run all tests
pytest

# Run with coverage
pytest --cov=src --cov-report=term-missing

# Run specific test file
pytest tests/test_api.py

# Linting
pylint src/ tests/
```

### Docker

**Frontend:**
```bash
docker build -t playground-front frontend/
docker run -d -p 3000:80 --name playground-front playground-front
```

**Backend:**
```bash
# Build Panda first, then from static_core:
docker build -t arkts-playground -f ./plugins/ets/playground/backend/Dockerfile .
docker run -d --name arkts-playground -p 8000:8000 arkts-playground
```

## Architecture

### Frontend Architecture

**State Management (Redux Toolkit):**
- 9 Redux slices managing different concerns:
  - `appState` - UI state (disasm toggle, AST view mode, verifier mode)
  - `code` - Source code, compilation/execution results, loading states
  - `logs` - Compilation/runtime logs with filtering
  - `options` - Compiler options (TS compiler flags)
  - `syntax` - Monaco syntax highlighting rules for ArkTS
  - `ast` - AST data and loading state
  - `features` - Feature flags
  - `notifications` - Toast notifications

**Data Flow:**
1. User action → Component event handler
2. Component dispatches Redux thunk action
3. Thunk calls service API (services/*.ts via axios)
4. Service calls backend endpoint
5. Thunk dispatches success/failure actions to update slice state
6. Component re-renders from Redux state via selectors

**Key Components:**
- **Monaco Editor** (src/pages/codeEditor/) - Code editing with ArkTS syntax
- **Mosaic Layout** (src/components/mosaic/) - Resizable split-panel system
- **Control Panel** (src/components/controlPanel/) - Run/compile buttons

**Code Sharing:**
URL parameters support sharing code snippets. Code is extracted from URL on initialization.

### Backend Architecture

**FastAPI Application Factory:**
`app.create_app()` returns configured FastAPI instance with:
- CORS middleware (configurable)
- Request ID correlation (asgi-correlation-id)
- Structured logging (structlog) with context binding
- Prometheus metrics (/metrics endpoint)
- Custom middleware for request logging and timing

**Routers:**
- `compile_run` - POST /compile, /run, /verify, /disasm, /ast
- `options` - GET /options, /versions, /features
- `formatting` - GET /syntax (Monaco tokenizer rules)

**Runner Integration:**
The `deps/runner.py` module wraps native binaries:
- `ark` - ArkTS compiler
- `es2panda` - TS to ArkTS compiler
- `disasm` - Disassembler
- `verifier` - Bytecode verifier
- `ark_aot` - AOT compiler

Binaries are invoked as subprocesses with timeout, output captured, and Prometheus metrics tracked for failures.

**Configuration Management:**
- Pydantic settings from YAML file
- Cached with `functools.lru_cache`
- Supports CLI override via `--config` flag
- Models: `PlaygroundSettings`, `Server`, `Binary`, `SyntaxModel`, `Features`

**Logging:**
- `dev` mode: Human-readable logs for debugging
- `production` mode: Structured JSON logs for observability
- Request ID tracking across async context
- Access logs with duration tracking

**Metrics:**
- Prometheus instrumentation on all endpoints
- Custom counters for compiler/disasm/runtime failures
- Excluded paths: /metrics, /syntax, /versions, /features, /health

### API Communication

Frontend → Backend endpoints:
- `POST /compile` - Compile ArkTS code to bytecode
- `POST /run` - Execute compiled bytecode
- `POST /disasm` - Disassemble bytecode
- `POST /ast` - Get AST of source code
- `GET /options` - Get available compiler options
- `GET /syntax` - Get Monaco syntax highlighting rules
- `GET /versions` - Get ArkTS/Playground/es2panda versions
- `GET /features` - Get feature flags

## Important Notes

### Claude Skills System
Custom skills are defined in `SKILLS/` directory using a folder-per-skill structure:

```
SKILLS/
├── add-endpoint/
│   └── add-endpoint.md
├── copyright/
│   └── copyright.md
├── playground_test/
│   └── playground_test.md
└── ...
```

Each skill folder contains `<skill-name>.md` with YAML front matter. Available skills:
- `/add-endpoint` - Add new FastAPI endpoints
- `/copyright` - Update copyright years in changed files
- `/docker` - Manage Docker containers
- `/explore` - Explore codebase architecture
- `/fullstack` - Start development servers
- `/plan-feature` - Plan new features
- `/playground-test` - Run tests (frontend/backend)
- `/shellcheck` - Validate bash scripts

See `SKILLS/README.md` for detailed skill documentation and usage.

Skills are configured in `.claude/settings.json`:
```json
{
  "skills": {
    "directory": "SKILLS",
    "structure": "folder-per-skill",
    "naming": "match-folder"
  }
}
```

### Licensing
All files include Apache 2.0 license headers with copyright to Huawei Device Co., Ltd. (2024-2026). When creating new files, include:
```python
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
```
Copyright format varies by file type:
- Shell/Python/YAML/TOML: `# Copyright (c) YYYY Huawei Device Co., Ltd.`
- TypeScript/JavaScript/SCSS: ` * Copyright (c) YYYY Huawei Device Co., Ltd.`
- Markdown: `<!-- Copyright (c) YYYY Huawei Device Co., Ltd. -->` or `# Copyright...`
Use `/copyright` skill to update years across files
Use `/shellcheck` skill to validate bash scripts for security and portability issues

### Build Dependencies
Backend requires ArkTS compiler binaries to be built first:
```bash
# Build Panda runtime from static_core root
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target returntype
```

### Environment Integration
The playground is part of the larger Panda runtime ecosystem:
- Located at: `runtime_core/static_core/plugins/ets/playground`
- Depends on: `runtime_core/static_core/plugins/ets/` compiler toolchain
- Binary paths in config.yaml are relative to playground backend

### Frontend Build System
- Uses Create React App (react-scripts 5.0) with custom webpack config for SCSS
- TypeScript strict mode enabled
- Production builds optimized for modern browsers (ES2020)

### Backend Packaging
- Python package built with `python3 -m build`
- Version managed by setuptools_scm from git tags
- Installed as console script: `arkts-playground`
