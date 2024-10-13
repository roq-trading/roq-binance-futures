# ReferenceData

## LINEAR (USD-M, fapi)

```
{
  symbol="BTCUSDT",
  pair="BTCUSDT",
  contract_type=PERPETUAL,
  delivery_date=4133404800000ms,
  onboard_date=1569398400000ms,
  status=TRADING,
  maint_margin_percent=2.5,
  required_margin_percent=5,
  base_asset="BTC",
  quote_asset="USDT",
  margin_asset="USDT",
  price_precision=2,
  quantity_precision=3,
  base_asset_precision=8,
  quote_precision=8,
  underlying_type="COIN",
  settle_plan=0,
  trigger_protect=0.05000000000000001,
  filters="[{
    "minPrice":"556.80",
    "maxPrice":"4529764",
    "filterType":"PRICE_FILTER",
    "tickSize":"0.10"
  }, {
    "minQty":"0.001",
    "maxQty":"1000",
    "stepSize":"0.001",
    "filterType":"LOT_SIZE"
  }, {
    "filterType":"MARKET_LOT_SIZE",
    "minQty":"0.001",
    "maxQty":"120",
    "stepSize":"0.001"
  }, {"filterType":"MAX_NUM_ORDERS",
    "limit":200
  }, {
    "limit":10,
    "filterType":"MAX_NUM_ALGO_ORDERS"
  }, {
    "notional":"100",
    "filterType":"MIN_NOTIONAL"
  }, {
    "multiplierDown":"0.9500",
    "filterType":"PERCENT_PRICE",
    "multiplierUp":"1.0500",
    "multiplierDecimal":"4"
  }
  ]",
  liquidation_fee=0.012500000000000002,
  market_take_bound=0.05000000000000001,
  contract_status="",
  contract_size=nan
}
```

## INVERSE (COIN-M, dapi)

```
{
  symbol="BTCUSD_241227",
  pair="BTCUSD",
  contract_type=CURRENT_QUARTER,
  delivery_date=1735286400000ms,
  onboard_date=1719561600000ms,
  status=<UNDEFINED>,
  maint_margin_percent=2.5,
  required_margin_percent=5,
  base_asset="BTC",
  quote_asset="USD",
  margin_asset="BTC",
  price_precision=1,
  quantity_precision=0,
  base_asset_precision=8,
  quote_precision=8,
  underlying_type="COIN",
  settle_plan=0,
  trigger_protect=0.05000000000000001,
  filters="[{
    "minPrice":"1000",
    "maxPrice":"4671848",
    "filterType":"PRICE_FILTER",
    "tickSize":"0.1"
  }, {
    "stepSize":"1",
    "filterType":"LOT_SIZE",
    "maxQty":"1000000",
    "minQty":"1"
  }, {
    "stepSize":"1",
    "filterType":"MARKET_LOT_SIZE",
    "maxQty":"1000",
    "minQty":"1"
  }, {
    "limit":200,
    "filterType":"MAX_NUM_ORDERS"
  }, {
    "limit":20,
    "filterType":"MAX_NUM_ALGO_ORDERS"
  }, {
    "multiplierDown":"0.9500",
    "multiplierUp":"1.0500",
    "multiplierDecimal":"4",
    "filterType":"PERCENT_PRICE"
  }
  ]",
  liquidation_fee=0.010000000000000002,
  market_take_bound=0.05000000000000001,
  contract_status="TRADING",
  contract_size=100
}
```

papi
-- no auto-cancel
-- something weird with funds
-- can't download trades
-- failures
-- validate CreateOrder.margin_mode






