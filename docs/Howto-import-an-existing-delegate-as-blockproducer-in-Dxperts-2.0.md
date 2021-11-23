## Preparations in Dxperts 0.9 network

We need to Extract the signing public and private key from Dxperts 0.9.

Let's obtain the `<publickey>`:

    >>> get_account <delegatename>
    [...]
    Block Signing Key: <publickey>
    [...]

**Remark**: Public keys in the Dxperts network have the prefix `DXP`. Hence, in the case of the Graphene testnet you should replace `DXP` by `GPH`.

and the corresponding `<wifkey>`:

    >>> wallet_dump_account_private_key <delegatename> signing_key
    "<wifkey>"

## Dxperts 2.0 network (or Graphene testnet)

### Download the genesis block (only for testnet)

For the testnet we need to download the proper genesis block. Eventually, the
genesis block will be part of the client so that this step will not be required
for the real network later. The genesis block can be downloaded (here)[https://drive.google.com/open?id=0B_GVo0GoC_v_S3lPOWlUbFJFWTQ].

### Run the blockproducer as a node in the network

We first run the blockproducer node without block production and connect it to the P2P
network with the following command:

    $ programs/blockproducer_node/blockproducer_node -s 104.200.28.117:61705 --rpc-endpoint 127.0.0.1:8090 --genesis-json aug-14-test-genesis.json

The address `104.200.28.117` is one of the public seed nodes.

### Retreiving blockproducer_id

We now open up the `cli_wallet` and connect to our plain and stupid blockproducer node:

    $ programs/cli_wallet/cli_wallet -s ws://127.0.0.1:8090

The blockproducer_id can be obtain from the blockchain:

    locked >>> get_blockproducer <delegatename>

where `<delegatename` is the name of the account used as delegate in
Dxperts0.9. This delegate is a "blockproducer" in Dxperts 2.0.

### Running a block producing blockproducer

Now we need to start the blockproducer, so shut down the wallet (ctrl-d), and shut
down the blockproducer (ctrl-c). Re-launch the blockproducer, now mentioning the new
blockproducer 1.6.10 and its keypair:

    ./blockproducer_node --rpc-endpoint=127.0.0.1:8090 \
                   --blockproducer-id '"<blockproducerid>"' \
                   --private-key '["<publickey>", "<wifkey>"]' \
                   --genesis-json aug-14-test-genesis.json \
                   -s 104.200.28.117:61705

Alternatively, you can also add this line into yout config.ini:

    blockproducer-id = "<blockproducerid>"
    private-key = ["<publickey>", "<wifkey>"]

If you monitor the output of the `blockproducer_node`, you should see it generate
blocks signed by your blockproducer:

    Blockproducer 1.6.10 production slot has arrived; generating a block now...
    Generated block #367 with timestamp 2015-07-05T20:46:30 at time 2015-07-05T20:46:30
