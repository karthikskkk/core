In contrast to most cryptocurrency wallets, the Dxperts 2.0 has a different
model to represent the blockchain, its transactions and accounts. This chapter
wants to given an introduction to the concepts of _objects_ as they are used by
the Dxperts 2.0 client. Furthermore, we will briefly introduce the API and
show how to subscribe to object changes (such as new blocks or incoming
deposits). Afterwards, we will show how exchange may monitor their accounts and
credit incoming funds to their corresponding users.

Another helpful document can be found at this link: [Dxperts Developers Portal - Blockchain Objects and their Identifiers](https://dev.dxperts.works/en/master/api/blockchain-objects-ids.html).

## Objects

On the Dxperts blockchains there are no addresses, but objects identified by a
unique _id_, an _type_ and a _space_ in the form:

    space.type.id

Some examples:

    1.2.15   # protocol space / account / id: 15
    1.6.105  # protocol space / blockproducer / id: 105
    1.14.7   # protocol space / benefactor / id: 7

    2.1.0    # implementation space / dynamic global properties
    2.3.8    # implementation space / dynamic asset data / id: 8

    2.5.80    # implementation space / account-balance / id: 80
    2.6.80    # implementation space / account-statistics / id: 80
    2.9.80    # implementation space / account-transactions / id: 80
    2.7.80    # implementation space / transactions / id: 80
    2.8.80    # implementation space / block-summary / id: 80

Related source code: [this](https://gitlab.com/dxperts/dxperts-core/-/blob/main/libraries/protocol/include/graphene/protocol/types.hpp) and [this](https://gitlab.com/dxperts/dxperts-core/-/blob/main/libraries/chain/include/graphene/chain/types.hpp).

## Accounts

The Dxperts blockchain users are required to register each account with a
unique username and 3 public keys on the blockchain. The blockchain assigns an
incremental user _id_ and offers to resolve the name-to-id pair. For instance
`1.2.15`.
