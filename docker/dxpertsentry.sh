#!/bin/bash
DXPERTSD="/usr/local/bin/blockproducer_node"

# For blockchain download
VERSION=`cat /etc/dxperts/version`

## Supported Environmental Variables
#
#   * $DXPERTSD_SEED_NODES
#   * $DXPERTSD_RPC_ENDPOINT
#   * $DXPERTSD_PLUGINS
#   * $DXPERTSD_REPLAY
#   * $DXPERTSD_RESYNC
#   * $DXPERTSD_P2P_ENDPOINT
#   * $DXPERTSD_BLOCKPRODUCER_ID
#   * $DXPERTSD_PRIVATE_KEY
#   * $DXPERTSD_TRACK_ACCOUNTS
#   * $DXPERTSD_PARTIAL_OPERATIONS
#   * $DXPERTSD_MAX_OPS_PER_ACCOUNT
#   * $DXPERTSD_ES_NODE_URL
#   * $DXPERTSD_ES_START_AFTER_BLOCK
#   * $DXPERTSD_TRUSTED_NODE
#

ARGS=""
# Translate environmental variables
if [[ ! -z "$DXPERTSD_SEED_NODES" ]]; then
    for NODE in $DXPERTSD_SEED_NODES ; do
        ARGS+=" --seed-node=$NODE"
    done
fi
if [[ ! -z "$DXPERTSD_RPC_ENDPOINT" ]]; then
    ARGS+=" --rpc-endpoint=${DXPERTSD_RPC_ENDPOINT}"
fi

if [[ ! -z "$DXPERTSD_REPLAY" ]]; then
    ARGS+=" --replay-blockchain"
fi

if [[ ! -z "$DXPERTSD_RESYNC" ]]; then
    ARGS+=" --resync-blockchain"
fi

if [[ ! -z "$DXPERTSD_P2P_ENDPOINT" ]]; then
    ARGS+=" --p2p-endpoint=${DXPERTSD_P2P_ENDPOINT}"
fi

if [[ ! -z "$DXPERTSD_BLOCKPRODUCER_ID" ]]; then
    ARGS+=" --blockproducer-id=$DXPERTSD_BLOCKPRODUCER_ID"
fi

if [[ ! -z "$DXPERTSD_PRIVATE_KEY" ]]; then
    ARGS+=" --private-key=$DXPERTSD_PRIVATE_KEY"
fi

if [[ ! -z "$DXPERTSD_TRACK_ACCOUNTS" ]]; then
    for ACCOUNT in $DXPERTSD_TRACK_ACCOUNTS ; do
        ARGS+=" --track-account=$ACCOUNT"
    done
fi

if [[ ! -z "$DXPERTSD_PARTIAL_OPERATIONS" ]]; then
    ARGS+=" --partial-operations=${DXPERTSD_PARTIAL_OPERATIONS}"
fi

if [[ ! -z "$DXPERTSD_MAX_OPS_PER_ACCOUNT" ]]; then
    ARGS+=" --max-ops-per-account=${DXPERTSD_MAX_OPS_PER_ACCOUNT}"
fi

if [[ ! -z "$DXPERTSD_ES_NODE_URL" ]]; then
    ARGS+=" --elasticsearch-node-url=${DXPERTSD_ES_NODE_URL}"
fi

if [[ ! -z "$DXPERTSD_ES_START_AFTER_BLOCK" ]]; then
    ARGS+=" --elasticsearch-start-es-after-block=${DXPERTSD_ES_START_AFTER_BLOCK}"
fi

if [[ ! -z "$DXPERTSD_TRUSTED_NODE" ]]; then
    ARGS+=" --trusted-node=${DXPERTSD_TRUSTED_NODE}"
fi

## Link the dxperts config file into home
## This link has been created in Dockerfile, already
ln -f -s /etc/dxperts/config.ini /var/lib/dxperts
ln -f -s /etc/dxperts/logging.ini /var/lib/dxperts

# Plugins need to be provided in a space-separated list, which
# makes it necessary to write it like this
if [[ ! -z "$DXPERTSD_PLUGINS" ]]; then
   exec "$DXPERTSD" --data-dir "${HOME}" ${ARGS} ${DXPERTSD_ARGS} --plugins "${DXPERTSD_PLUGINS}"
else
   exec "$DXPERTSD" --data-dir "${HOME}" ${ARGS} ${DXPERTSD_ARGS}
fi
