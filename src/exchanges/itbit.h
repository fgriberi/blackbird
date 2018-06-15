#ifndef ITBIT_H
#define ITBIT_H

#include "quote_t.h"
#include <string>

struct json_t;
struct Parameters;

namespace ItBit {

quote_t getQuote(Parameters &params, std::string pair);

double getAvail(Parameters& params, std::string currency);

double getActivePos(Parameters& params, std::string currency);

double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair);

}

#endif
