/*
 * @file     gdax.h
 * @brief    Provides the interface to for exchanging using GDAX.
 */

#ifndef GDAX_H
#define GDAX_H

#include <string>
#include "iExchange.h"
#include "quote_t.h"
#include "utils/restapi.h"
#include "unique_json.hpp"

namespace NSExchange
{

/**
 * @brief GDAX is an implementation of IExchange interface. https://www.gdax.com
 */
class GDAX : public IExchange
{
	virtual ~GDAX() = default;

	quote_t getQuote(Parameters &params, std::string pair) override final;

	double getAvail(Parameters& params, std::string currency) override final;

	double getActivePos(Parameters& params, std::string currency) override final;

	double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair) override final;

	std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair) override final;

	std::string sendShortOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair) override final;

	bool isOrderComplete(Parameters& params, std::string orderId) override final;

	json_t* authRequest(Parameters& params, std::string method, std::string request,const std::string &options);

	std::string gettime();

	void testGDAX();

	static RestApi& queryHandle(Parameters &params);
};

} //namespace NSExchange
#endif /* GDAX_H */
