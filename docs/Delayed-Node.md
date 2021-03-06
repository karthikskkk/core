The `blockproducer_node` program now can be used to serve the features that was served by `delayed_node`.

## Configurations

To use `blockproducer_node` as a delayed node, some options are required to be configured appropriately.

### 1. Disable P2P connection

Start `blockproducer_node` with command line option `--seed-nodes="[]"` and/or `--p2p-endpoint=127.0.0.1:0`, or configure the options in `config.ini`:

```
p2p-endpoint = 127.0.0.1:0
seed-nodes = []
```

### 2. Enable delayed_node plugin

Start `blockproducer_node` with `--plugins="delayed_node [and other required plugins]"`, or configure it in `config.ini`:

```
plugins = delayed_node
```

### 3. Fetch blocks from a trusted blockproducer node via RPC

Start `blockproducer_node` with `--trusted-node="ip.address.of.the.blockproducer.node:rpc-port"`, or configure it in `config.ini`:

```
trusted-node = ip.address.of.the.blockproducer.node:rpc-port
```

## Sample Configurations

Assuming the RPC endpoint of the trusted node is `127.0.0.1:8090`, we can have following options in `config.ini` of a delayed node.

```
p2p-endpoint = 127.0.0.1:0
seed-nodes = []
plugins = delayed_node account_history api_helper_indexes
trusted-node = 127.0.0.1:8090
```
