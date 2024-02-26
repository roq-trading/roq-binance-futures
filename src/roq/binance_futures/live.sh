#!/usr/bin/env bash

if [ "$1" == "debug" ]; then
  PREFIX="gdb --args"
else
  PREFIX=
fi

NAME="binance-futures"

CONFIG="${CONFIG:-$NAME}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-binance-futures/$CONFIG.toml"

URI="binance.com"

API="dapi"

REST_URI="https://$API.$URI"
WS_URI="wss://${API:0:1}stream.$URI/ws"

REST_PM_URI="https://papi.$URI"
WS_PM_URI="wss://fstream.$URI/pm/ws"

$PREFIX ./roq-binance-futures \
  --name "$NAME" \
  --api "$API" \
  --config_file "$CONFIG_FILE" \
  --cache_dir "$HOME/var/lib/roq/cache" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --event_log_symlink true \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  --ws_uri "$WS_URI" \
  --rest_uri "$REST_URI" \
  --ws_pm_uri "$WS_PM_URI" \
  --rest_pm_uri "$REST_PM_URI" \
  --download_symbols="BTCUSD_PERP" \
  --download_trades_lookback=300s \
  $@
