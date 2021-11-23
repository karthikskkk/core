## Setting up private testnet

## Genesis creation

First, create a subdirectory named `genesis` and create a file within it
named `my-genesis.json`. This file dictates the initial state of the network.

You can use the existing developer `genesis.json` file as an example.
That file can be found at
https://gitlab.com/dxperts/dxperts-core/-/blob/main/libraries/egenesis/genesis-dev.json

The rest of these instructions assume you create your `my-genesis.json`
file in a subdirectory named `genesis`.

## Genesis editing

If you want to customize the network's initial state, edit `my-genesis.json`.
This allows you to control things such as:

- The accounts that exist at genesis, their names and public keys
- Assets and their initial distribution (including core asset)
- The initial values of chain parameters
- The account / signing keys of the `init` blockproducers (or in fact any
  account at all).

The chain ID is a hash of the genesis state. All transaction signatures
are only valid for a single chain ID. So editing the genesis file will
change your chain ID, and make you unable to sync with all existing
chains (unless one of them has exactly the same genesis file you do).

For testing purposes, the `--dbg-init-key` option will allow you to
quickly create a new chain against any genesis file, by replacing the
blockproducers' block production keys.

## Embedding genesis (optional)

Once you have `genesis.json`, you may set a `cmake` variable like so:

    cmake -DGRAPHENE_EGENESIS_JSON="$(pwd)/genesis/my-genesis.json"

and then rebuild. Note, sometimes I've had to clean the build and
CMake cache variables in order for `GRAPHENE_EGENESIS_JSON` to take
effect:

    make clean
    find . -name "CMakeCache.txt" | xargs rm -f
    find . -name "CMakeFiles" | xargs rm -Rf
    cmake -DGRAPHENE_EGENESIS_JSON="$(pwd)/genesis/my-genesis.json" .

Deleting caches will reset all `cmake` variables, so if you have used
instructions like [build-ubuntu](build-ubuntu.md) which tells you to
set other `cmake` variables, you will have to add those variables
to the `cmake` line above.

Embedding the genesis copies the entire content of `genesis.json`
into the `blockproducer_node` binary, and additionally copies the chain ID
into the `cli_wallet` binary. Embedded genesis allows the following
simplifications to the subsequent instructions:

- You need not specify the `genesis.json` file on the blockproducer node
  command line, or in the blockproducer node configuration file.

- You need not specify the chain ID on the `cli_wallet` command line
  when starting a new wallet.

Embedded genesis is a feature designed to make life easier for
consumers of pre-compiled binaries, in exchange for slight, optional
complication of the process for producing binaries.

## Creating data directory

We will a new data directory for our blockproducer as follows:

    programs/blockproducer_node/blockproducer_node --data-dir data/my-blockprod --genesis-json genesis/my-genesis.json --seed-nodes "[]"

The `data/my-blockprod` directory does not exist, it will be created
by the blockproducer node.

Several messages will go to the console. When you see messages like these:

    3501235ms th_a       main.cpp:165                  main                 ] Started blockproducer node on a chain with 0 blocks.
    3501235ms th_a       main.cpp:166                  main                 ] Chain ID is cf307110d029cb882d126bf0488dc4864772f68d9888d86b458d16e6c36aa74b

the initialization is complete, and you can press Ctrl+C to quit the blockproducer node.
(Note: Initialization will complete nearly instantaneously with the tiny
example genesis, unless you added a ton of balances.)

The reason for running the blockproducer node: It tells us the chain ID,
and it initializes the `data/my-blockprod` directory.

## Starting block production

Open `data/my-blockprod/config.ini` in your favorite text editor,
and set the following settings, uncommenting them if necessary:

    p2p-endpoint = 127.0.0.1:11010
    rpc-endpoint = 127.0.0.1:11011

    genesis-json = genesis/my-genesis.json

    private-key = ["GPH6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3"]

    required-participation = 0

    blockproducer-id = "1.6.1"
    blockproducer-id = "1.6.2"
    blockproducer-id = "1.6.3"
    blockproducer-id = "1.6.4"
    blockproducer-id = "1.6.5"
    blockproducer-id = "1.6.6"
    blockproducer-id = "1.6.7"
    blockproducer-id = "1.6.8"
    blockproducer-id = "1.6.9"
    blockproducer-id = "1.6.10"
    blockproducer-id = "1.6.11"

This authorizes the `blockproducer_node` to produce blocks on behalf of the
listed `blockproducer-id`'s, and specifies the private key needed to sign
those blocks. Normally each blockproducer would be on a different node, but
for the purposes of this testnet, we will start out with all blockproducers
signing blocks on a single node.

Now run `blockproducer_node` again:

    programs/blockproducer_node/blockproducer_node --data-dir data/my-blockprod --enable-stale-production --seed-nodes "[]"

Note that we need not specify `genesis.json` on the command line, since
we now specify it in the config file. The `--enable-stale-production`
flag tells the `blockproducer_node` to produce on a chain with zero blocks or
very old blocks. We specify the `--enable-stale-production` parameter
on the command line as we will not normally need it (although it can
also be specified in the config file).
The empty `--seed-nodes` is added to avoid connecting to the default seed nodes hardcoded for production.

