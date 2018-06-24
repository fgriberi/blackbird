/*
 * @file     bitfinex.h
 * @brief    Provides the interface to for exchanging using Bitfinex
 */

#ifndef BITFINEX_H
#define BITFINEX_H

#include <string>
#include "iExchange.h"
#include "quote_t.h"
#include "utils/restapi.h"
#include "unique_json.hpp"

namespace NSExchange
{

/**
 * @brief Bitfinex is an implementation of IExchange interface. https://www.bitfinex.com/
 */
class Bitfinex : public IExchange
{
    virtual ~Bitfinex() = default;

	quote_t getQuote(Parameters &params, std::string pair) override final;

	double getAvail(Parameters& params, std::string currency) override final;

	std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair) override final;

	std::string sendShortOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair) override final;

	std::string sendOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair);

	bool isOrderComplete(Parameters& params, std::string orderId) override final;

	double getActivePos(Parameters& params, std::string currency) override final;

	double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair) override final;

	json_t* authRequest(Parameters &params, std::string request, std::string options);

	static RestApi& queryHandle(Parameters &params);

	static json_t* checkResponse(std::ostream &logFile, json_t *root);

	std::string getMatchingPair(std::string pair);
};

} //namespace NSExchange
#endif /* BITFINEX_H */
