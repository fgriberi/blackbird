#ifndef WEX_H
#define WEX_H

#include "quote_t.h"
#include <string>

struct Parameters;

namespace WEX {

quote_t getQuote(Parameters &params, std::string pair);

double getAvail(Parameters& params, std::string currency);

std::string sendLongOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair);

bool isOrderComplete(Parameters& params, std::string orderId);

double getActivePos(Parameters& params, std::string currency);

double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair);

}

#endif
