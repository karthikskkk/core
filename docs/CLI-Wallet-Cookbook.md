# Graphene CLI Wallet Cookbook

### Running a Local Test Network

Right now, there is no public testnet, so the only way to test is to run your
own private network. To do this, launch a blockproducer node to generate blocks. In
the directory where you built your graphene distribution:

```
cd programs/blockproducer_node
# if you have previously run a blockproducer node, you may need to remove the old blockchain.
# at this early stage, new commits often make it impossible to reuse an old database
#   rm -r blockproducer_node_data_dir
./blockproducer_node --rpc-endpoint "127.0.0.1:8090" --enable-stale-production -w \""1.6.0"\" \""1.6.1"\" \""1.6.2"\" \""1.6.3"\" \""1.6.4"\"
```

The initial genesis state has ten pre-configured delegates (1.6.0-9) that all
use the same private key to sign their blocks, and the blockproducer node has the
private keys for these initial delgates built in.. Launching `blockproducer_node`
this way allows you to act as all ten delegates.

Now, in a second window, launch a `cli_wallet` process to interact with the
network.

```
cd programs/cli_wallet
# similarly, if you have previously run a wallet, you may need to wipe out your
# old wallet
#    rm wallet.json
./cli_wallet
```

Before doing anything with the new wallet, set a password and unlock the
wallet.

_Warning_: your passwords will be displayed on the screen.

```
new >>> set_password my_password
locked >>> unlock my_password
unlocked >>>
```

### Account Management

To create a new account, you will need to start with an existing account with
some of the CORE asset that will pay the transaction fee registering your new
account. The account paying this fee will be the _Registrar_.

In the initial genesis state, there are about a dozen pre-existing accounts.
We use the 'matrix' account as a general purpose test account, and its private
key is printed at blockproducer startup to allow us to import it here:

```
# first, import the private key to take ownership of the 'matrix' account
unlocked >>> import_key "matrix" 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3
unlocked >>> list_my_accounts
[{
    "id": "1.2.15",
    ...
    "name": "matrix",
    ...
]
unlocked >>> list_account_balances matrix
unlocked >>>
```

We control the account now, but there is no money in the account yet. In the genesis
state, none of the accounts have balances in them. In the first Dxperts network,
accounts were less tightly coupled to balances. Balances were associated with
public keys, and an account could have hundreds of public keys with balances (or,
conversely, public keys with balances could exist without any account associated
with them). When the real network launches, each of the public keys with a balance
will be converted into a _balance object_ in Graphene, and they will not be associated
with any account until their owner publishes a transaction claiming the balance.

In the test genesis state, there is only one balance object and it owns 100% of the
funds in the system. Let's import that here:

```
unlocked >>> import_balance matrix [5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3] true
unlocked >>> list_account_balances matrix
10000000000 CORE
```

So we now have an account to act as registrar and it has plenty of funds to pay
the registration key for new accounts. Only lifetime (prime?) members are allowed
to register accounts, so we must upgrade `matrix` first. Then, go ahead and create
our test account named _my-account_:

```
# before matrix can create other accounts, we need to upgrade it to a prime member.
unlocked >>> upgrade_account matrix true
# register our account.  we list matrix as both the referrer and registrar.
unlocked >>> create_account_with_brain_key "this is the brain key for my account" my-account matrix matrix true
```

Like most methods in the wallet, `create_account_with_brain_key`'s last
parameter is the boolean `broadcast`. This parameter tells the wallet whether
you want to publish the transaction on the network immediately, which is
usually what you want to do. If you pass false, it will just create the
transaction and sign it, and display it on the console, but it wouldn't be sent
out onto the network. This could be used to build up a multi-sig transaction
and collect the other signatures offline, or it could be used to construct a
transaction in a offline cold wallet that you could put on a flash drive and
broadcast from a machine connected to the network. Here, we'll always pass
`true` for the `broadcast` parameter.

If you were to execute `list_my_accounts` now, you would see that you
control both `matrix` and `my-account`.

### Transferring Currency

Your newly-created account doesn't have any funds in it yet, the `matrix`
account still has all the money. To send some CORE from `matrix` to your
account, use the `transfer` command:

```
unlocked >>> transfer matrix my-account 10000 CORE "have some CORE" true
```

