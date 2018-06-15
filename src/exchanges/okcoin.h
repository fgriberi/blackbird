#ifndef OKCOIN_H
#define OKCOIN_H

#include "quote_t.h"
#include <string>

struct json_t;
struct Parameters;

namespace OKCoin {

quote_t getQuote(Parameters &params, std::string pair);

double getAvail(Parameters& params, std::string currency);

std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair);

std::string sendShortOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair);

bool isOrderComplete(Parameters& params, std::string orderId);

double getActivePos(Parameters& params, std::string currency);

double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair);

json_t* authRequest(Parameters& params, std::string url, std::string signature, std::string content);

void getBorrowInfo(Parameters& params);

int borrowBtc(Parameters& params, double amount);

void repayBtc(Parameters& params, int borrowId);

}

#endif
