/*
 * @file  mop.h
 * @brief mediator file
 */

#ifndef MOP_H
#define MOP_H

#include <string>
#include "parameters.h"
#include "exchanges/iExchange.h"
#include "exchanges/bitfinex.h"
#include "exchanges/okcoin.h"
#include "exchanges/bitstamp.h"
#include "exchanges/gemini.h"
#include "exchanges/kraken.h"
#include "exchanges/quadrigacx.h"
#include "exchanges/itbit.h"
#include "exchanges/btce.h"
#include "exchanges/wex.h"
#include "exchanges/poloniex.h"
#include "exchanges/gdax.h"
#include "exchanges/exmo.h"
#include "exchanges/cexio.h"
#include "exchanges/bittrex.h"
#include "exchanges/binance.h"
#include "bitcoin.h"

namespace NSExchange
{

/** @brief Represents all exchanges availables */
typedef std::vector<IExchange*> Exchanges;

} //namespace NSExchange

namespace NSMop
{

typedef std::vector<Bitcoin> BitcoinContainer;

/**
 * @brief This structure contains the balance of both exchanges,
 *        *before* and *after* an arbitrage trade.
 *        This is used to compute the performance of the trade,
 *        by comparing the balance before and after the trade.
 */
struct Balance {
  	double leg1, leg2;
	double leg1After, leg2After;
};

/** @brief Represents all exchange balances */
typedef std::vector<Balance> Balances;

/** @brief Converts string to lower string */
std::string str_tolower(std::string s);

/**
 * @brief Does some verifications about the parameters

 * @param params[in]: hold the blackbird parameters configuration
 */
void demoModeAssert(const Parameters& params);

/**
 * @brief Initializes all exchanges configured in the Blackbird configuration file
 *
 * @param params[in]: hold the exchanges information
 * @param exchanges[out]: to fill with the exchanges pointers
 * @param dbTableName[out]: to fill with the availables exchanges names
 */
void initializeExchages(Parameters& params, NSExchange::Exchanges& exchanges, std::string* dbTableName);

/**
 * @brief Gets the the balances from every exchange. This is only done when not in Demo mode.
 *
 * @param params[in]: hold the exchanges information
 * @param params[out]: hold the exchanges information
 * @param balance[out]: to fill with the exchange balances
 */
void getExchangeBalances(const NSExchange::Exchanges& exchanges, Parameters& params, NSMop::Balances& balance);

/**
 * @brief Writes the current balances into the log file
 *
 * @param balance[in]: the exchanges balance container
 * @param params[in]: hold the exchanges information
 * @param numExch[in]: amount of exchanges availables
 * @param logFile[out]: the log file reference
 */
void logCurrentBalances(const Parameters& params, const NSMop::Balances& balance, const size_t numExch,
						std::ofstream& logFile);

} //namspace NSMop
#endif /* MOP_H */
