/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "wsapi_parser_tester.hpp"

using namespace roq;
using namespace roq::binance_futures;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::WSAPIAccountBalance;

TEST_CASE("fapi", "[json_wsapi_account_balance]") {
  auto message = R"({)"
                 R"("id":"hIQeAAUAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",)"
                 R"("status":200,)"
                 R"("result":[{)"
                 R"("accountAlias":"mYmYfWoCuXFzfWfW",)"
                 R"("asset":"FDUSD",)"
                 R"("balance":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("marginAvailable":true,)"
                 R"("updateTime":0)"
                 R"(},{)"
                 R"("accountAlias":"mYmYfWoCuXFzfWfW",)"
                 R"("asset":"LDUSDT",)"
                 R"("balance":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("maxWithdrawAmount":"0.00000000",)"
                 R"("marginAvailable":true,)"
                 R"("updateTime":0)"
                 R"(},{)"
                 R"("accountAlias":"mYmYfWoCuXFzfWfW",)"
                 R"("asset":"USDC",)"
                 R"("balance":"375.00000000",)"
                 R"("crossWalletBalance":"375.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"375.00000000",)"
                 R"("maxWithdrawAmount":"375.00000000",)"
                 R"("marginAvailable":true,)"
                 R"("updateTime":1764103550800)"
                 R"(})"
                 R"(],)"
                 R"("rateLimits":[{)"
                 R"("rateLimitType":"REQUEST_WEIGHT",)"
                 R"("interval":"MINUTE",)"
                 R"("intervalNum":1,)"
                 R"("limit":2400,)"
                 R"("count":10)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "hIQeAAUAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    CHECK(obj.status == 200);
    REQUIRE(std::size(obj.result) == 3);
    REQUIRE(std::size(obj.rate_limits) == 1);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("dapi", "[json_wsapi_account_balance]") {
  auto message = R"({)"
                 R"("id":"hIQeAAUAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",)"
                 R"("status":200,)"
                 R"("result":[{)"
                 R"("accountAlias":"mYmYfWmYSgAuTisR",)"
                 R"("asset":"ETH",)"
                 R"("balance":"0.09952085",)"
                 R"("withdrawAvailable":"0.09952085",)"
                 R"("crossWalletBalance":"0.09952085",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.09952085",)"
                 R"("updateTime":1765291573774)"
                 R"(},{)"
                 R"("accountAlias":"mYmYfWmYSgAuTisR",)"
                 R"("asset":"XLM",)"
                 R"("balance":"0.00000000",)"
                 R"("withdrawAvailable":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("updateTime":0)"
                 R"(},{)"
                 R"("accountAlias":"mYmYfWmYSgAuTisR",)"
                 R"("asset":"RUNE",)"
                 R"("balance":"0.00000000",)"
                 R"("withdrawAvailable":"0.00000000",)"
                 R"("crossWalletBalance":"0.00000000",)"
                 R"("crossUnPnl":"0.00000000",)"
                 R"("availableBalance":"0.00000000",)"
                 R"("updateTime":0)"
                 R"(})"
                 R"(],)"
                 R"("rateLimits":[{)"
                 R"("rateLimitType":"REQUEST_WEIGHT",)"
                 R"("interval":"MINUTE",)"
                 R"("intervalNum":1,)"
                 R"("limit":2400,)"
                 R"("count":21)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "hIQeAAUAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    CHECK(obj.status == 200);
    REQUIRE(std::size(obj.result) == 3);
    REQUIRE(std::size(obj.rate_limits) == 1);
  };
  WSAPIParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
