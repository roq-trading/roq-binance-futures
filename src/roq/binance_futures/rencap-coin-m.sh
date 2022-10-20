#!/usr/bin/env bash

CWD="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

if [ "$1" == "debug" ]; then
	PREFIX="gdb --args"
else
	PREFIX=
fi

NAME="rencap"

CONFIG_FILE="$CWD/config/$NAME.toml"

URI="binance.com"

REST_URI="https://dapi.$URI"
WS_URI="wss://dstream.$URI/ws"

$PREFIX ./roq-binance-futures \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --client_listen_address $CWD/$NAME.sock \
  --metrics_listen_address $CWD/metrics/${NAME}.sock \
  --ws_uri "$WS_URI" \
  --rest_uri "$REST_URI" \
  --api dapi \
  $@
