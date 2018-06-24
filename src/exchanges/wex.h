/*
 * @file     wex.h
 * @brief    Provides the interface to for exchanging using WEX.
 */

#ifndef WEX_H
#define WEX_H

#include <string>
#include "iExchange.h"
#include "quote_t.h"
#include "utils/restapi.h"
#include "unique_json.hpp"

namespace NSExchange
{

/**
 * @brief WEX is an implementation of IExchange interface. https://wex.nz
 */
class WEX : public IExchange
{
	virtual ~WEX() = default;

	quote_t getQuote(Parameters &params, std::string pair) override final;

	double getAvail(Parameters& params, std::string currency) override final;

	std::string sendLongOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair) override final;

	std::string sendShortOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair) override final;

	bool isOrderComplete(Parameters& params, std::string orderId) override final;

	double getActivePos(Parameters& params, std::string currency) override final;

	double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair) override final;

	static json_t* authRequest(Parameters &, const char *, const std::string & = "");
	static json_t* adjustResponse(json_t *);
	static RestApi& queryHandle(Parameters &params);
	static json_t* checkResponse(std::ostream &logFile, json_t *root);
};

} //namespace NSExchange
#endif /* WEX_H */
