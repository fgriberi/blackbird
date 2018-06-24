/*
 * @file     cexio.h
 * @brief    Provides the interface to for exchanging using Cexio.
 */

#ifndef CEXIO_H
#define CEXIO_H

#include <string>
#include <sstream>
#include "iExchange.h"
#include "quote_t.h"
#include "unique_json.hpp"
#include "utils/restapi.h"


namespace NSExchange
{

/**
 * @brief Cexio is an implementation of IExchange interface. https://cex.io/
 */
class Cexio : public IExchange
{
	virtual ~Cexio() = default;

	quote_t getQuote(Parameters &params, std::string pair) override final;

	double getAvail(Parameters& params, std::string currency) override final;

	std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair) override final;

	std::string sendShortOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair) override final;

	std::string sendOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair);

	std::string openPosition(Parameters& params,std::string direction, double quantity, double price, std::string pair);

	std::string closePosition(Parameters& params, std::string pair);

	bool isOrderComplete(Parameters& params, std::string orderId) override final;

	double getActivePos(Parameters& params, std::string currency) override final;

	double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair) override final;

	static json_t* authRequest(Parameters &, std::string, std::string);
	static RestApi& queryHandle(Parameters &params);
	static json_t* checkResponse(std::ostream &logFile, json_t *root);
	std::string getMatchingPair(std::string pair);
	std::string getTickerPair(std::string pair);
	int getMatchingSymbols(std::string pair, std::string symbols[]);

	void testCexio();

};

} //namespace NSExchange
#endif /* CEXIO_H */
