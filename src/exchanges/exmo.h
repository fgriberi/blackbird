/*
 * @file     exmo.h
 * @brief    Provides the interface to for exchanging using Exmo.
 */

#ifndef EXMO_H
#define EXMO_H

#include <string>
#include <sstream>
#include <algorithm>
#include "iExchange.h"
#include "quote_t.h"
#include "utils/restapi.h"
#include "unique_json.hpp"

namespace NSExchange
{

/**
 * @brief Exmo is an implementation of IExchange interface. https://exmo.com
 */
class Exmo : public IExchange
{
	virtual ~Exmo() = default;

	quote_t getQuote(Parameters &params, std::string pair) override final;

	double getAvail(Parameters& params, std::string currency) override final;

	std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair) override final;

	// TODO multi currency support
	//std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair = "btc_usd");

	std::string sendShortOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair) override final;

	bool isOrderComplete(Parameters& params, std::string orderId) override final;

	double getActivePos(Parameters& params, std::string currency) override final;

	double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair) override final;

	void testExmo();

	static json_t* authRequest(Parameters &, const char* URL_Request, std::string URL_Options = "");
	static std::string getSignature(Parameters &, std::string);
	static RestApi& queryHandle(Parameters &params);
};

} //namespace NSExchange
#endif /* EXMO_H */
