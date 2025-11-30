.. _roq-binance-futures:

.. |dagger| unicode:: U+2020
.. |double-dagger| unicode:: U+2021
.. |right-arrow| unicode:: U+2192
.. |right-double-arrow| unicode:: U+21D2
.. |left-right-double-arrow| unicode:: U+21D4
.. |check-mark| unicode:: U+2705
.. |cross-mark| unicode:: U+274C
.. |negative-cross-mark| unicode:: U+274E
.. |footnote-1| unicode:: U+2776
.. |footnote-2| unicode:: U+2777
.. |footnote-3| unicode:: U+2778


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


Supports
--------

.. grid::  2
  :gutter: 2

  .. grid-item-card::  Products

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:enumerator:`Spot <roq::SecurityType::SPOT>`
        - |cross-mark|
        -
      * - :cpp:enumerator:`Futures <roq::SecurityType::FUTURES>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Swap <roq::SecurityType::SWAP>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Option <roq::SecurityType::OPTION>`
        - |cross-mark|
        -

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`ReferenceData <roq::ReferenceData>`
        - |check-mark|
        - |footnote-1|
      * - :cpp:class:`MarketStatus <roq::MarketStatus>`
        - |check-mark|
        - |footnote-1|
      * - :cpp:class:`TopOfBook <roq::TopOfBook>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByPrice <roq::MarketByPriceUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByOrder <roq::MarketByOrderUpdate>`
        - |cross-mark|
        -
      * - :cpp:class:`TradeSummary <roq::TradeSummary>`
        - |check-mark|
        -
      * - :cpp:class:`Statistics <roq::StatisticsUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`TimeSeries <roq::TimeSeriesUpdate>`
        - |check-mark|
        -

  .. grid-item-card::  Orders & Quotes

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`CreateOrder <roq::CreateOrder>`
        - |check-mark|
        -
      * - :cpp:class:`ModifyOrder <roq::ModifyOrder>`
        - |check-mark|
        - |footnote-2|
      * - :cpp:class:`CancelOrder <roq::CancelOrder>`
        - |check-mark|
        -
      * - :cpp:class:`CancelAllOrders <roq::CancelAllOrders>`
        - |check-mark|
        -
      * - :cpp:class:`MassQuote <roq::MassQuote>`
        - |cross-mark|
        -
      * - :cpp:class:`CancelQuotes <roq::CancelQuotes>`
        - |cross-mark|
        -

  .. grid-item-card::  Account

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`Funds <roq::FundsUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`Position <roq::PositionUpdate>`
        - |check-mark|
        -

.. note::

   |check-mark| = Available.

   |negative-cross-mark| = Not implemented.

   |cross-mark| = Unavailable.

   |footnote-1| The exchange protocol does not support streaming updates for reference data and market status.

   |footnote-2| The PAPI protocol does not support order modifications.


Using
-----

.. code-block:: shell

   $ roq-binance-futures [FLAGS]


.. _roq-binance-futures-flags:

Flags
-----


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
------------

.. tab:: Prod (USD-M)

   .. code-block:: shell

      $ --flagfile $CONDA_PREFIX/share/roq-binance-futures/flags/prod/flags-fapi.cfg

   .. include:: flags/prod/flags-fapi.cfg
     :code: ini

.. tab:: Prod (COIN-M)

   .. code-block:: shell

      $ --flagfile $CONDA_PREFIX/share/roq-binance-futures/flags/prod/flags-dapi.cfg

   .. include:: flags/prod/flags-dapi.cfg
     :code: ini

.. tab:: Test (USD-M)

   .. code-block:: shell

      $ --flagfile $CONDA_PREFIX/share/roq-binance-futures/flags/test/flags-fapi.cfg

   .. include:: flags/test/flags-fapi.cfg
     :code: ini

.. tab:: Test (COIN-M)

   .. code-block:: shell

      $ --flagfile $CONDA_PREFIX/share/roq-binance-futures/flags/test/flags-dapi.cfg

   .. include:: flags/test/flags-dapi.cfg
     :code: ini


