---
name: add-endpoint
description: Add a new backend API endpoint with full Pydantic models, routing, and pytest tests
---

Automates the complete workflow of adding a new FastAPI endpoint to the ArkTS Playground backend.

## Usage

```bash
/add-endpoint [endpoint description]
```

## Examples

### Simple Endpoint
```bash
/add-endpoint GET /stats that returns compilation statistics
```

### Complex Endpoint with Request Body
```bash
/add-endpoint POST /optimize that accepts bytecode and optimization options
```

### Endpoint with Multiple Operations
```bash
/add-endpoint endpoint for bytecode verification with GET and POST methods
```

## What It Does

### 1. Planning Phase
- Explores existing router patterns in `backend/src/arkts_playground/routers/`
- Identifies appropriate router (compile_run, options, formatting, or new)
- Analyzes similar endpoints for consistency
- Checks Pydantic models in `backend/src/arkts_playground/models/`
- Reviews test patterns in `backend/tests/`

### 2. Implementation Phase
Creates or modifies:

**Pydantic Models** (`backend/src/arkts_playground/models/`)
- Request models (if endpoint accepts data)
- Response models (for structured responses)
- Validation rules and examples
- Docstrings with OpenAPI documentation

**Router** (`backend/src/arkts_playground/routers/`)
- New route handler function
- Request validation
- Error handling (HTTP exceptions)
- Response formatting
- OpenAPI spec decorators
- Prometheus metrics integration

**Tests** (`backend/tests/`)
- Unit test file following pytest conventions
- Happy path test cases
- Error condition tests
- Validation tests
- Edge case coverage
- Mock fixtures for external dependencies

### 3. Integration
- Registers route in `backend/src/arkts_playground/app.py`
- Adds Prometheus metrics (if applicable)
- Updates API documentation
- Ensures CORS configuration includes new endpoint

### 4. Verification
- Runs test suite to verify implementation
- Checks test coverage
- Validates OpenAPI docs at `/docs`
- Tests endpoint manually (if backend running)

## Project-Specific Patterns

### Existing Router Structure
```
backend/src/arkts_playground/routers/
├── compile_run.py  # POST /compile, /run, /verify, /disasm, /ast
├── options.py      # GET /options, /versions, /features
└── formatting.py   # GET /syntax
```

### Common Patterns

**Request/Response Models:**
```python
# backend/src/arkts_playground/models/requests.py
class MyRequest(BaseModel):
    code: str
    options: Dict[str, Any] = Field(default_factory=dict)

# backend/src/arkts_playground/models/responses.py
class MyResponse(BaseModel):
    status: str
    data: Optional[Dict[str, Any]] = None
    error: Optional[str] = None
```

**Router Pattern:**
```python
# backend/src/arkts_playground/routers/my_router.py
@router.post("/my-endpoint", response_model=MyResponse)
async def my_endpoint(
    request: MyRequest,
    runner_wrapper: RunnerWrapper = Depends(get_runner_wrapper),
) -> MyResponse:
    """Execute my operation."""
    try:
        result = await runner_wrapper.run_my_command(request.code)
        return MyResponse(status="success", data=result)
    except Exception as e:
        log.error("Failed to execute", error=str(e))
        raise HTTPException(status_code=500, detail=str(e))
```

**Test Pattern:**
```python
# backend/tests/test_my_endpoint.py
import pytest
from httpx import AsyncClient

@pytest.mark.asyncio
async def test_my_endpoint_success(async_client: AsyncClient):
    response = await async_client.post("/my-endpoint", json={"code": "test"})
    assert response.status_code == 200
    assert response.json()["status"] == "success"

@pytest.mark.asyncio
async def test_my_endpoint_validation_error(async_client: AsyncClient):
    response = await async_client.post("/my-endpoint", json={})
    assert response.status_code == 422
```

## Best Practices

### Endpoint Design
- Use appropriate HTTP methods (GET for retrieval, POST for actions)
- Return structured JSON responses
- Include `status` field in responses
- Use `Optional` fields for conditional data
- Add comprehensive error messages

### Error Handling
- Validate input with Pydantic
- Use `HTTPException` for errors
- Log errors with structlog
- Return appropriate status codes (400, 422, 500)
- Include error details in response

### Testing
- Aim for >80% code coverage
- Test both success and failure paths
- Mock external dependencies (runner binaries)
- Use pytest fixtures for common setup
- Test validation logic thoroughly

### Documentation
- Add docstrings to endpoint functions
- Include Field descriptions in Pydantic models
- Provide examples in model schemas
- Update OpenAPI docs automatically visible at `/docs`

## When to Use

✅ **Use this skill for:**
- Adding new API endpoints
- Creating new router modules
- Adding request/response models
- Writing endpoint tests
- Implementing new compiler/disasm features

❌ **Don't use for:**
- Frontend changes (use `/plan-feature` instead)
- Simple config changes
- Modifying existing tests (use `/playground-test`)

## Example Output

After completion, you'll have:
```
✓ Created: backend/src/arkts_playground/models/my_endpoint.py
  - MyEndpointRequest
  - MyEndpointResponse

✓ Created: backend/src/arkts_playground/routers/my_router.py
  - POST /my-endpoint
  - Integrated with runner wrapper
  - Error handling and logging

✓ Created: backend/tests/test_my_endpoint.py
  - 8 test cases
  - 100% coverage of new endpoint

✓ Updated: backend/src/arkts_playground/app.py
  - Registered new router

✓ All tests passing (12/12)

Access endpoint at: http://localhost:8000/my-endpoint
Docs at: http://localhost:8000/docs
```

## Related Skills

- `/plan-feature` - For complex feature planning affecting multiple files
- `/playground-test` - To run the test suite after endpoint creation
- `/explore` - To understand existing endpoint patterns
- `/docker` - To rebuild backend Docker image with new endpoint
