#include "itbit.h"
#include "parameters.h"
#include "curl_fun.h"
#include "utils/restapi.h"
#include "unique_json.hpp"


namespace NSExchange {

RestApi& ItBit::queryHandle(Parameters &params)
{
  static RestApi query ("https://api.itbit.com",
                        params.cacert.c_str(), *params.logFile);
  return query;
}

quote_t ItBit::getQuote(Parameters &params, std::string pair)
{
  auto &exchange = queryHandle(params);
  unique_json root { exchange.getRequest("/v1/markets/XBTUSD/ticker") };

  const char *quote = json_string_value(json_object_get(root.get(), "bid"));
  auto bidValue = quote ? std::stod(quote) : 0.0;

  quote = json_string_value(json_object_get(root.get(), "ask"));
  auto askValue = quote ? std::stod(quote) : 0.0;

  return std::make_pair(bidValue, askValue);
}

double ItBit::getAvail(Parameters& params, std::string currency)
{
  // TODO
  return 0.0;
}

double ItBit::getActivePos(Parameters& params, std::string currency)
{
  // TODO
  return 0.0;
}

double ItBit::getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair)
{
  // TODO
  return 0.0;
}

std::string ItBit::sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair)
{
  //TODO
  return "0";
}

std::string ItBit::sendShortOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair)
{
  //TODO
  return "0";
}

bool ItBit::isOrderComplete(Parameters& params, std::string orderId)
{
  //TODO
  return false;
}

} //namespace NSExchange
