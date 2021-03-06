# Quick Tips for Dxperts Developers

This guide is intended for developers who will be using the existing Dxperts Core platform. Detailed documentation can be found at the following resources.

- [Developer documentation](https://dev.dxperts.works)
- [General Documentation](https://how.dxperts.works/)

Quick tips for common developer actions are linked to below.

---

**Table of Contents**

- [Accounts, Permissions, and Authorities](#accounts)
- [Standard Transactions](#standard-transactions)
- [Proposed Transactions](#proposed-transactions)
- [User Issued Assets](#user-issued-assets)
- [Vesting Balances](#vesting-balances)
- [Hashed Time-Lock Contracts](#htlc)
- [Software Tools](#software-tools)
- [Configuration for Testnet](#how-to-testnet)

---

# <div id="accounts"/> Accounts, Permissions, and Authorities

- [About Accounts](https://dev.dxperts.works/en/master/dxp_guide/accounts/index_account.html)
- [About Account Permissions](https://dev.dxperts.works/en/master/dxp_guide/accounts/dxp_permissions.html)
- [About Dynamic Account Permissions](https://dxperts.org/technology/dynamic-account-permissions/)
- [About Multi-Signature Authorities](https://dev.dxperts.works/en/master/dxp_guide/accounts/dxp_multi-sign.html#dxp-multi-sign)
- [How to change account authority thresholds (GUI)](https://dev.dxperts.works/en/master/dxp_guide/accounts/dxp_permissions.html#permissions-in-wallet-settings)
- [How to register an account](https://dev.dxperts.works/en/master/dxp_guide/accounts/account-create.html#create-account-dev-cli)
- [How to import a GUI-wallet account into the CLI-wallet](https://dev.dxperts.works/en/master/dxp_guide/tutorials/cli_import_guiwallet_account.html#howto-import-gui-wallet-account-cli)
- [How to generate a random key in the CLI](https://dev.dxperts.works/en/master/api/namespaces/wallet.html?highlight=suggest#classgraphene_1_1wallet_1_1utility_1a2c813fc0587d67ed483372ff38bb5273)

# <div id="standard-transactions"/> Standard Transactions

Transactions allow multiple operations to be performed atomically within Dxperts

- [How to construct a transaction](https://dev.dxperts.works/en/master/dxp_guide/tutorials/construct-transaction.html#manually-construct-transaction) in the [CLI wallet](#CLI)
- [Tips about transactions](https://dev.dxperts.works/en/master/dxp_guide/tutorials/index.html#transfer-transactions)

# <div id="proposed-transactions"/> Proposed Transactions

Proposed transaction are useful for performing actions on the blockchain that require approval of multiple accounts.

- [About Proposed Transactions](https://dev.dxperts.works/en/master/knowledge_base/trn_proposed_transactions.html#proposed-tran)
- [How to create a proposed transaction](https://dev.dxperts.works/en/master/dxp_guide/tutorials/construct-transaction.html#manually-construct-transaction)
- [How to approve a proposed transaction](https://dev.dxperts.works/en/master/dxp_guide/tutorials/propose-transaction.html#approving-a-proposal)

# <div id="user-issued-assets"/> User-Issued Assets (UIA)

- [About User-Issued Assets](https://dev.dxperts.works/en/master/dxp_guide/tokens/uia.html)
- [How to create a UIA in the GUI](https://dev.dxperts.works/en/master/dxp_guide/tutorials/uia-create-gui.html#creating-new-uia-gui)
- [How to create a UIA in the CLI](https://dev.dxperts.works/en/master/dxp_guide/tutorials/uia-create-manual.html#uia-create-manual) ([Example](https://gitlab.com/kongfuyuan/dxperts-core_wiki/Testing-HF-1268:-Market-Fee-Sharing#create-and-issue-the-uia))
- [How to create parent.child assets](https://dev.dxperts.works/en/master/dxp_guide/index_faq.html#what-about-parent-and-child-assets)
- [How to update an existing UIA](https://dev.dxperts.works/en/master/dxp_guide/tutorials/uia-update-manual.html#uia-update-manual)
- [How can the issuer issue a UIA](https://gitlab.com/kongfuyuan/dxperts-core_wiki/Testing-HF-1268:-Market-Fee-Sharing#asset-owner-issue-the-asset)
- [How can the issuer retract a UIA](https://steemit.com/dxperts/@xeroc/how-the-issuer-of-an-iouuia-can-transfer-assets-back-to-himself)
- [How can the issuer burn a UIA (CLI)](https://dev.dxperts.works/en/master/api/wallet_api.html?highlight=burn#reserve-asset)

# <div id="vesting-balances"/> Vesting Balances

- [About vesting balances](https://dev.dxperts.works/en/master/dxp_guide/accounts/vesting_balances.html?highlight=vesting)
- [Example of vesting balances with CryptoBridge](https://crypto-bridge.org/2018/10/09/what-it-means-to-stake/)
- [How to create vesting balance](https://dev.dxperts.works/en/master/components/lib_operations.html?highlight=vesting#vesting-balance-create-operation)
- [How to check a vesting balance](https://dev.dxperts.works/en/master/dxp_guide/tutorials/vesting-list.html#list-vesting-balances)
- [Example of supplementing payouts for vesting balances](https://cryptobridge.freshdesk.com/support/solutions/articles/35000061225-how-will-funds-be-distributed-)
- [How to claim a vesting balance (CLI)](https://dev.dxperts.works/en/master/dxp_guide/tutorials/vesting-claim.html#claiming-vesting-balance)
- [How to claim a vesting balance (GUI Wallet)](https://dev.dxperts.works/en/master/dxp_guide/accounts/vesting_balances.html?highlight=vesting#claiming-a-vesting-balance)

# <div id="htlc" /> Hashed Time-Lock Contracts (HTLC)

- [How to create and redeem an HTLC (CLI)](https://gitlab.com/kongfuyuan/dxperts-core_wiki/HTLC)

# <div id="software-tools"/> Software Tools

## API Calls

- [How to make one-off API calls](https://gitlab.com/kongfuyuan/dxperts-core_wiki/API)
- [About Remote Procedure Call API](https://dev.dxperts.works/en/master/api/rpc.html#rpc)
- [How to subscribe to object data with websockets](https://gitlab.com/kongfuyuan/dxperts-core_wiki/Websocket-Subscriptions)

## Software Clients and Libraries

- [Open Source GUI Wallets](https://gitlab.com/dxperts/awesome-dxperts#opensource-wallets)
- Open Source Command Line Wallets
  - [CLI Wallet (C++)](https://dev.dxperts.works/en/master/development/index_cli.html)
  - [Uptick (Python)](https://gitlab.com/dxperts/uptick)
- [Open Source Libraries](https://gitlab.com/dxperts/awesome-dxperts#libraries)
- [Open Source Tools](https://gitlab.com/dxperts/awesome-dxperts#tools-and-scripts)

# <div id="how-to-testnet"/> How to Configure Software for Testnet

## Reference Wallet

The [GUI reference wallet](https://gitlab.com/Robertz0/dxperts-ui), which is also [hosted by several parties](https://gitlab.com/dxperts/awesome-dxperts#hosted-wallets), can be connected to any testnode under Settings.

## <div id="CLI" /> Command Line Interface Wallet (CLI)

- The [CLI Wallet (C++)](https://dev.dxperts.works/en/master/development/index_cli.html) must
  - have been built from the [testnet branch](https://gitlab.com/dxperts)
  - be configured to point to a public API node by using the `-s` switch

```
cli_wallet -s ws://<HOST_NAME_OR_IP>:<HOST_PORT>
```

or if using a secure connection

```
cli_wallet -s wss://<HOST_NAME_OR_IP>:<HOST_PORT>
```

## Python Wallet

[Uptick](https://gitlab.com/dxperts/uptick) is a Python-based CLI tool set for Dxperts blockchain. Documentation can be found [here](http://uptick.rocks/).

It can be configured to connect to the public testnet by setting any testnet API node. For example

```
uptick set node wss://testnet.nodes.dxperts.ws
```

## DEXBot

[DEXBot](http://dexbot.info/) is a market maker bot that can be configured to point to a testnet API node by leaving it as the only API node in the configuration file.
