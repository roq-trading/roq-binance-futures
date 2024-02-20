#!/usr/bin/env bash

if [ "$1" == "debug" ]; then
  PREFIX="gdb --args"
else
  PREFIX=
fi

NAME="binance-futures"

CONFIG="${CONFIG:-$NAME}-testnet"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-binance-futures/$CONFIG.toml"

URI="binancefuture.com"

REST_URI="https://testnet.$URI"
WS_URI="wss://fstream.$URI/ws"

$PREFIX ./roq-binance-futures \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --cache_dir "$HOME/var/lib/roq/cache" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --event_log_symlink true \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  --ws_uri "$WS_URI" \
  --rest_uri "$REST_URI" \
  $@
