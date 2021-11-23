We need a client to connect to. Either we run a blockproducer in (monitor mode) or use
a public trusted blockproducer. For exchanges, it is though recommended to run a
full node. We can connect to the network via a seed node:

    programs/blockproducer_node/blockproducer_node -s 104.200.28.117:61705 --rpc-endpoint 127.0.0.1:8090  # FIXME?

This opens up a node that we can connect to via the inluded wallet

    programs/cli_wallet/cli_wallet -s ws://127.0.0.1:8090 -H 127.0.0.1:8091

which will open port `8091` for HTTP-RPC requests _and_ has the capabilities to
handle accounts while the blockproducer_node can only answer queries to the
blockchain.
