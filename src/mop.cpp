#include <algorithm>
#include "mop.h"
#include "db_fun.h"
#include "time_fun.h"

namespace NSMop
{

std::string str_tolower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower
                // static_cast<int(*)(int)>(std::tolower)         // wrong
                // [](int c){ return std::tolower(c); }           // wrong
                // [](char c){ return std::tolower(c); }          // wrong
                  //  [](unsigned char c){ return std::tolower(c); } // correct
                  );
    return s;
}


void demoModeAssert(const Parameters& params)
{
  	if (!params.isDemoMode) {
	    if (!params.useFullExposure) {
	      	if (params.testedExposure < 10.0 && (params.leg2.compare("USD") == 0 || params.leg2.compare("EUR") == 0))
	      	{
	        	// TODO do the same check for other currencies. Is there a limi?
	        	std::cout << "ERROR: Minimum USD needed: $10.00" << std::endl;
	        	std::cout << "       Otherwise some exchanges will reject the orders\n" << std::endl;
	        	exit(EXIT_FAILURE);
	      	}
	      	if (params.testedExposure > params.maxExposure)
	      	{
	        	std::cout << "ERROR: Test exposure (" << params.testedExposure << ") is above max exposure (" << params.maxExposure << ")\n" << std::endl;
	        	exit(EXIT_FAILURE);
	      	}
	    }
  	}
}

void initializeExchages(Parameters& params, NSExchange::Exchanges& exchanges, std::string* dbTableName)
{
	size_t index = 0;
	if (params.bitfinexEnable && (params.bitfinexApi.empty() == false || params.isDemoMode))
	{
		params.addExchange("Bitfinex", params.bitfinexFees, true, true);
		exchanges.push_back(new NSExchange::Bitfinex());
		dbTableName[index] = "bitfinex";
		createTable(dbTableName[index], params);
		++index;
	}
	if (params.okcoinEnable && (params.okcoinApi.empty() == false || params.isDemoMode))
	{
		params.addExchange("OKCoin", params.okcoinFees, false, true);
		exchanges.push_back(new NSExchange::OKCoin());
		dbTableName[index] = "okcoin";
		createTable(dbTableName[index], params);
		++index;
	}
	if (params.bitstampEnable && (params.bitstampClientId.empty() == false || params.isDemoMode))
	{
		params.addExchange("Bitstamp", params.bitstampFees, false, true);
		exchanges.push_back(new NSExchange::Bitstamp());
		dbTableName[index] = "bitstamp";
		createTable(dbTableName[index], params);
		++index;
	}
	if (params.geminiEnable && (params.geminiApi.empty() == false || params.isDemoMode))
	{
		params.addExchange("Gemini", params.geminiFees, false, true);
		exchanges.push_back(new NSExchange::Gemini());
		dbTableName[index] = "gemini";
		createTable(dbTableName[index], params);
		++index;
	}
	if (params.krakenEnable && (params.krakenApi.empty() == false || params.isDemoMode))
	{
		params.addExchange("Kraken", params.krakenFees, true, true);
		exchanges.push_back(new NSExchange::Kraken());
		dbTableName[index] = "kraken";
		createTable(dbTableName[index], params);
		++index;
	}
	if (params.itbitEnable && (params.itbitApi.empty() == false || params.isDemoMode))
	{
		params.addExchange("ItBit", params.itbitFees, false, false);
		exchanges.push_back(new NSExchange::ItBit());
		dbTableName[index] = "itbit";
		createTable(dbTableName[index], params);
		++index;
	}
	if (params.btceEnable && (params.btceApi.empty() == false || params.isDemoMode))
	{
		params.addExchange("BTC-e", params.btceFees, false, true);
		exchanges.push_back(new NSExchange::BTCe());
		dbTableName[index] = "btce";
		createTable(dbTableName[index], params);
		++index;
	}
	if (params.wexEnable && (params.wexApi.empty() == false || params.isDemoMode))
	{
		params.addExchange("WEX", params.wexFees, false, true);
		exchanges.push_back(new NSExchange::WEX());
		dbTableName[index] = "wex";
		createTable(dbTableName[index], params);
		++index;
	}
	if (params.poloniexEnable && (params.poloniexApi.empty() == false || params.isDemoMode))
	{
		params.addExchange("Poloniex", params.poloniexFees, true, false);
		exchanges.push_back(new NSExchange::Poloniex());
		dbTableName[index] = "poloniex";
		createTable(dbTableName[index], params);
		++index;
	}
	if (params.gdaxEnable && (params.gdaxApi.empty() == false || params.isDemoMode))
	{
		params.addExchange("GDAX", params.gdaxFees, false, true);
		exchanges.push_back(new NSExchange::GDAX());
		dbTableName[index] = "gdax";
		createTable(dbTableName[index], params);
		++index;
	}
	if (params.quadrigaEnable && (params.quadrigaApi.empty() == false || params.isDemoMode))
	{
		params.addExchange("QuadrigaCX", params.quadrigaFees, false, true);
		exchanges.push_back(new NSExchange::QuadrigaCX());
		dbTableName[index] = "quadriga";
		createTable(dbTableName[index], params);
		++index;
	}
	if (params.exmoEnable && (params.exmoApi.empty() == false || params.isDemoMode))
	{
		params.addExchange("Exmo", params.exmoFees, false, true);
		exchanges.push_back(new NSExchange::Exmo());
		dbTableName[index] = "exmo";
		createTable(dbTableName[index], params);
		++index;
	}
	if (params.cexioEnable && (params.cexioApi.empty() == false || params.isDemoMode))
	{
		params.addExchange("Cexio", params.cexioFees, false, true);
		exchanges.push_back(new NSExchange::Cexio());
		dbTableName[index] = "cexio";
		createTable(dbTableName[index], params);
		++index;
	}
	if (params.bittrexEnable && (params.bittrexApi.empty() == false || params.isDemoMode))
	{
		params.addExchange("Bittrex", params.bittrexFees, false, true);
		exchanges.push_back(new NSExchange::Bittrex());
		dbTableName[index] = "bittrex";
		createTable(dbTableName[index], params);
		++index;
	}
	if (params.binanceEnable && (params.binanceApi.empty() == false || params.isDemoMode))
	{
		params.addExchange("Binance", params.binanceFees, false, true);
		exchanges.push_back(new NSExchange::Binance());
		dbTableName[index] = "binance";
		createTable(dbTableName[index], params);
		++index;
	}

	// We need at least two exchanges to run Blackbird
	if (index < 2)
	{
		std::cout << "ERROR: Blackbird needs at least two Bitcoin exchanges. Please edit the config.json file to add new exchanges\n" << std::endl;
		exit(EXIT_FAILURE);
	}
}