MAKER
```
body="symbol=XRPUSDT&side=SELL&type=LIMIT&timeInForce=GTC&quantity=5.0&reduceOnly=false&price=1.0865&newClientOrderId=2gAC6gMAAQAAlXbMA0wX&recvWindow=5000"
status=OK, body="{"orderId":17846577272,"symbol":"XRPUSDT","status":"NEW","clientOrderId":"2gAC6gMAAQAAlXbMA0wX","price":"1.0865","avgPrice":"0.00000","origQty":"5","executedQty":"0","cumQty":"0","cumQuote":"0","timeInForce":"GTC","type":"LIMIT","reduceOnly":false,"closePosition":false,"side":"SELL","positionSide":"BOTH","stopPrice":"0","workingType":"CONTRACT_PRICE","priceProtect":false,"origType":"LIMIT","updateTime":1635074448807}"
message="{"e":"ORDER_TRADE_UPDATE","T":1635074448807,"E":1635074448812,"o":{"s":"XRPUSDT","c":"2gAC6gMAAQAAlXbMA0wX","S":"SELL","o":"LIMIT","f":"GTC","q":"5","p":"1.0865","ap":"0","sp":"0","x":"NEW","X":"NEW","i":17846577272,"l":"0","z":"0","L":"0","T":1635074448807,"t":0,"b":"0","a":"5.43250","m":false,"R":false,"wt":"CONTRACT_PRICE","ot":"LIMIT","ps":"BOTH","cp":false,"rp":"0","pP":false,"si":0,"ss":0}}"
order_trade_update={event_type=ORDER_TRADE_UPDATE, event_time=1635074448812ms, transaction_time=1635074448807ms, execution_report={symbol="XRPUSDT", client_order_id="2gAC6gMAAQAAlXbMA0wX", side=SELL, order_type=LIMIT, time_in_force=GTC, original_quantity=5, original_price=1.0865, average_price=0, stop_price=0, execution_type=NEW, order_status=NEW, order_id=17846577272, last_filled_quantity=0, order_filled_accumulated_quantity=0, last_filled_price=0, commission_asset="", commission=nan, order_trade_time=1635074448807ms, trade_id=0, bids_notional=0, asks_notional=5.4325, is_trade_maker=false, is_reduce_only=false, stop_price_working_type=CONTRACT_PRICE, original_order_type=LIMIT, position_side=BOTH, if_close_all=false, activation_price=nan, callback_rate=nan, realized_profit=0, unknown_1=false, unknown_2=0, unknown_3=0}}
message="{"e":"ACCOUNT_UPDATE","T":1635074453178,"E":1635074453187,"a":{"B":[{"a":"USDT","wb":"21.18255611","cw":"21.18255611","bc":"0"}],"P":[{"s":"XRPUSDT","pa":"-5","ep":"1.08650","cr":"0.03000000","up":"0.00032505","mt":"cross","iw":"0","ps":"BOTH","ma":"USDT"}],"m":"ORDER"}}"
item={asset="USDT", wallet_balance=21.18255611, cross_wallet_balance=21.18255611, balance_change=0}
item={symbol="XRPUSDT", position_amount=-5, entry_price=1.0865, accumulated_realized=0.030000000000000006, unrealized_pnl=0.00032505000000000007, margin_type="cross", isolated_wallet=0, position_side=BOTH, unknown_1="USDT"}
message="{"e":"ORDER_TRADE_UPDATE","T":1635074453178,"E":1635074453187,"o":{"s":"XRPUSDT","c":"2gAC6gMAAQAAlXbMA0wX","S":"SELL","o":"LIMIT","f":"GTC","q":"5","p":"1.0865","ap":"1.08650","sp":"0","x":"TRADE","X":"FILLED","i":17846577272,"l":"5","z":"5","L":"1.0865","n":"0.00108650","N":"USDT","T":1635074453178,"t":676522096,"b":"0","a":"0","m":true,"R":false,"wt":"CONTRACT_PRICE","ot":"LIMIT","ps":"BOTH","cp":false,"rp":"0","pP":false,"si":0,"ss":0}}"
order_trade_update={event_type=ORDER_TRADE_UPDATE, event_time=1635074453187ms, transaction_time=1635074453178ms, execution_report={symbol="XRPUSDT", client_order_id="2gAC6gMAAQAAlXbMA0wX", side=SELL, order_type=LIMIT, time_in_force=GTC, original_quantity=5, original_price=1.0865, average_price=1.0865, stop_price=0, execution_type=TRADE, order_status=FILLED, order_id=17846577272, last_filled_quantity=5, order_filled_accumulated_quantity=5, last_filled_price=1.0865, commission_asset="USDT", commission=0.0010865000000000002, order_trade_time=1635074453178ms, trade_id=676522096, bids_notional=0, asks_notional=0, is_trade_maker=true, is_reduce_only=false, stop_price_working_type=CONTRACT_PRICE, original_order_type=LIMIT, position_side=BOTH, if_close_all=false, activation_price=nan, callback_rate=nan, realized_profit=0, unknown_1=false, unknown_2=0, unknown_3=0}}
```

