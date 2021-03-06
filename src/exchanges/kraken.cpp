#include <iomanip>
#include <vector>
#include <array>
#include <ctime>
#include "kraken.h"
#include "parameters.h"
#include "utils/base64.h"

#include "openssl/sha.h"
#include "openssl/hmac.h"

namespace NSExchange
{

// Initialise internal variables
static unique_json krakenTicker = nullptr;
static bool krakenGotTicker = false;


RestApi& Kraken::queryHandle(Parameters &params)
{
  static RestApi query("https://api.kraken.com",
                       params.cacert.c_str(), *params.logFile);
  return query;
}

std::string Kraken::getMatchingPair(std::string pair)
{
  if (pair.compare("btcusd") == 0) {
    return "XXBTZUSD";
  } else if (pair.compare("ethusd") == 0) {
    return "XETHZUSD";
  } else if (pair.compare("btceur") == 0) {
    return "XXBTZEUR";
  } else if (pair.compare("etheur") == 0) {
    return "XETHZEUR";
  } else if (pair.compare("bchusd") == 0) {
    return "BCHUSD";
  } else {
    return "";
  }
}

// quote_t getQuote(Parameters &params, std::string pair)
// {
//   std::string matchingPair = getMatchingPair(pair);
//   if (matchingPair.compare("") == 0) {
//     *params.logFile << "<Kraken> Pair not supported" << std::endl;
//     // return "0";
//   }

//   if (krakenGotTicker) {
//     krakenGotTicker = false;
//   } else {
//     auto &exchange = queryHandle(params);
//     krakenTicker.reset(exchange.getRequest("/0/public/Ticker?pair="+ matchingPair));
//     krakenGotTicker = true;
//   }

//   json_t *root = krakenTicker.get();
//   const char *quote = json_string_value(json_array_get(json_object_get(json_object_get(json_object_get(root, "result"), matchingPair), "b"), 0));
//   auto bidValue = quote ? std::stod(quote) : 0.0;

//   quote = json_string_value(json_array_get(json_object_get(json_object_get(json_object_get(root, "result"), matchingPair), "a"), 0));
//   auto askValue = quote ? std::stod(quote) : 0.0;

//   return std::make_pair(bidValue, askValue);
// }

quote_t Kraken::getQuote(Parameters &params, std::string pair)
{
  std::string matchingPair = getMatchingPair(pair);
  if (matchingPair.compare("") == 0) {
    *params.logFile << "<Kraken> Pair not supported" << std::endl;
    // return "0";
  }

  if (krakenGotTicker)
  {
    krakenGotTicker = false;
  }
  else
  {
    auto &exchange = queryHandle(params);
    krakenTicker.reset(exchange.getRequest("/0/public/Ticker?pair="+matchingPair));
    krakenGotTicker = true;
  }

  const char * c = matchingPair.c_str();

  json_t *root = krakenTicker.get();
  const char *quote = json_string_value(json_array_get(json_object_get(json_object_get(json_object_get(root, "result"), c), "b"), 0));
  auto bidValue = quote ? std::stod(quote) : 0.0;

  quote = json_string_value(json_array_get(json_object_get(json_object_get(json_object_get(root, "result"), c), "a"), 0));
  auto askValue = quote ? std::stod(quote) : 0.0;


  // std::string orderId;
  // *params.logFile << "Sending Short XMR order for 0.177 XMR @BID! USD: ";
  // orderId = sendShortOrder(params,"sell",0.372817, 1125.90, pair);
  // *params.logFile << "Order: " << orderId;


  return std::make_pair(bidValue, askValue);
}

double Kraken::getAvail(Parameters &params, std::string currency)
{
  *params.logFile << "<Kraken> getAvail" << std::endl;

  unique_json root{authRequest(params, "/0/private/Balance")};
  json_t *result = json_object_get(root.get(), "result");
  if (json_object_size(result) == 0)
  {
    return 0.0;
  }
  double available = 0.0;
  if (currency.compare("usd") == 0)
  {
    const char *avail_str = json_string_value(json_object_get(result, "ZUSD"));
    available = avail_str ? atof(avail_str) : 0.0;
  }
  else if (currency.compare("btc") == 0)
  {
    const char *avail_str = json_string_value(json_object_get(result, "XXBT"));
    available = avail_str ? atof(avail_str) : 0.0;
  } else if (currency.compare("eur") == 0) {
    const char * avail_str = json_string_value(json_object_get(result, "EUR"));
    available = avail_str ? atof(avail_str) : 0.0;
  } else if (currency.compare("eth") == 0) {
    const char * avail_str = json_string_value(json_object_get(result, "ETH"));
    available = avail_str ? atof(avail_str) : 0.0;
  } else if (currency.compare("xrp") == 0) {
    const char * avail_str = json_string_value(json_object_get(result, "XXRP"));
    available = avail_str ? atof(avail_str) : 0.0;
  } else if (currency.compare("bch") == 0) {
    const char * avail_str = json_string_value(json_object_get(result, "BCH"));
    available = avail_str ? atof(avail_str) : 0.0;
  } else {
    *params.logFile << "<Kraken> Currency not supported" << std::endl;
  }

  return available;
}

std::string Kraken::sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair)
{
  return sendOrder(params, direction, quantity, price, pair);
}

