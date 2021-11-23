# Docker Container

This repository comes with built-in Dockerfile to support docker
containers. This README serves as documentation.

## Dockerfile Specifications

The `Dockerfile` performs the following steps:

1. Obtain base image (phusion/baseimage:0.10.1)
2. Install required dependencies using `apt-get`
3. Add dxperts-core source code into container
4. Update git submodules
5. Perform `cmake` with build type `Release`
6. Run `make` and `make_install` (this will install binaries into `/usr/local/bin`
7. Purge source code off the container
8. Add a local dxperts user and set `$HOME` to `/var/lib/dxperts`
9. Make `/var/lib/dxperts` and `/etc/dxperts` a docker *volume*
10. Expose ports `8090` and `1776`
11. Add default config from `docker/default_config.ini` and
    `docker/default_logging.ini`
12. Add an entry point script
13. Run the entry point script by default

The entry point simplifies the use of parameters for the `blockproducer_node`
(which is run by default when spinning up the container).

### Supported Environmental Variables

* `$DXPERTSD_SEED_NODES`
* `$DXPERTSD_RPC_ENDPOINT`
* `$DXPERTSD_PLUGINS`
* `$DXPERTSD_REPLAY`
* `$DXPERTSD_RESYNC`
* `$DXPERTSD_P2P_ENDPOINT`
* `$DXPERTSD_BLOCKPRODUCER_ID`
* `$DXPERTSD_PRIVATE_KEY`
* `$DXPERTSD_TRACK_ACCOUNTS`
* `$DXPERTSD_PARTIAL_OPERATIONS`
* `$DXPERTSD_MAX_OPS_PER_ACCOUNT`
* `$DXPERTSD_ES_NODE_URL`
* `$DXPERTSD_TRUSTED_NODE`

### Default config

The default configuration is:

    p2p-endpoint = 0.0.0.0:1776
    rpc-endpoint = 0.0.0.0:8090
    bucket-size = [60,300,900,1800,3600,14400,86400]
    history-per-size = 1000
    max-ops-per-account = 100
    partial-operations = true

# Docker Compose

With docker compose, multiple nodes can be managed with a single
`docker-compose.yaml` file:

    version: '3'
    services:
     main:
      # Image to run
      image: dxperts/dxperts-core:latest
      # 
      volumes:
       - ./docker/conf/:/etc/dxperts/
      # Optional parameters
      environment:
       - DXPERTSD_ARGS=--help


    version: '3'
    services:
     fullnode:
      # Image to run
      image: dxperts/dxperts-core:latest
      environment:
      # Optional parameters
      environment:
       - DXPERTSD_ARGS=--help
      ports:
       - "0.0.0.0:8090:8090"
      volumes:
      - "dxperts-fullnode:/var/lib/dxperts"


# Docker Hub

This container is properly registered with docker hub under the name:

* [dxperts/dxperts-core](https://hub.docker.com/r/dxperts/dxperts-core/)

Going forward, every release tag as well as all pushes to `develop` and
`testnet` will be built into ready-to-run containers, there.

# Docker Compose

One can use docker compose to setup a trusted full node together with a
delayed node like this:

```
version: '3'
services:

 fullnode:
  image: dxperts/dxperts-core:latest
  ports:
   - "0.0.0.0:8090:8090"
  volumes:
  - "dxperts-fullnode:/var/lib/dxperts"

 delayed_node:
  image: dxperts/dxperts-core:latest
  environment:
   - 'DXPERTSD_PLUGINS=delayed_node blockproducer'
   - 'DXPERTSD_TRUSTED_NODE=ws://fullnode:8090'
  ports:
   - "0.0.0.0:8091:8090"
  volumes:
  - "dxperts-delayed_node:/var/lib/dxperts"
  links: 
  - fullnode

volumes:
 dxperts-fullnode:
```

```
mkdir dxperts-fullnode
cp ../genesis/genesis.json dxperts-fullnode/.
docker-compose up
sudo cp docker/default_config.ini dxperts-fullnode/config.ini 
docker-compose up
```