void getExchangeBalances(const NSExchange::Exchanges& exchanges, Parameters& params, NSMop::Balances& balance)
{
	if (!params.isDemoMode) {
		std::transform(exchanges.begin(), exchanges.end(),
					   balance.begin(),
					   [&params](NSExchange::IExchange* exchange)
					   {
							NSMop::Balance tmp {};

							std::string leg1Lower = NSMop::str_tolower(params.leg1);
							std::string leg2Lower = NSMop::str_tolower(params.leg2);

							tmp.leg1 = exchange->getAvail(params, leg1Lower);
							tmp.leg2 = exchange->getAvail(params, leg2Lower);
							return tmp;
					   });
	}
}

void logCurrentBalances(const Parameters& params, const NSMop::Balances& balance, const size_t numExch,
						std::ofstream& logFile)
{
  logFile << "[ Current balances ]" << std::endl;
  for (size_t i(0u); i < numExch; ++i) {
    logFile << "   " << params.exchName[i] << ":\t";
    if (params.isDemoMode) {
      logFile << "n/a (demo mode)" << std::endl;
    } else if (!params.isImplemented[i]) {
      logFile << "n/a (API not implemented)" << std::endl;
    } else {
      logFile << std::setprecision(2) << balance[i].leg2 << " " << params.leg2 << "\t"
              << std::setprecision(6) << balance[i].leg1 << " " << params.leg1 << std::endl;
    }
    if (balance[i].leg1 > 0.0050) { // FIXME: hard-coded number
      logFile << "ERROR: All " << params.leg1 << " accounts must be empty before starting Blackbird" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  logFile << std::endl;
  logFile << "[ Cash exposure ]\n";
  if (params.isDemoMode) {
    logFile << "   No cash - Demo mode\n";
  } else {
    if (params.useFullExposure) {
      logFile << "   FULL exposure used!\n";
    } else {
      logFile << "   TEST exposure used\n   Value: "
              << std::setprecision(2) << params.testedExposure << '\n';
    }
  }
  logFile << std::endl;
}

} //namspace NSMop