### Becoming a Blockproducer

To become a blockproducer and be able to produce blocks, you first need to create a
blockproducer object that can be voted in.

Note: If you want to experiment with things that require voting, be aware that
votes are only tallied once per day at the maintenance interval. For testing,
it's helpful to change the `GRAPHENE_DEFAULT_MAINTENANCE_INTERVAL` in
`libraries/chain/include/graphene/chain/config.hpp` to, say, 10 minutes.

Before we get started, we can see the current list of blockproducers voted in, which
will simply be the ten default blockproducers:

```
unlocked >>> get_global_properties
...
  "active_blockproducers": [
    "1.6.0",
    "1.6.1",
    "1.6.2",
    "1.6.3",
    "1.6.4",
    "1.6.5",
    "1.6.6",
    "1.6.7",
    "1.6.8",
    "1.6.9"
  ],
...
```

Only lifetime members can become blockproducers, so you must first upgrade to a
lifetime member. Upgrade and create our blockproducer object.

```
unlocked >>> upgrade_account my-account true
unlocked >>> create_blockproducer my-account "http://blockproducer.bar.com/" true
{
  "ref_block_num": 139,
  "ref_block_prefix": 3692461913,
  "relative_expiration": 3,
  "operations": [[
      21,{
        "fee": {
          "amount": 0,
          "asset_id": "1.3.0"
        },
        "blockproducer_account": "1.2.16",
        "url": "http://blockproducer.bar.com/",
        "block_signing_key": "PUBLIC KEY",
        "initial_secret": "00000000000000000000000000000000000000000000000000000000"
      }
    ]
  ],
  "signatures": [
      "1f2ad5597af2ac4bf7a50f1eef2db49c9c0f7616718776624c2c09a2dd72a0c53a26e8c2bc928f783624c4632924330fc03f08345c8f40b9790efa2e4157184a37"
  ]
}
```

Our blockproducer is registered, but it can't produce blocks because nobody has voted
it in. You can see the current list of active blockproducers with
`get_global_properties`:

```
unlocked >>> get_global_properties
{
  "active_blockproducers": [
    "1.6.0",
    "1.6.1",
    "1.6.2",
    "1.6.3",
    "1.6.4",
    "1.6.5",
    "1.6.7",
    "1.6.8",
    "1.6.9"
  ],
  ...
```

Now, we should vote our blockproducer in. Vote all of the shares in both
`my-account` and `matrix` in favor of your new blockproducer.

```
unlocked >>> vote_for_blockproducer my-account my-account true true
unlocked >>> vote_for_blockproducer matrix my-account true true
```

Now we wait until the next maintenance interval.
`get_dynamic_global_properties` tells us when that will be in
`next_maintenance_time`. Once the next maintenance interval passes, run
`get_global_properties` again and you should see that your new blockproducer has been
voted in.

Even though it's voted in, it isn't producing any blocks yet because we only
told the blockproducer_node to produce blocks for 1.6.0 - 1.6.9 on the command line,
and it doesn't know the private key for the blockproducer. Get the blockproducer object
using `get_blockproducer` and take note of two things. The `id` is displayed in
`get_global_properties` when the blockproducer is voted in, and we will need it
on the `blockproducer_node` command line to produce blocks. We'll also need the
public `signing_key` so we can look up the correspoinding private key.

Once we have that, run `dump_private_keys` which lists the public-key
private-key pairs to find the private key.

Warning: `dump_private_keys` will display your keys unencrypted on the
terminal, don't do this with someone looking over your shoulder.

```
unlocked >>> get_blockproducer my-account
{
  "id": "1.6.10",
  "blockproducer_account": "1.2.16",
  "signing_key": "GPH7vQ7GmRSJfDHxKdBmWMeDMFENpmHWKn99J457BNApiX1T5TNM8",
}
unlocked >>> dump_private_keys
[[
  ...
  ],[
    "GPH7vQ7GmRSJfDHxKdBmWMeDMFENpmHWKn99J457BNApiX1T5TNM8",
    "5JGi7DM7J8fSTizZ4D9roNgd8dUc5pirUe9taxYCUUsnvQ4zCaQ"
  ]
]
```

