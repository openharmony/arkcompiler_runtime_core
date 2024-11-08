## Installation

* Install `python3 >=3.10`
* Install `arkts-playground` package

`pip install arkts_playground-0.0.1-py3-none-any.whl`

## Running

* Create `config.yaml` following the [sample](./config.yaml.sample)
* Run server

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
