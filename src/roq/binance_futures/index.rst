.. _roq-binance-futures:

.. |checkmark| unicode:: U+2713

roq-binance-futures
===================

.. tab:: Unstable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/unstable \
           roq-binance-futures

.. tab:: Stable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/stable \
           roq-binance-futures


:code:`roq-binance-futures`
---------------------------

.. code-block:: shell

   $ roq-binance-futures [FLAGS]


Description
~~~~~~~~~~~

:code:`roq-binance-futures` is a gateway


Supports
~~~~~~~~

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
      * - Combos
        -

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto

      * - Reference Data
        - |checkmark|
      * - Market Status
        - |checkmark|
      * - Top of Book
        - |checkmark|
      * - Market by Price
        - |checkmark|
      * - Market by Order
        -
      * - Trade Summary
        - |checkmark|
      * - Statistics
        - |checkmark|
      * - Time Series
        - |checkmark|

  .. grid-item-card::  Order Management

    .. list-table::
      :widths: auto

      * - Create
        - |checkmark|
      * - Modify
        - (|checkmark|)
      * - Cancel
        - |checkmark|
      * - Cancel All
        - |checkmark|
      * - Auto-Cancel
        - (|checkmark|)

  .. grid-item-card::  Account Management

    .. list-table::
      :widths: auto

      * - Positions
        - |checkmark|
      * - Funds
        - |checkmark|

.. note::

   * Modify and Auto-Cancel not possible with PAPI.


.. _roq-binance-futures-flags:

Flags
~~~~~


.. code-block:: shell

   $ roq-binance-futures --help

.. tab:: Flags

   .. include:: flags/flags.rstinc

.. tab:: REST

   .. include:: flags/rest.rstinc

.. tab:: WS

   .. include:: flags/ws.rstinc

.. tab:: WS API

   .. include:: flags/ws_api.rstinc

.. tab:: MBP

   .. include:: flags/mbp.rstinc

.. tab:: Request

   .. include:: flags/request.rstinc

.. tab:: Misc

   .. include:: flags/misc.rstinc


Environments
~~~~~~~~~~~~

.. tab:: Prod (USD-M)

   .. code-block:: shell

      $ $CONDA_PREFIX/share/roq-binance-futures/flags/prod/flags-fapi.cfg

   .. include:: flags/prod/flags-fapi.cfg
     :code: ini

.. tab:: Prod (COIN-M)

   .. code-block:: shell

      $ $CONDA_PREFIX/share/roq-binance-futures/flags/prod/flags-dapi.cfg

   .. include:: flags/prod/flags-dapi.cfg
     :code: ini

.. tab:: Test (USD-M)

   .. code-block:: shell

      $ $CONDA_PREFIX/share/roq-binance-futures/flags/test/flags-fapi.cfg

   .. include:: flags/test/flags-fapi.cfg
     :code: ini

.. tab:: Test (COIN-M)

   .. code-block:: shell

      $ $CONDA_PREFIX/share/roq-binance-futures/flags/test/flags-dapi.cfg

   .. include:: flags/test/flags-dapi.cfg
     :code: ini


Configuration
~~~~~~~~~~~~~

.. code-block:: shell

   $ $CONDA_PREFIX/share/roq-binance-futures/config.toml

.. important::

   The template will be replaced when the software is upgraded.
   Make a copy and modify to your needs.

.. include:: config.toml
   :code: toml


Market Data
~~~~~~~~~~~

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
^^^^^^^^^^

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
~~~~~~~~~~~~~~~~

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
^^^^^^^^^^^

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
^^^^^^^^^^^^^

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
^^^^^^^^^^^^^^^

.. note::

  Not supported


Execution Instructions
^^^^^^^^^^^^^^^^^^^^^^

TBD


Account Management
~~~~~~~~~~~~~~~~~~

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
~~~~~~~

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
~~~~~~~~~~~

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
~~~~~~~~

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

* PAPI has a race between order matching an trade reporting

  * A modify or cancel request may be rejected with the :code:`Order does not exist.` text message if the order
    has already been matched but the completion has not yet been reported.

  * This is indicative that Binance's implementation has matching engine logic separated from PAPI / trade reporting.
    The matching engine will not report fills directly, rather it will hand the update off to centralized logic that
    will udpate and validate portfolio margin.

* WSAPI is **WORK-IN-PROGRESS**

  * Some exchange features are missing (they are available from spot WSAPI):

    * Download working orders

    * Download trade history

    * Cancel all working orders


References
----------

Common
~~~~~~

* :ref:`Using Conda <tutorial-conda>`
* :ref:`Using Flags <abseil-cpp>`
* :ref:`Gateway Flags <gateway-flags>`
* :ref:`Gateway Config <gateway-config>`

Binance
~~~~~~~

* `Website <https://www.binance.com/en/futures/BTCUSDT>`__
* `Testnet <https://testnet.binancefuture.com/en/futures/BTCUSDT>`__
* `Support <https://www.binance.com/en/support-center>`__
* `Documentation <https://binance-docs.github.io/apidocs/futures/en/>`__
