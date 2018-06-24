/*
 * @file     bitstamp.h
 * @brief    Provides the interface to for exchanging using Bitstamp.
 */

#ifndef BITSTAMP_H
#define BITSTAMP_H

#include <string>
#include "iExchange.h"
#include "quote_t.h"
#include "unique_json.hpp"
#include "utils/restapi.h"

namespace NSExchange
{

/**
 * @brief Bitstamp is an implementation of IExchange interface. https://www.bitstamp.net/
 */
class Bitstamp : public IExchange
{
	virtual ~Bitstamp() = default;

	quote_t getQuote(Parameters& params, std::string pair) override final;

	double getAvail(Parameters& params, std::string currency) override final;

	std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair) override final;

	std::string sendShortOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair) override final;

	bool isOrderComplete(Parameters& params, std::string orderId) override final;

	double getActivePos(Parameters& params, std::string currency) override final;

	double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair) override final;

	static json_t* authRequest(Parameters &, std::string, std::string);
	static RestApi& queryHandle(Parameters &params);
	static json_t* checkResponse(std::ostream &logFile, json_t *root);
	std::string getMatchingPair(std::string pair);
};

} //namespace NSExchange
#endif /* BITSTAMP_H */
