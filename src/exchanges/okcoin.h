/*
 * @file     okcoin.h
 * @brief    Provides the interface to for exchanging using OkCoin.
 */

#ifndef OKCOIN_H
#define OKCOIN_H

#include <string>
#include "iExchange.h"
#include "quote_t.h"
#include "utils/restapi.h"
#include "unique_json.hpp"

namespace NSExchange
{

/**
 * @brief OKCoin is an implementation of IExchange interface. https://www.okcoin.com/
 */
class OKCoin : public IExchange
{
	virtual ~OKCoin() = default;

	quote_t getQuote(Parameters &params, std::string pair) override final;

	double getAvail(Parameters& params, std::string currency) override final;

	std::string sendLongOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair) override final;

	std::string sendShortOrder(Parameters& params, std::string direction, double quantity, double price, std::string pair) override final;

	bool isOrderComplete(Parameters& params, std::string orderId) override final;

	double getActivePos(Parameters& params, std::string currency) override final;

	double getLimitPrice(Parameters& params, double volume, bool isBid, std::string pair) override final;

	json_t* authRequest(Parameters& params, std::string url, std::string signature, std::string content);

	void getBorrowInfo(Parameters& params);

	int borrowBtc(Parameters& params, double amount);

	void repayBtc(Parameters& params, int borrowId);

	static RestApi& queryHandle(Parameters &params);
};

} //namespace NSExchange
#endif /* OKCOIN_H */

