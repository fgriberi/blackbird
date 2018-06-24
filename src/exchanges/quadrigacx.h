/*
 * @file     quadrigacx.h
 * @brief    Provides the interface to for exchanging using QuadrigaCX.
 */

#ifndef QUADRIGACX_H
#define QUADRIGACX_H

#include <string>
#include <sstream>
#include "iExchange.h"
#include "quote_t.h"
#include "utils/restapi.h"
#include "unique_json.hpp"

namespace NSExchange
{

/**
 * @brief QuadrigaCX is an implementation of IExchange interface that use QuadrigaCX exchange
 *        URL: https://www.quadrigacx.com
 */
class QuadrigaCX : public IExchange
{
	virtual ~QuadrigaCX() = default;

	quote_t getQuote(Parameters &params, std::string pair) override final;

	double getAvail(Parameters& params, std::string currency) override final;

	std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair) override final;

	std::string sendShortOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair) override final;

	bool isOrderComplete(Parameters& params, std::string orderId) override final;

	double getActivePos(Parameters& params, std::string currency) override final;

	double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair) override final;

	void testQuadriga();

	static std::string getSignature(Parameters& params, const uint64_t nonce);
	static json_t* authRequest(Parameters& params, std::string request, json_t * options = nullptr);
	static RestApi& queryHandle(Parameters &params);

};

} //namespace NSExchange
#endif /* QUADRIGACX_H */