Configuration
-------------

.. code-block:: shell

   $ --flagfile $CONDA_PREFIX/share/roq-binance-futures/config.toml

.. important::

   This template will be replaced when the software is upgraded.
   Make a copy and modify to your own needs.

.. include:: config.toml
   :code: toml


Market Data
-----------


Inbound
~~~~~~~

.. tab:: TradingStatus

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`TRADING`
       - |right-double-arrow|
       - :cpp:enumerator:`OPEN <roq::TradingStatus::OPEN>`

     * - :code:`HALT`
       - |right-double-arrow|
       - :cpp:enumerator:`HALT <roq::TradingStatus::HALT>`

     * - :code:`BREAK`
       - |right-double-arrow|
       - :cpp:enumerator:`CLOSE <roq::TradingStatus::CLOSE>`

     * - :code:`END_OF_DAY`
       - |right-double-arrow|
       - :cpp:enumerator:`END_OF_DAY <roq::TradingStatus::END_OF_DAY>`

     * - :code:`PRE_TRADING`
       - |right-double-arrow|
       - :cpp:enumerator:`PRE_TRADING <roq::TradingStatus::PRE_OPEN>`

     * - :code:`AUCTION_MATCH`
       - |right-double-arrow|
       - :cpp:enumerator:`PRE_OPEN <roq::TradingStatus::PRE_OPEN>`

     * - :code:`POST_TRADING`
       - |right-double-arrow|
       - :cpp:enumerator:`CLOSE <roq::TradingStatus::CLOSE>`

     * - :code:`SETTLING`
       - |right-double-arrow|
       - :cpp:enumerator:`PRE_CLOSE <roq::TradingStatus::PRE_CLOSE>`

     * - :code:`PENDING_TRADING`
       - |right-double-arrow|
       - :cpp:enumerator:`PRE_OPEN <roq::TradingStatus::PRE_OPEN>`

     * - :code:`DELIVERING`
       - |right-double-arrow|
       - :cpp:enumerator:`UNDEFINED <roq::TradingStatus::UNDEFINED>`


.. tab:: StatisticsType

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Event
       - Field
       - Comment
       -
       -

     * - :code:`MarkPriceUpdate`
       - :code:`p`
       - Mark price
       - |right-double-arrow|
       - :cpp:enumerator:`SETTLEMENT_PRICE <roq::StatisticsType::SETTLEMENT_PRICE>`

     * - :code:`MarkPriceUpdate`
       - :code:`P`
       - Est. mark price
       - |right-double-arrow|
       - :cpp:enumerator:`PRE_SETTLEMENT_PRICE <roq::StatisticsType::PRE_SETTLEMENT_PRICE>`

     * - :code:`MarkPriceUpdate`
       - :code:`i`
       - Index price
       - |right-double-arrow|
       - :cpp:enumerator:`INDEX_VALUE <roq::StatisticsType::INDEX_VALUE>`

     * - :code:`MarkPriceUpdate`
       - :code:`r`
       - Funding rate
       - |right-double-arrow|
       - :cpp:enumerator:`FUNDING_RATE <roq::StatisticsType::FUNDING_RATE>`

     * - :code:`MiniTicker`
       - :code:`o`
       - Open price
       - |right-double-arrow|
       - :cpp:enumerator:`OPEN_PRICE <roq::StatisticsType::OPEN_PRICE>`

     * - :code:`MiniTicker`
       - :code:`h`
       - High price
       - |right-double-arrow|
       - :cpp:enumerator:`HIGHEST_TRADED_PRICE <roq::StatisticsType::HIGHEST_TRADED_PRICE>`

     * - :code:`MiniTicker`
       - :code:`l`
       - Low price
       - |right-double-arrow|
       - :cpp:enumerator:`LOWEST_TRADED_PRICE <roq::StatisticsType::LOWEST_TRADED_PRICE>`

     * - :code:`MiniTicker`
       - :code:`c`
       - Close price
       - |right-double-arrow|
       - :cpp:enumerator:`CLOSE_PRICE <roq::StatisticsType::CLOSE_PRICE>`

     * - :code:`MiniTicker`
       - :code:`v`
       - Total volume (base)
       - |right-double-arrow|
       - :cpp:enumerator:`TRADE_VOLUME <roq::StatisticsType::TRADE_VOLUME>`


