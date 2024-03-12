.. _roq-binance-futures:

.. |checkmark| unicode:: U+2713

roq-binance-futures
===================

.. important::
   There are different network end-points required for USD-M and COIN-M futures.
   The API's are different but sufficiently similar to allow this gateway to
   support both. (The :code:`--api` flag controls which API will be used.)
   The implication of this is that you will need more instances of this gateway
   if you need support for both product groups.


Links
-----

* `Website <https://www.binance.com/en/futures/BTCUSDT>`__
* `Testnet <https://testnet.binancefuture.com/en/futures/BTCUSDT>`__
* `Support <https://www.binance.com/en/support-center>`__
* `API <https://binance-docs.github.io/apidocs/futures/en/>`__


Purpose
-------

* Maintain network connectivity with the Binance Futures exchange
* Route exchange updates to connected clients
* Route client requests to the relevant exchange accounts
* Stream all messages to an event-log


Overview
--------

.. grid::  2
  :gutter: 2

  .. grid-item-card::  Products

    .. list-table::
      :widths: auto

      * - Spot
        -
      * - Futures
        - |checkmark|
      * - Options
        -

    .. note::
       Crypto and USDT margined products are **NOT** supported by the same API.

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto

      * - Reference Data
        - |checkmark|
      * - Market Status
        - |checkmark|
      * - Top of Book
        - |checkmark|
      * - Market by Price (L2)
        - |checkmark|
      * - Market by Order (L3)
        -
      * - Trade Summary
        - |checkmark|
      * - Statistics
        - |checkmark|

  .. grid-item-card::  Order Management

    .. list-table::
      :widths: auto

      * - Create
        - |checkmark|
      * - Modify
        -
      * - Cancel
        - |checkmark|
      * - Cancel All
        - |checkmark|
      * - Auto Cancellation
        - |checkmark| (*)

  .. grid-item-card::  Account Management

    .. list-table::
      :widths: auto

      * - Positions
        - |checkmark|
      * - Funds
        - |checkmark|

* Data center located in Japan (to be confirmed)
* No test environment
* Auto-cancel is not possible with PAPI


Conda
-----

* :ref:`Using Conda <tutorial-conda>`

.. tab:: Install

  .. code-block:: bash

    $ mamba install \
      --channel https://roq-trading.com/conda/stable \
      roq-binance-futures

.. tab:: Configure

  .. code-block:: bash

    $ cp $CONDA_PREFIX/share/roq-binance-futures/config.toml $CONFIG_FILE_PATH

    # Then modify $CONFIG_FILE_PATH to match your specific configuration

.. tab:: Run

  .. code-block:: bash

    $ roq-binance-futures \
          --name "binance-futures" \
          --config_file "$CONFIG_FILE_PATH" \
          --client_listen_address "$UNIX_SOCKET_PATH" \
          --service_listen_address "$TCP_LISTEN_PORT" \
          --flagfile "$FLAG_FILE"


Config
------

* :ref:`Common Config <gateway-config>`


.. _roq-binance-futures-flags:

Flags
-----

* :ref:`Using Flags <abseil-cpp>`
* :ref:`Common Flags <gateway-flags>`

.. code-block:: bash

   $ roq-binance-futures --help

.. tab:: Flags

   .. include:: flags/flags.rstinc

.. tab:: Common

   .. include:: flags/common.rstinc

.. tab:: REST

   .. include:: flags/rest.rstinc

.. tab:: WS

   .. include:: flags/ws.rstinc


Environments
------------

.. code-block:: bash

  $ $CONDA_PREFIX/share/roq-binance-futures/flags

USD-M Futures
~~~~~~~~~~~~~

.. tab:: Prod

   .. include:: flags/prod/flags-fapi.cfg
     :code: ini

.. tab:: Test

   .. include:: flags/test/flags-fapi.cfg
     :code: ini

COIN-M Futures
~~~~~~~~~~~~~~

.. tab:: Prod

   .. include:: flags/prod/flags-dapi.cfg
     :code: ini

.. tab:: Test

   .. include:: flags/test/flags-dapi.cfg
     :code: ini


Market Data
-----------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::ReferenceData`
      -
      -
      - Unavailable

    * - :cpp:class:`roq::MarketStatus`
      -
      -
      - Unavailable

    * - :cpp:class:`roq::TopOfBook`
      - MarketData
      - <symbol>@bookTicker
      -

    * - :cpp:class:`roq::MarketByPriceUpdate`
      - MarketData
      - <symbol>@depth@<freq>
      -

    * - :cpp:class:`roq::MarketByOrderUpdate`
      -
      -
      - Unavailable

    * - :cpp:class:`roq::TradeSummary`
      - MarketData
      - <symbol>@aggTrade
      -

    * - :cpp:class:`roq::StatisticsUpdate`
      - MarketData
      - <symbol>@miniTicker, <symbol>@markPrice
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::ReferenceData`
      - OrderEntry
      - GET /fapi/v1/exchangeInfo
      - There is **no** live feed

    * - :cpp:class:`roq::MarketStatus`
      - OrderEntry
      - GET /fapi/v1/exchangeInfo
      - There is **no** live feed

    * - :cpp:class:`roq::TopOfBook`
      -
      -
      -

    * - :cpp:class:`roq::MarketByPriceUpdate`
      - OrderEntry
      - GET /fapi/v1/depth
      - See :ref:`roq-binance-futures-flags`

    * - :cpp:class:`roq::MarketByOrderUpdate`
      -
      -
      -

    * - :cpp:class:`roq::TradeSummary`
      -
      -
      -

    * - :cpp:class:`roq::StatisticsUpdate`
      -
      -
      -