std::string Kraken::sendOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair)
{
  if (direction.compare("buy") != 0 && direction.compare("sell") != 0)
  {
    *params.logFile << "<Kraken> Error: Neither \"buy\" nor \"sell\" selected" << std::endl;
    return "0";
  }
  std::string matchingPair = getMatchingPair(pair);
  if (matchingPair.compare("") == 0) {
    *params.logFile << "<Kraken> Pair not supported" << std::endl;
    return "0";
  }
  *params.logFile << "<Kraken> Trying to send a \"" << direction << "\" limit order: "
                  << std::setprecision(6) << quantity << " @ $"
                  << std::setprecision(2) << price << "...\n";


  // std::string pair = matchingPair;
  std::string type = direction;
  std::string ordertype = "limit";
  std::string pricelimit = std::to_string(price);
  std::string volume = std::to_string(quantity);
  std::string options = "pair=" + matchingPair + "&type=" + type + "&ordertype=" + ordertype + "&price=" + pricelimit + "&volume=" + volume + "&trading_agreement=agree";
  unique_json root{authRequest(params, "/0/private/AddOrder", options)};
  json_t *res = json_object_get(root.get(), "result");
  if (json_is_object(res) == 0)
  {
    *params.logFile << json_dumps(root.get(), 0) << std::endl;
    exit(0);
  }
  std::string txid = json_string_value(json_array_get(json_object_get(res, "txid"), 0));
  *params.logFile << "<Kraken> Done (transaction ID: " << txid << ")\n"
                  << std::endl;
  return txid;
}

std::string Kraken::sendShortOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair)
{
  if (direction.compare("buy") != 0 && direction.compare("sell") != 0)
  {
    *params.logFile << "<Kraken> Error: Neither \"buy\" nor \"sell\" selected" << std::endl;
    return "0";
  }
  std::string matchingPair = getMatchingPair(pair);
  if (matchingPair.compare("") == 0) {
    *params.logFile << "<Kraken> Pair not supported" << std::endl;
    return "0";
  }

  *params.logFile << "<Kraken> Trying to send a short \"" << direction << "\" limit order: "
                  << std::setprecision(6) << quantity << " @ $"
                  << std::setprecision(2) << price << "...\n";
  // std::string pair = matchingPair;
  std::string type = direction;
  std::string ordertype;
  std::string options;
  std::string pricelimit = std::to_string(price);
  std::string volume = std::to_string(quantity);
  std::string leverage = "2";
  ordertype = "limit";
  options = "pair=" + matchingPair + "&type=" + type + "&ordertype=" + ordertype + "&price=" + pricelimit + "&volume=" + volume + "&leverage=" + leverage + "&trading_agreement=agree";
  // options = "pair=" + matchingPair + "&type=" + type + "&ordertype=" + ordertype + "&price=" + pricelimit + "&volume=" + volume + "&trading_agreement=agree";
  unique_json root{authRequest(params, "/0/private/AddOrder", options)};
  json_t *res = json_object_get(root.get(), "result");
  if (json_is_object(res) == 0)
  {
    *params.logFile << json_dumps(root.get(), 0) << std::endl;
    exit(0);
  }
  std::string txid = json_string_value(json_array_get(json_object_get(res, "txid"), 0));
  *params.logFile << "<Kraken> Done (transaction ID: " << txid << ")\n"
                  << std::endl;
  return txid;
}

bool Kraken::isOrderComplete(Parameters &params, std::string orderId)
{
  unique_json root{authRequest(params, "/0/private/OpenOrders")};
  // no open order: return true
  auto res = json_object_get(json_object_get(root.get(), "result"), "open");
  if (json_object_size(res) == 0)
  {
    *params.logFile << "<Kraken> No order exists" << std::endl;
    return true;
  }
  res = json_object_get(res, orderId.c_str());
  // open orders exist but specific order not found: return true
  if (json_object_size(res) == 0)
  {
    *params.logFile << "<Kraken> Order " << orderId << " does not exist" << std::endl;
    return true;
    // open orders exist and specific order was found: return false
  }
  else
  {
    *params.logFile << "<Kraken> Order " << orderId << " still exists!" << std::endl;
    return false;
  }
}

double Kraken::getActivePos(Parameters &params, std::string currency)
{
  return getAvail(params, currency);
}