Order Management
----------------


Inbound
~~~~~~~

.. tab:: OrderType

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`MARKET`
       - |right-double-arrow|
       - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`

     * - :code:`LIMIT`
       - |right-double-arrow|
       - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`


.. tab:: TimeInForce

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`GTC`
       - |right-double-arrow|
       - :cpp:enumerator:`GTC <roq::TimeInForce::GTC>`

     * - :code:`IOC`
       - |right-double-arrow|
       - :cpp:enumerator:`IOC <roq::TimeInForce::IOC>`

     * - :code:`FOK`
       - |right-double-arrow|
       - :cpp:enumerator:`FOK <roq::TimeInForce::FOK>`

     * - :code:`GTX`
       - |right-double-arrow|
       - :cpp:enumerator:`GTX <roq::TimeInForce::GTX>`


.. tab:: OrderStatus

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`NEW`
       - |right-double-arrow|
       - :cpp:enumerator:`WORKING <roq::OrderStatus::WORKING>`

     * - :code:`PARTIALLY_FILLED`
       - |right-double-arrow|
       - :cpp:enumerator:`WORKING <roq::OrderStatus::WORKING>`

     * - :code:`FILLED`
       - |right-double-arrow|
       - :cpp:enumerator:`COMPLETED <roq::OrderStatus::COMPLETED>`

     * - :code:`CANCELED`
       - |right-double-arrow|
       - :cpp:enumerator:`CANCELED <roq::OrderStatus::CANCELED>`

     * - :code:`EXPIRED`
       - |right-double-arrow|
       - :cpp:enumerator:`EXPIRED <roq::OrderStatus::EXPIRED>`

     * - :code:`NEW_INSURANCE`
       - |right-double-arrow|
       - :cpp:enumerator:`UNDEFINED <roq::OrderStatus::UNDEFINED>`

     * - :code:`NEW_ADL`
       - |right-double-arrow|
       - :cpp:enumerator:`UNDEFINED <roq::OrderStatus::UNDEFINED>`


Outbound
~~~~~~~~

.. tab:: CreateOrder

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - :cpp:member:`order_type <roq::CreateOrder::order_type>`
       - :cpp:member:`execution_instructions <roq::CreateOrder::execution_instructions>`
       - :cpp:member:`price <roq::CreateOrder::price>`
       - :cpp:member:`stop_price <roq::CreateOrder::stop_price>`
       -
       - :code:`type`
       - :code:`price`
       - :code:`stopPrice`
       - :code:`reduceOnly`

     * - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`
       -
       - :code:`NaN`
       - :code:`NaN`
       - |right-double-arrow|
       - :code:`MARKET`
       - |cross-mark|
       - |cross-mark|
       -

     * - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`
       -
       - :code:`NaN`
       - |check-mark|
       - |right-double-arrow|
       - :code:`MARKET`
       - |cross-mark|
       - |check-mark|
       -

     * - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`
       -
       - |check-mark|
       - :code:`NaN`
       - |right-double-arrow|
       - :code:`LIMIT`
       - |check-mark|
       - |cross-mark|
       -

     * - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`
       -
       - |check-mark|
       - |check-mark|
       - |right-double-arrow|
       - :code:`LIMIT`
       - |check-mark|
       - |check-mark|
       -


.. tab:: ModifyOrder

   TBD


.. tab:: CancelOrder

   TBD


.. tab:: CancelAllOrders

   TBD


Comments
--------

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


Exchange
~~~~~~~~

* `Website <https://www.binance.com/futures/>`__
* `Testnet <https://testnet.binancefuture.com/futures/>`__
* `Documentation <https://www.binance.com/binance-api/>`__
