#!/usr/bin/env bash

if [ "$1" == "debug" ]; then
  PREFIX="gdb --args"
else
  PREFIX=
fi

NAME="binance-futures"

CONFIG="${CONFIG:-$NAME}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-binance-futures/$CONFIG.toml"

FLAGFILE="../../../share/flags/prod/flags-fapi.cfg"

WS_API=true

DOWNLOAD_SYMBOLS="BTCUSD_PERP"

$PREFIX ./roq-binance-futures \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --flagfile "$FLAGFILE" \
  --cache_dir "$HOME/var/lib/roq/cache" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  --ws_api=$WS_API \
  --download_symbols="$DOWNLOAD_SYMBOLS" \
  $@