TAKER
```
body="symbol=XRPUSDT&side=BUY&type=LIMIT&timeInForce=GTC&quantity=5.0&reduceOnly=false&price=1.0860&newClientOrderId=cAAC6wMAAQAAjzMqEUwX&recvWindow=5000"
status=OK, body="{"orderId":17846602125,"symbol":"XRPUSDT","status":"NEW","clientOrderId":"cAAC6wMAAQAAjzMqEUwX","price":"1.0860","avgPrice":"0.00000","origQty":"5","executedQty":"0","cumQty":"0","cumQuote":"0","timeInForce":"GTC","type":"LIMIT","reduceOnly":false,"closePosition":false,"side":"BUY","positionSide":"BOTH","stopPrice":"0","workingType":"CONTRACT_PRICE","priceProtect":false,"origType":"LIMIT","updateTime":1635074673046}"
message="{"e":"ORDER_TRADE_UPDATE","T":1635074673046,"E":1635074673052,"o":{"s":"XRPUSDT","c":"cAAC6wMAAQAAjzMqEUwX","S":"BUY","o":"LIMIT","f":"GTC","q":"5","p":"1.0860","ap":"0","sp":"0","x":"NEW","X":"NEW","i":17846602125,"l":"0","z":"0","L":"0","T":1635074673046,"t":0,"b":"0","a":"0","m":false,"R":false,"wt":"CONTRACT_PRICE","ot":"LIMIT","ps":"BOTH","cp":false,"rp":"0","pP":false,"si":0,"ss":0}}"
order_trade_update={event_type=ORDER_TRADE_UPDATE, event_time=1635074673052ms, transaction_time=1635074673046ms, execution_report={symbol="XRPUSDT", client_order_id="cAAC6wMAAQAAjzMqEUwX", side=BUY, order_type=LIMIT, time_in_force=GTC, original_quantity=5, original_price=1.086, average_price=0, stop_price=0, execution_type=NEW, order_status=NEW, order_id=17846602125, last_filled_quantity=0, order_filled_accumulated_quantity=0, last_filled_price=0, commission_asset="", commission=nan, order_trade_time=1635074673046ms, trade_id=0, bids_notional=0, asks_notional=0, is_trade_maker=false, is_reduce_only=false, stop_price_working_type=CONTRACT_PRICE, original_order_type=LIMIT, position_side=BOTH, if_close_all=false, activation_price=nan, callback_rate=nan, realized_profit=0, unknown_1=false, unknown_2=0, unknown_3=0}}
message="{"e":"ACCOUNT_UPDATE","T":1635074673046,"E":1635074673052,"a":{"B":[{"a":"USDT","wb":"21.18338431","cw":"21.18338431","bc":"0"}],"P":[{"s":"XRPUSDT","pa":"0","ep":"0.00000","cr":"0.03300000","up":"0","mt":"cross","iw":"0","ps":"BOTH","ma":"USDT"}],"m":"ORDER"}}"
item={asset="USDT", wallet_balance=21.18338431, cross_wallet_balance=21.18338431, balance_change=0}
item={symbol="XRPUSDT", position_amount=0, entry_price=0, accumulated_realized=0.03300000000000001, unrealized_pnl=0, margin_type="cross", isolated_wallet=0, position_side=BOTH, unknown_1="USDT"}
message="{"e":"ORDER_TRADE_UPDATE","T":1635074673046,"E":1635074673052,"o":{"s":"XRPUSDT","c":"cAAC6wMAAQAAjzMqEUwX","S":"BUY","o":"LIMIT","f":"GTC","q":"5","p":"1.0860","ap":"1.08590","sp":"0","x":"TRADE","X":"FILLED","i":17846602125,"l":"5","z":"5","L":"1.0859","n":"0.00217180","N":"USDT","T":1635074673046,"t":676524317,"b":"0","a":"0","m":false,"R":false,"wt":"CONTRACT_PRICE","ot":"LIMIT","ps":"BOTH","cp":false,"rp":"0.00300000","pP":false,"si":0,"ss":0}}"
order_trade_update={event_type=ORDER_TRADE_UPDATE, event_time=1635074673052ms, transaction_time=1635074673046ms, execution_report={symbol="XRPUSDT", client_order_id="cAAC6wMAAQAAjzMqEUwX", side=BUY, order_type=LIMIT, time_in_force=GTC, original_quantity=5, original_price=1.086, average_price=1.0859, stop_price=0, execution_type=TRADE, order_status=FILLED, order_id=17846602125, last_filled_quantity=5, order_filled_accumulated_quantity=5, last_filled_price=1.0859, commission_asset="USDT", commission=0.0021718000000000006, order_trade_time=1635074673046ms, trade_id=676524317, bids_notional=0, asks_notional=0, is_trade_maker=false, is_reduce_only=false, stop_price_working_type=CONTRACT_PRICE, original_order_type=LIMIT, position_side=BOTH, if_close_all=false, activation_price=nan, callback_rate=nan, realized_profit=0.003000000000000001, unknown_1=false, unknown_2=0, unknown_3=0}}
```
