/*
 * @file     itbit.h
 * @brief    Provides the interface to for exchanging using itBit.
 */

#ifndef ITBIT_H
#define ITBIT_H

#include <string>
#include <sstream>
#include "iExchange.h"
#include "quote_t.h"
#include "utils/restapi.h"

namespace NSExchange
{

/**
 * @brief ItBit is an implementation of IExchange interface. https://www.itbit.com/
 */
class ItBit : public IExchange
{
	virtual ~ItBit() = default;

	quote_t getQuote(Parameters &params, std::string pair) override final;

	double getAvail(Parameters& params, std::string currency) override final;

	double getActivePos(Parameters& params, std::string currency) override final;

	double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair) override final;

	std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair) override final;

	std::string sendShortOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair) override final;

	bool isOrderComplete(Parameters& params, std::string orderId) override final;

	static RestApi& queryHandle(Parameters &params);
};

} //namespace NSExchange
#endif /* ITBIT_H */