double Kraken::getLimitPrice(Parameters &params, double volume, bool isBid, std::string pair)
{
  std::string matchingPair = getMatchingPair(pair);
  if (matchingPair.compare("") == 0) {
    *params.logFile << "<Kraken> Pair not supported" << std::endl;
    // return "0";
  }
  auto &exchange = queryHandle(params);
  unique_json root { exchange.getRequest("/0/public/Depth?pair="+ matchingPair) };

  const char * c = matchingPair.c_str();

  auto branch = json_object_get(json_object_get(root.get(), "result"), c);
  branch = json_object_get(branch, isBid ? "bids" : "asks");

  // loop on volume
  double totVol = 0.0;
  double currPrice = 0;
  double currVol = 0;
  unsigned int i;
  // [[<price>, <volume>, <timestamp>], [<price>, <volume>, <timestamp>], ...]
  for (i = 0; i < json_array_size(branch); i++)
  {
    // volumes are added up until the requested volume is reached
    currVol = atof(json_string_value(json_array_get(json_array_get(branch, i), 1)));
    currPrice = atof(json_string_value(json_array_get(json_array_get(branch, i), 0)));
    totVol += currVol;
    if (totVol >= volume * params.orderBookFactor)
      break;
  }

  return currPrice;
}

json_t* Kraken::authRequest(Parameters &params, std::string request, std::string options)
{
  // create nonce and POST data
  static uint64_t nonce = time(nullptr) * 4;
  std::string post_data = "nonce=" + std::to_string(++nonce);
  if (!options.empty())
    post_data += "&" + options;

  // Message signature using HMAC-SHA512 of (URI path + SHA256(nonce + POST data))
  // and base64 decoded secret API key
  auto sig_size = request.size() + SHA256_DIGEST_LENGTH;
  std::vector<uint8_t> sig_data(sig_size);
  copy(std::begin(request), std::end(request), std::begin(sig_data));

  std::string payload_for_signature = std::to_string(nonce) + post_data;
  SHA256((uint8_t *)payload_for_signature.c_str(), payload_for_signature.size(),
         &sig_data[sig_size - SHA256_DIGEST_LENGTH]);

  std::string decoded_key = base64_decode(params.krakenSecret);
  uint8_t *hmac_digest = HMAC(EVP_sha512(),
                              decoded_key.c_str(), decoded_key.length(),
                              sig_data.data(), sig_data.size(), NULL, NULL);
  std::string api_sign_header = base64_encode(hmac_digest, SHA512_DIGEST_LENGTH);
  // cURL header
  std::array<std::string, 2> headers{
      "API-KEY:" + params.krakenApi,
      "API-Sign:" + api_sign_header,
  };

  // cURL request
  auto &exchange = queryHandle(params);
  return exchange.postRequest(request,
                              make_slist(std::begin(headers), std::end(headers)),
                              post_data);
}

void Kraken::testKraken()
{

  Parameters params("bird.conf");
  params.logFile = new std::ofstream("./test.log", std::ofstream::trunc);

  // std::string orderId;

  // std::cout << "Current value LEG1_LEG2 bid: " << getQuote(params).bid() << std::endl;
  // std::cout << "Current value LEG1_LEG2 ask: " << getQuote(params).ask() << std::endl;
  // std::cout << "Current balance BTC: " << getAvail(params, "btc") << std::endl;
  // std::cout << "Current balance USD: " << getAvail(params, "usd") << std::endl;
  // std::cout << "Current balance ETH: " << getAvail(params, "eth") << std::endl;
  // std::cout << "Current balance XMR: " << getAvail(params, "xmr") << std::endl;
  // std::cout << "current bid limit price for .09 units: " << getLimitPrice(params, 0.09, true) << std::endl;
  // std::cout << "Current ask limit price for .09 units: " << getLimitPrice(params, 0.09, false) << std::endl;
  //std::cout << "Sending buy order for 0.01 XMR @ $100 USD - TXID: " << std::endl;
  //orderId = sendLongOrder(params, "buy", 0.01, 100);
  //std::cout << orderId << std::endl;
  ///// if you don't wait bittrex won't recognize order for iscomplete
  //sleep(5);
  //std::cout << "Buy Order is complete: " << isOrderComplete(params, orderId) << std::endl;

  //std::cout << "Sending Short XMR order for 0.177 XMR @BID! USD: ";
  //orderId = sendShortOrder(params,"sell",0.133, getLimitPrice(params,0.133,true));
  //std::cout << orderId << std::endl;
  //std::cout << "Closing Short XMR order for .09 - TXID: ";
  //orderId = sendShortOrder(params, "buy", 0.046, getLimitPrice(params,0.046, false));
  //std::cout << orderId  << std::endl;

  //vanilla sell orders below
  //std::cout << "Buy order is complete: " << isOrderComplete(params, orderId) << std::endl;
  //std::cout << "Sending sell order for 0.01 XMR @ 5000 USD - TXID: " << std::endl ;
  //orderId = sendLongOrder(params, "sell", 0.01, 5000);
  //std:: cout << orderId << std::endl;
  //std::cout << "Sell order is complete: " << isOrderComplete(params, orderId) << std::endl;
  //std::cout << "Active Position: " << getActivePos(params);
}

} //namespace NSExchange

