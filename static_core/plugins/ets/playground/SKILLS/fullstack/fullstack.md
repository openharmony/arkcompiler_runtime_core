---
name: fullstack
description: Start both backend and frontend development servers
---

Starts the ArkTS Playground full-stack development environment with both backend and frontend servers running.

## Usage

```bash
/fullstack
```

## Examples

### Quick Start
```bash
/fullstack
```

### Start with Specific Backend Config
```bash
/fullstack backend/config-dev.yaml
```

## What It Does

1. **Prerequisites Check**
   - Verifies Python 3.10+ is installed
   - Checks arkts-playground package is installed
   - Validates Node.js exists
   - Confirms config.yaml is present
   - Ensures frontend dependencies are installed

2. **Backend Server** (Port 8000)
   - Starts FastAPI server using `backend/config.yaml`
   - Waits up to 30 seconds for backend to be ready
   - Logs to `/tmp/playground-backend.log`
   - Endpoints:
     - API: http://localhost:8000
     - Docs: http://localhost:8000/docs
     - Metrics: http://localhost:8000/metrics

3. **Frontend Server** (Port 3000)
   - Starts React dev server with hot reload
   - Logs to `/tmp/playground-frontend.log`
   - Automatically connects to backend at http://localhost:8000

4. **Cleanup**
   - Press `Ctrl+C` to stop both servers gracefully
   - Both processes are cleaned up automatically

## Manual Setup (if needed)

### Backend
```bash
cd backend
arkts-playground -c config.yaml
# Or: python -m arkts_playground.main -c config.yaml
```

### Frontend
```bash
cd frontend
npm start
```

## Best Practices

- Use this skill for local development (not for production)
- Check port availability if servers fail to start
- Review logs if startup fails
- Keep backend running when working on frontend (faster iteration)
- Restart backend after Python code changes

## Tips

- Backend requires manual restart after code changes
- Frontend auto-reloads on file changes
- Check `/tmp/playground-*.log` for detailed logs
- Use `lsof -i :8000` to check if port 8000 is in use
- Use `lsof -i :3000` to check if port 3000 is in use

## Troubleshooting

### Port Already in Use
```bash
# Check what's using port 8000
lsof -i :8000

# Kill the process
kill -9 <PID>
```

### Backend Won't Start
- Check logs: `tail -f /tmp/playground-backend.log`
- Verify config.yaml: `cat backend/config.yaml`
- Reinstall package: `cd backend && pip install -e .`

### Frontend Won't Start
- Check logs: `tail -f /tmp/playground-frontend.log`
- Reinstall dependencies: `cd frontend && npm install`

### Connection Issues
- Verify backend is running: `curl http://localhost:8000/versions`
- Check CORS settings in `backend/config.yaml`
- Verify backend URL in `frontend/public/env.js`

## Development Workflow

1. Make changes to frontend or backend code
2. Frontend auto-reloads on file changes
3. Backend requires manual restart (`Ctrl+C` then restart)
4. Open browser to http://localhost:3000
5. Check API docs at http://localhost:8000/docs

## Related Skills

- `/playground-test` - Run tests for frontend or backend
- `/docker` - Run services in Docker containers
- `/add-endpoint` - Add new API endpoints
- `/explore` - Explore codebase while developing
