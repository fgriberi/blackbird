#ifndef CEXIO_H
#define CEXIO_H

#include "quote_t.h"
#include <string>
#include <sstream>

struct json_t;
struct Parameters;

namespace Cexio {

quote_t getQuote(Parameters &params, std::string pair);

double getAvail(Parameters& params, std::string currency);

std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair);

std::string sendShortOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair);

std::string sendOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair);

std::string openPosition(Parameters& params,std::string direction, double quantity, double price, std::string pair);

std::string closePosition(Parameters& params, std::string pair);


bool isOrderComplete(Parameters& params, std::string orderId);

double getActivePos(Parameters& params, std::string currency);

double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair);

void testCexio();

}

#endif