Subsequent runs which connect to an existing blockproducer node over the p2p
network, or which get blockchain state from an existing data directory,
need not have the `--enable-stale-production` flag.

## Obtaining the chain ID

Each wallet is specifically associated with a single chain, specified
by its chain ID. This is to protect the user from e.g. unintentionally
using a testnet wallet on the real chain.

The chain ID is printed at blockproducer node startup. It can also be
obtained by using the API to query a running blockproducer node with the
`get_chain_properties` API call:

    curl --data '{"jsonrpc": "2.0", "method": "get_chain_properties", "params": [], "id": 1}' http://127.0.0.1:11011/rpc && echo

This `curl` command will return a short JSON object including the `chain_id`.

## Creating a wallet

In order to create a wallet, you must specify a chain ID and server.
With the blockproducer node's default access control settings, a blank
username and password will suffice:

    programs/cli_wallet/cli_wallet --wallet-file my-wallet.json --chain-id cf307110d029cb882d126bf0488dc4864772f68d9888d86b458d16e6c36aa74b --server-rpc-endpoint ws://127.0.0.1:11011 -u '' -p ''

Note, since the genesis timestamp will likely be different, your chain
ID will be different! Instead of `cf3071110...` you should use the
chain ID reported by your `blockproducer_node`. (See TODO:link for
instructions.)

Before continuing, we should set a password. This password is used
to encrypt the private keys in the wallet. We will use the word
`supersecret` in this example.

    set_password supersecret

## Gaining access to stake

In Graphene, balances are contained in accounts. To claim an account
that exists in the Graphene genesis, use the `import_key` command:

    unlock supersecret
    import_key matrix "5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3"

Funds are stored in genesis balance objects. These funds can be
claimed, with no fee, using the `import_balance` command.

    import_balance matrix ["5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3"] true

## Creating accounts

Creating an account requires lifetime member (LTM) status. To upgrade
to LTM, use the `upgrade_account` command:

    upgrade_account matrix true

We can now register an account. The `register_account` command
allows you to register an account using only a public key:

    register_account alpha DXP4zSJHx7D84T1j6HQ7keXWdtabBBWJxvfJw72XmEyqmgdoo1njF DXP4zSJHx7D84T1j6HQ7keXWdtabBBWJxvfJw72XmEyqmgdoo1njF matrix matrix 0 true
    transfer matrix alpha 100000 DXP "here is the cash" true

We can now open a new wallet for `alpha` user:

    import_key alpha 5HuCDiMeESd86xrRvTbexLjkVg2BEoKrb7BAA5RLgXizkgV3shs
    upgrade_account alpha true
    create_blockproducer alpha "http://www.alpha" true

The `get_private_key` command allows us to obtain the public key corresponding to the block signing key:

    get_private_key GPH6viEhYCQr8xKP3Vj8wfHh6WfZeJK7H9uhLPDYWLGCRSj5kHQZM

## Creating dxpcore members

    create_account_with_brain_key com0 com0 matrix matrix true
    create_account_with_brain_key com1 com1 matrix matrix true
    create_account_with_brain_key com2 com2 matrix matrix true
    create_account_with_brain_key com3 com3 matrix matrix true
    create_account_with_brain_key com4 com4 matrix matrix true
    create_account_with_brain_key com5 com5 matrix matrix true
    create_account_with_brain_key com6 com6 matrix matrix true
    transfer matrix com0 100000 CORE "some cash" true
    transfer matrix com1 100000 CORE "some cash" true
    transfer matrix com2 100000 CORE "some cash" true
    transfer matrix com3 100000 CORE "some cash" true
    transfer matrix com4 100000 CORE "some cash" true
    transfer matrix com5 100000 CORE "some cash" true
    transfer matrix com6 100000 CORE "some cash" true
    upgrade_account com0 true
    upgrade_account com1 true
    upgrade_account com2 true
    upgrade_account com3 true
    upgrade_account com4 true
    upgrade_account com5 true
    upgrade_account com6 true
    create_dxpcore_member com0 "http://www.com0" true
    create_dxpcore_member com1 "http://www.com1" true
    create_dxpcore_member com2 "http://www.com2" true
    create_dxpcore_member com3 "http://www.com3" true
    create_dxpcore_member com4 "http://www.com4" true
    create_dxpcore_member com5 "http://www.com5" true
    create_dxpcore_member com6 "http://www.com6" true
    vote_for_dxpcore_member matrix com0 true true
    vote_for_dxpcore_member matrix com1 true true
    vote_for_dxpcore_member matrix com2 true true
    vote_for_dxpcore_member matrix com3 true true
    vote_for_dxpcore_member matrix com4 true true
    vote_for_dxpcore_member matrix com5 true true
    vote_for_dxpcore_member matrix com6 true true

    propose_parameter_change com0 {"block_interval" : 6} true
