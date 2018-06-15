#include "bitstamp.h"
#include "parameters.h"
#include "utils/restapi.h"
#include "utils/base64.h"
#include "unique_json.hpp"
#include "hex_str.hpp"

#include "openssl/sha.h"
#include "openssl/hmac.h"
#include <chrono>
#include <thread>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <iomanip>

namespace Bitstamp {

static json_t* authRequest(Parameters &, std::string, std::string);

static RestApi& queryHandle(Parameters &params)
{
  static RestApi query ("https://www.bitstamp.net",
                        params.cacert.c_str(), *params.logFile);
  return query;
}

static json_t* checkResponse(std::ostream &logFile, json_t *root)
{
  auto errstatus = json_object_get(root, "error");
  if (errstatus)
  {
    // Bitstamp errors could sometimes be strings or objects containing error string
    auto errmsg = json_dumps(errstatus, JSON_ENCODE_ANY);
    logFile << "<Bitstamp> Error with response: "
            << errmsg << '\n';
    free(errmsg);
  }

  return root;
}

std::string getMatchingPair(std::string pair) {
  if (pair.compare("btcusd") == 0) {
    return "btcusd";
  } else if (pair.compare("ethusd") == 0) {
    return "ethusd";
  } else if (pair.compare("btceur") == 0) {
    return "btceur";
  } else if (pair.compare("etheur") == 0) {
    return "etheur";
  } else if (pair.compare("bchusd") == 0) {
    return "bchusd";
  } else {
    return "";
  }
}

quote_t getQuote(Parameters& params, std::string pair)
{
  std::string matchingPair = getMatchingPair(pair);
  if (matchingPair.compare("") == 0) {
    *params.logFile << "<Bitstamp> Pair not supported" << std::endl;
    // return "0";
  }

  auto &exchange = queryHandle(params);
  unique_json root { exchange.getRequest("/api/v2/ticker/"+ matchingPair) };

  const char *quote = json_string_value(json_object_get(root.get(), "bid"));
  auto bidValue = quote ? atof(quote) : 0.0;

  quote = json_string_value(json_object_get(root.get(), "ask"));
  auto askValue = quote ? atof(quote) : 0.0;

  return std::make_pair(bidValue, askValue);
}

double getAvail(Parameters& params, std::string currency)
{
  *params.logFile << "<Bitstamp> getAvail" << std::endl;

  unique_json root { authRequest(params, "/api/v2/balance/", "") };
  while (json_object_get(root.get(), "message") != NULL)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto dump = json_dumps(root.get(), 0);
    *params.logFile << "<Bitstamp> Error with JSON: " << dump << ". Retrying..." << std::endl;
    free(dump);
    root.reset(authRequest(params, "/api/v2/balance/", ""));
  }
  double availability = 0.0;
  const char* returnedText = NULL;
  if (currency == "btc")
  {
    returnedText = json_string_value(json_object_get(root.get(), "btc_balance"));
  }
  else if (currency == "usd")
  {
    returnedText = json_string_value(json_object_get(root.get(), "usd_balance"));
  }
  else if (currency == "eur")
  {
    returnedText = json_string_value(json_object_get(root.get(), "eur_balance"));
  }
  else if (currency == "eth")
  {
    returnedText = json_string_value(json_object_get(root.get(), "eth_balance"));
  }
  else if (currency == "xrp")
  {
    returnedText = json_string_value(json_object_get(root.get(), "xrp_balance"));
  }
  else if (currency == "bch")
  {
    returnedText = json_string_value(json_object_get(root.get(), "bch_balance"));
  }
  if (returnedText != NULL)
  {
    availability = atof(returnedText);
  }
  else
  {

    returnedText = json_string_value(json_object_get(root.get(), ""));
    *params.logFile << "<Bitstamp> Error with the credentials." << "/api/v2/balance/"+currency+"usd - " << returnedText << std::endl;
    availability = 0.0;
  }

  return availability;
}

