---
name: docker
description: Manage Docker containers and images for the ArkTS Playground
---

Uses the Bash agent to handle Docker operations for the playground frontend and backend services.

## Usage

```bash
/docker [command] [service]
```

## Commands

### Build Images
```bash
# Build frontend image
docker build frontend

# Build backend image (requires Panda runtime built first)
docker build backend

# Build both images
docker build all
```

### Run Containers
```bash
# Run frontend container (port 3000)
docker run frontend

# Run backend container (port 8000)
docker run backend

# Run both containers
docker run all
```

### Container Management
```bash
# List running playground containers
docker ps

# Stop all playground containers
docker stop all

# Remove all playground containers
docker rm all

# View logs
docker logs [frontend|backend]
```

### Cleanup
```bash
# Remove all playground images and containers
docker clean

# Remove dangling build artifacts
docker prune
```

## What It Does

### Frontend Docker Operations
- Builds React SPA production bundle
- Creates optimized Docker image with nginx
- Exposes port 3000 (or custom port)
- Handles static asset serving

### Backend Docker Operations
- Builds Python FastAPI service
- Installs arkts-playground package
- Configures entry point for `arkts-playground` command
- Exposes port 8000 for API endpoints
- Mounts config.yaml from host

## Prerequisites

### Backend Build Requirements
Before building backend Docker image, you must:
1. Build the Panda runtime from `static_core/` root:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --target returntype
   ```
2. Ensure compiler binaries exist at `../../../../build/returntype`

### Frontend Build
No prerequisites - builds standalone React bundle

## Common Workflows

### Development Setup
```bash
/docker build all
/docker run all
```
Access at:
- Frontend: http://localhost:3000
- Backend: http://localhost:8000
- API Docs: http://localhost:8000/docs

### Production Deployment
```bash
# Build optimized images
docker build frontend --production
docker build backend --production

# Run with custom ports
docker run frontend -p 8080:80
docker run backend -p 9000:8000
```

### Troubleshooting
```bash
# Check container status
docker ps

# View backend logs
docker logs backend

# Enter container for debugging
docker exec backend /bin/bash

# Rebuild after code changes
docker build backend --no-cache
```

## Best Practices

- Test production builds locally before deploying
- Use multi-stage builds for smaller images
- Tag images with version numbers for traceability
- Use volume mounts for configuration files
- Check container health endpoints
- Frontend serves static files, backend serves API only

- Backend build requires **Panda runtime** to be built first
- Frontend builds are faster (no external dependencies)
- Use `--no-cache` when troubleshooting build issues
- Mount config.yaml as volume for backend configuration
- Check health endpoint: `curl http://localhost:8000/health`

## Tips

- Use `docker ps` to check container status
- Use `docker logs` to view container output
- Use `docker exec -it <container> /bin/bash` to enter container for debugging
- Use `--no-cache` when troubleshooting build issues
- Stop containers with `docker stop` before rebuilding

## Docker Compose (Alternative)

For multi-container orchestration, you can also use:
```bash
docker-compose up -d
```

This skill uses direct Docker commands for finer control and better error output.

## Related Skills

- `/fullstack` - Start development servers without Docker
- `/add-endpoint` - Endpoints may require Docker image rebuild
- `/playground-test` - Run tests before building Docker images