Now we need to re-start the blockproducer, so shut down the wallet (ctrl-d), and
shut down the blockproducer (ctrl-c). Re-launch the blockproducer, now mentioning the new
blockproducer 1.6.10 and its keypair:

```
./blockproducer_node --rpc-endpoint=0.0.0.0:8090 --enable-stale-production --blockproducer-id \""1.6.0"\" \""1.6.1"\" \""1.6.2"\" \""1.6.3"\" \""1.6.4"\"  \""1.6.5"\" \""1.6.6"\" \""1.6.7"\" \""1.6.8"\" \""1.6.9"\"  \""1.6.10"\" --private-key "[\"GPH7vQ7GmRSJfDHxKdBmWMeDMFENpmHWKn99J457BNApiX1T5TNM8\", \"5JGi7DM7J8fSTizZ4D9roNgd8dUc5pirUe9taxYCUUsnvQ4zCaQ\"]"
```

If you monitor the output of the `blockproducer_node`, you should see it generate
blocks signed by your blockproducer:

```
Blockproducer 1.6.10 production slot has arrived; generating a block now...
Generated block #367 with timestamp 2015-07-05T20:46:30 at time 2015-07-05T20:46:30
```

### Becoming a Delegate

Becoming a delegate is almost the same as becoming a blockproducer, but it is
simpler because delegates don't have a separate private key for signing blocks.

As for blockproducers, only lifetime members can become delegates, so you must first
upgrade to a lifetime member if you haven't already. Upgrade and create our
delegate object.

```
unlocked >>> upgrade_account my-account true
unlocked >>> create_delegate my-account "http://delegate.baz.com/" true
```

Now that we're registered as a delegate, we should vote e should vote our delegate
in. Vote all of the shares in both `my-account` and `matrix` in favor of your new delegate.

```
unlocked >>> vote_for_delegate my-account my-account true true
unlocked >>> vote_for_delegate matrix my-account true true
```

Like with blockproducers, you will have to wait for the next maintenance interval before
the delegate becomes active. Get the id of the delegate with:

```
unlocked >>> get_delegate my-account
{
  "id": "1.5.10",
  ...
```

and then run `get_global_properties` after the maintenance period and you should
see the new delegate `1.5.10` listed in the `active_delegates` list.

### Creating New Keys

Registering a new account requires the provision of at least two public keys to the registrar: one for the owner key and another for the active key. Some registrars require a third public key for the memo.

Pseudo-random keys for an account can be created by using the CLI Wallet command `suggest_brain_key`.

```
locked >>> suggest_brain_key
{
  "brain_priv_key": "XXXXXX ...... ...... ...... ...... ...... ...... ...... ...... ...... ...... ...... ...... ...... ...... ......",
  "wif_priv_key": "5......",
  "pub_key": "TEST......"
}
```

The public key is provided to a registrar during account registration. The WIF private key should be securely archived.

After account registration, a key can be imported into a CLI wallet with the command `import_key`.

```
locked >>> gethelp import_key

usage: import_key ACCOUNT_NAME_OR_ID  WIF_PRIVATE_KEY

example: import_key "1.3.11" 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3
example: import_key "usera" 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3
```

### Claiming Accumulated Fees

The accumulated fees for the asset owner can be claimed with the following steps.

Two identifiers will be needed to claim the vesting balance.

- The identifier of the account (1.2.x)
- The identifier of the asset (1.3.y)
- The amount to be claimed in satoshis (N)

The identifier of the account can be found by inspecting the `id` ("1.3.x") in the output of `get_account`

```
get_account <ACCOUNT_NAME>
```

The identifier of the asset can be found by inspecting the `id` in the output of `get_token` ("1.2.y").

```
get_token <ACCOUNT_NAME>
```

Withdraw the vesting balance with the asset_claim_fees_operation which has an operation identifier of 43. This will require building a transaction in the CLI Wallet.

```
begin_builder_transaction
add_operation_to_builder_transaction 0 [43, {"issuer":"1.2.x", "amount_to_claim":{"amount":N, "asset_id":"1.3.y"}}]
set_fees_on_builder_transaction 0 1.3.0
```

Optionally preview the transaction before signing.

```
preview_builder_transaction 0
```

Sign and broadcast the transaction.

```
sign_builder_transaction 0 true
```
