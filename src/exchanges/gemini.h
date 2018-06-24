/*
 * @file     gemini.h
 * @brief    Provides the interface to for exchanging using Gemini.
 */

#ifndef GEMINI_H
#define GEMINI_H

#include <string>
#include <sstream>
#include "iExchange.h"
#include "quote_t.h"
#include "utils/restapi.h"
#include "unique_json.hpp"

namespace NSExchange
{

/**
 * @brief Gemini is an implementation of IExchange interface. https://gemini.com/
 */
class Gemini : public IExchange
{
	virtual ~Gemini() = default;

	quote_t getQuote(Parameters &params, std::string pair) override final;

	double getAvail(Parameters& params, std::string currency) override final;

	std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair) override final;

	std::string sendShortOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair) override final;

	bool isOrderComplete(Parameters& params, std::string orderId) override final;

	double getActivePos(Parameters& params, std::string currency) override final;

	double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair) override final;

	json_t* authRequest(Parameters& params, std::string url, std::string request, std::string options);

	static RestApi& queryHandle(Parameters &params);
};

} //namespace NSExchange
#endif /* GEMINI_H */

