/**
 * @file iExchange.h
 * @brief IExhange provides the interface for exchanging.
 */

#ifndef IEXCHANGE_H
#define IEXCHANGE_H

#include <string>
#include "quote_t.h"
#include "parameters.h"

// TODO: check const-correctness
namespace NSExchange
{

/**
 * @brief Interface for exchanging
 */
struct IExchange
{
    virtual ~IExchange() = default;

	virtual quote_t getQuote(Parameters &params, std::string pair) = 0;

	virtual double getAvail(Parameters &params, std::string currency) = 0;

	virtual std::string sendLongOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair) = 0;

	virtual std::string sendShortOrder(Parameters &params, std::string direction, double quantity, double price, std::string pair) = 0;

	virtual bool isOrderComplete(Parameters &params, std::string orderId) = 0;

	virtual double getActivePos(Parameters &params, std::string currency) = 0;

	virtual double getLimitPrice(Parameters &params, double volume, bool isBid, std::string pair) = 0;
};

} //namespace NSExchange
#endif /* IEXCHANGE_H */