Statistics
~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`HIGHEST_TRADED_PRICE`
    - (miniTicker) :code:`highPrice`

  * - :cpp:class:`LOWEST_TRADED_PRICE`
    - (miniTicker) :code:`lowPrice`

  * - :cpp:class:`OPEN_PRICE`
    - (miniTicker) :code:`openPrice`

  * - :cpp:class:`CLOSE_PRICE`
    - (miniTicker) :code:`closePrice`

  * - :cpp:class:`SETTLEMENT_PRICE`
    - (markPrice) :code:`markPrice`

  * - :cpp:class:`PRE_SETTLEMENT_PRICE`
    - (markPrice) :code:`estSettlePrice`

  * - :cpp:class:`INDEX_VALUE`
    - (markPrice) :code:`indexPrice`

  * - :cpp:class:`FUNDING_RATE`
    - (markPrice) :code:`fundingRate`


Order Management
----------------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderUpdate`
      - DropCopy
      - ORDER_TRADE_UPDATE
      -

    * - :cpp:class:`roq::TradeUpdate`
      - DropCopy
      - ORDER_TRADE_UPDATE
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderUpdate`
      - OrderEntry
      - GET /fapi/v1/openOrders
      - It is only possible to download **open** orders

    * - :cpp:class:`roq::TradeUpdate`
      - OrderEntry
      - GET /fapi/v1/userTrades
      - There is a limit of **1000** trades

.. tab:: Request

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::CreateOrder`
      - OrderEntry
      - POST /fapi/v1/order
      -

    * - :cpp:class:`roq::ModifyOrder`
      -
      -
      - Unavailable

    * - :cpp:class:`roq::CancelOrder`
      - OrderEntry
      - DELETE /fapi/v1/order
      -

    * - :cpp:class:`roq::CancelAllOrders`
      - OrderEntry
      - DELETE /fapi/v1/allOpenOrders
      - This request is per-symbol!
        Only executed for those symbols where the gateway has seen order actions or download.

.. tab:: Response

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderAck`
      - OrderEntry
      - /fapi/v1/order
      -


Order Types
~~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`MARKET`
    - Mapped to :code:`'MARKET'` (JSON)

  * - :cpp:class:`LIMIT`
    - Mapped to :code:`'LIMIT'` (JSON)


Time in Force
~~~~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`GTC`
    - Mapped to :code:`'GTC'` (JSON)

  * - :cpp:class:`IOC`
    - Mapped to :code:`'IOC'` (JSON)

  * - :cpp:class:`FOK`
    - Mapped to :code:`'FOK'` (JSON)

  * - :cpp:class:`GTX`
    - Mapped to :code:`'GTX'` (JSON)


Position Effect
~~~~~~~~~~~~~~~

.. note::

  Not supported


Execution Instructions
~~~~~~~~~~~~~~~~~~~~~~

TBD


Account Management
------------------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::PositionUpdate`
      - DropCopy
      - ACCOUNT_UPDATE
      -

    * - :cpp:class:`roq::FundsUpdate`
      - DropCopy
      - ACCOUNT_UPDATE
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::PositionUpdate`
      - OrderEntry
      - GET /fapi/v2/account
      -

    * - :cpp:class:`roq::FundsUpdate`
      - OrderEntry
      - GET /fapi/v2/balance
      -


Streams
-------

.. tab:: OrderEntry

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - REST
      - Primary purpose

        * support order management

        Each connection

        * supports a single account
        * maintains a listen key (used by the DropCopy stream)

.. tab:: DropCopy

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - WebSocket
      - Primary purpose

        * live account updates, including orders and fills

        Each connection

        * supports a single account

.. tab:: MarketData

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - WebSocket
      - Primary purpose

        * live market data

        Each connection

        * supports a slice of the symbols

.. tab:: Rest

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - REST
      - Primary purpose

        * download reference data


        One connection


Constraints
------------

* It is only possible to download current order status for open orders.
  The implication is that backup procedures must be implemented to reoncile positions in the
  scenario where orders are completely filled during a disconnect.

* The :code:`newClientOrderId` field (used by :code:`CreateOrder`) must conform to the
  :code:`^[\.A-Z\:/a-z0-9_-]{1,36}$` regular expression (ECMAScript).
  This restricts length and character used when supplying the :code:`routing_id` field.

* The exchange will monitor rate-limit usage per IP address.

* Rate-limit usage is quite strict when downloading full order books.
  Due to this constraint, it may take a very long time to initialize all symbols.
  It is therefore **STRONGLY** recommended to reduce the configured number of symbols, e.g.
  :code:`symbols=".*BTC.*"`, or even more specific by using lists.


Comments
------------

* External trades can optionally be captured into the event log.

  .. note::

     These messages will not be routed to any client.

* Trades can optionally be downloaded.
  This is a very expensive operation and the list of symbols to download must
  therefore be explicitly controlled by the :code:`--download_symbols` flag.

* There are different end-points depending on the margin-mode.

  * If nothing is specified, the classic margin-mode is selected.
    The end-points are then taken from :code:`--rest_uri` and :code:`--ws_uri`.

  * The new end-points are selected if the toml config has :code:`margin_mode = "portfolio"`.
    The end-points are then taken from :code:`--rest_pm_uri` and :code:`--ws_pm_uri`.
