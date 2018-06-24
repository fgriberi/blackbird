/*
 * @file     binance.h
 * @brief    Provides the interface to for exchanging using Binance.
 */

#ifndef BINANCE_H
#define BINANCE_H

#include <string>
#include "iExchange.h"
#include "utils/restapi.h"
#include "unique_json.hpp"

namespace NSExchange
{

/**
 * @brief Binance is an implementation of IExchange interface. https://www.binance.com/
 */
class Binance : public IExchange
{
	virtual ~Binance() = default;

	quote_t getQuote(Parameters &params, std::string pair) override final;

	double getAvail(Parameters &params, std::string currency) override final;

	std::string sendLongOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair) override final;

	std::string sendShortOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair) override final;

	bool isOrderComplete(Parameters &params, std::string orderId) override final;

	double getActivePos(Parameters &params, std::string currency) override final;

	double getLimitPrice(Parameters &params, double volume, bool isBid, std::string pair) override final;

	static json_t* authRequest(Parameters &, std::string, std::string, std::string);

	static std::string getSignature(Parameters &params, std::string payload);

	static RestApi& queryHandle(Parameters &params);

	std::string getMatchingPair(std::string pair);

	void testBinance();
};

} //namespace NSExchange
#endif /* BINANCE_H */