std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair)
{
  std::string matchingPair = getMatchingPair(pair);
  if (matchingPair.compare("") == 0) {
    *params.logFile << "<Bitstamp> Pair not supported" << std::endl;
    return "0";
  }

  *params.logFile << "<Bitstamp> Trying to send a \"" << direction << "\" limit order: "
                  << std::setprecision(6) << quantity << "@$"
                  << std::setprecision(2) << price << "...\n";
  std::string url = "/api/v2/" + direction + '/' + matchingPair + '/';

  std::ostringstream oss;
  oss << "amount=" << quantity << "&price=" << std::fixed << std::setprecision(2) << price;
  std::string options = oss.str();
  unique_json root { authRequest(params, url, options) };
  //TODO why id is 0 here?
  /*
    <Bitstamp> Trying to send a "buy" limit order: 0.022242@$1,123.69...
    <Bitstamp> Order ID = 0. Message: {"price": "1123.69", "amount": "0.02224220", "type": "0", "id": "1362032057", "datetime": "2018-04-21 00:03:20.774398"}
    <Bitstamp> Done (order ID: 0)
  */

  auto orderId = std::to_string(json_integer_value(json_object_get(root.get(), "id")));
  if (orderId == "0")
  {
    auto dump = json_dumps(root.get(), 0);
    *params.logFile << "<Bitstamp> Order ID = 0. Message: " << dump << '\n';
    free(dump);
  }
  *params.logFile << "<Bitstamp> Done (order ID: " << orderId << ")\n" << std::endl;

  return orderId;
}

bool isOrderComplete(Parameters& params, std::string orderId)
{
  if (orderId == "0") return true;

  auto options = "id=" + orderId;
  unique_json root { authRequest(params, "/api/order_status/", options) };
  auto status = json_string_value(json_object_get(root.get(), "status"));
  return status && status == std::string("Finished");
}

double getActivePos(Parameters& params, std::string currency) { return getAvail(params, currency); }

double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair)
{
  std::string matchingPair = getMatchingPair(pair);
  if (matchingPair.compare("") == 0) {
    *params.logFile << "<Bitstamp> Pair not supported" << std::endl;
    // return "0";
  }

  auto &exchange = queryHandle(params);
  unique_json root { exchange.getRequest("/api/v2/order_book/"+matchingPair) };
  auto orderbook = json_object_get(root.get(), isBid ? "bids" : "asks");

  // loop on volume
  *params.logFile << "<Bitstamp> Looking for a limit price to fill "
                  << std::setprecision(6) << fabs(volume) << " " << pair << " ...\n";
  double tmpVol = 0.0;
  double p = 0.0;
  double v;
  int i = 0;
  while (tmpVol < fabs(volume) * params.orderBookFactor)
  {
    p = atof(json_string_value(json_array_get(json_array_get(orderbook, i), 0)));
    v = atof(json_string_value(json_array_get(json_array_get(orderbook, i), 1)));
    *params.logFile << "<Bitstamp> order book: "
                    << std::setprecision(6) << v << "@$"
                    << std::setprecision(2) << p << std::endl;
    tmpVol += v;
    i++;
  }
  return p;
}

json_t* authRequest(Parameters &params, std::string request, std::string options)
{
  static uint64_t nonce = time(nullptr) * 4;
  auto msg = std::to_string(++nonce) + params.bitstampClientId + params.bitstampApi;
  uint8_t *digest = HMAC (EVP_sha256(),
                          params.bitstampSecret.c_str(), params.bitstampSecret.size(),
                          reinterpret_cast<const uint8_t *>(msg.data()), msg.size(),
                          nullptr, nullptr);

  std::string postParams = "key=" + params.bitstampApi +
                           "&signature=" + hex_str<upperhex>(digest, digest + SHA256_DIGEST_LENGTH) +
                           "&nonce=" + std::to_string(nonce);
  if (!options.empty())
  {
    postParams += "&";
    postParams += options;
  }

  auto &exchange = queryHandle(params);
  return checkResponse(*params.logFile, exchange.postRequest(request, postParams));
}

}
