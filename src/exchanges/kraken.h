/*
 * @file     kraken.h
 * @brief    Provides the interface to for exchanging using Kraken.
 */

#ifndef KRAKEN_H
#define KRAKEN_H

#include <string>
#include "iExchange.h"
#include "quote_t.h"
#include "unique_json.hpp"
#include "utils/restapi.h"

namespace NSExchange
{

/**
 * @brief Kraken is an implementation of IExchange interface. https://www.kraken.com/
 */
class Kraken : public IExchange
{
    virtual ~Kraken() = default;

	quote_t getQuote(Parameters& params, std::string pair) override final;

	double getAvail(Parameters& params, std::string currency) override final;

	std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair) override final;

	std::string sendShortOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair) override final;

	std::string sendOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair);

	bool isOrderComplete(Parameters& params, std::string orderId) override final;

	double getActivePos(Parameters& params, std::string currency) override final;

	double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair) override final;

	json_t* authRequest(Parameters& params, std::string request, std::string options = "");

	static RestApi& queryHandle(Parameters &params);
	std::string getMatchingPair(std::string pair);

	void testKraken();
};

} //namespace NSExchange
#endif /* KRAKEN_H */
