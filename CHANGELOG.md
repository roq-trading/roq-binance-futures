# Change Log

All notable changes will be documented in this file.

## Head

### Changed

* Market data support for `--net_disconnect_on_idle_timeout`.

## 0.8.4 &ndash; 2022-05-14

### Changed

* Missing `TopOfBook.exchange_time_utc` (#205)

## 0.8.3 &ndash; 2022-03-22

### Changed

* Download orders (#39)

### Fixed

* Invalid client order id's when using `routing_id` (#183)

## 0.8.2 &ndash; 2022-02-18

## 0.8.1 &ndash; 2022-01-16

## 0.8.0 &ndash; 2022-01-12

### Added

* Support COIN-M API (#147)

## 0.7.9 &ndash; 2021-12-08

## 0.7.8 &ndash; 2021-11-02

### Added

* Add exchange sequence number to `MarketByPrice` and `MarketByOrder` (#101)
* Add `max_trade_vol` and `trade_vol_step_size` to ReferenceData (#100)

### Changed

* Move cache utilities to API (#111)
* Interface to support binary data from web::socket
* ReferenceData currencies should follow FX conventions (#99)
* Replace `snapshot` (bool) with `update_type` (UpdateType) (#97)
* Moved signature handling to tools library (chore)

### Removed

* Remove custom literals (#110)
* Remove external rate-limiter mirroring from the REST connection (#83)

## 0.7.7 &ndash; 2021-09-20

### Changed

* Added HTTP `request_id` (#55)
* Use `string_buffer` + `std::back_inserter` instead of `string_builder` (#53)
* Added new symbols fields: liquidationFee and marketTakeBound (#54)

## 0.7.6 &ndash; 2021-09-02

### Changed

* New order management interface (#25)

## 0.7.5 &ndash; 2021-08-08

## 0.7.4 &ndash; 2021-07-20

## 0.7.3 &ndash; 2021-07-06

## 0.7.2 &ndash; 2021-06-20

## 0.7.1 &ndash; 2021-05-30
