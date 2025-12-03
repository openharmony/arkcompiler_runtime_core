## Installation

* Install `python3 >=3.10`
* Install `arkts-playground` package

`pip install arkts_playground-0.0.1-py3-none-any.whl`

## Running

### Create `config.yaml`
Use `config.yaml.sample` as a starting point.

The build directory might be different in your environment, but otherwise by default it's set up for running locally out of the box.

### CORS
This is needed if the backend and frontend are on different ports, as is the case when developing locally outside of Docker.

This is already pre-configured in `config.yaml.sample`.

```yaml
# in config.yaml
server:
  cors_allow_origins: ["*"]
  cors_allow_methods: ["*"]
  cors_allow_headers: ["*"]
  cors_allow_credentials: true
  cors: true
```

### Run server

`arkts-playground -c /path/to/config.yaml`


## Docker
* Build panda
* Build pip package 
   * `cd /path/runtime_core/static_core/plugins/ets/playground/backend`
   * `python3 -m build`
* Build docker image
   * `cd /path/runtimne_core/static_core`
   * `docker build -t "your-tag" -f  ./plugins/ets/playground/backend/Dockerfile .`
* Run docker
   * `docker run -d --name  arkts-playground -p 8000:8000 your-tag`
