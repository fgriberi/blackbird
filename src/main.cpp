#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cmath>
#include <algorithm>
#include <string>

#include "bitcoin.h"
#include "result.h"
#include "time_fun.h"
#include "curl_fun.h"
#include "db_fun.h"
#include "parameters.h"
#include "check_entry_exit.h"
#include "utils/send_email.h"
#include "getpid.h"
#include "tradeOutput.h"
#include "mop.h"

using std::this_thread::sleep_for;
using millisecs = std::chrono::milliseconds;
using secs = std::chrono::seconds;

// 'main' function.
// Blackbird doesn't require any arguments for now.
int main(int argc, char** argv)
{
  std::cout << "Blackbird Bitcoin Arbitrage" << std::endl;
  std::cout << "DISCLAIMER: USE THE SOFTWARE AT YOUR OWN RISK\n" << std::endl;

  // Replaces the C++ global locale with the user-preferred locale
  std::locale mylocale("");

  // Loads all the parameters
  Parameters params("blackbird.conf");
  NSMop::demoModeAssert(params);

  // Connects to the SQLite3 database.
  // This database is used to collect bid and ask information
  // from the exchanges. Not really used for the moment, but
  // would be useful to collect historical bid/ask data.
  if (createDbConnection(params) != 0) {
    std::cerr << "ERROR: cannot connect to the database \'" << params.dbFile << "\'\n" << std::endl;
    exit(EXIT_FAILURE);
  }

  if ((params.leg1.compare("BTC") == 0 && params.leg1.compare("ETH") == 0) || (params.leg2.compare("USD") == 0 && params.leg1.compare("EUR") == 0)) {
    std::cout << "ERROR: Valid currency pair are only BTC/USD | BTC/EUR | ETH/USD | ETH/EUR for now.\n" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::string dbTableName[14];
  NSExchange::Exchanges exchanges;
  NSMop::initializeExchages(params, exchanges, dbTableName);

  // Creates the CSV file that will collect the trade results
  const std::string currDateTime {printDateTimeFileName()};
  const NSOutput::FileName csvFileName {"output/blackbird_result_" + currDateTime + ".csv"};
  NSOutput::TradeOutput csvFile(csvFileName);

  // Creates the log file where all events will be saved
  std::string logFileName = "output/blackbird_log_" + currDateTime + ".log";
  std::ofstream logFile(logFileName, std::ofstream::trunc);
  logFile.imbue(mylocale);
  logFile.precision(2);
  logFile << std::fixed;
  params.logFile = &logFile;

  // Log file header
  logFile << "--------------------------------------------" << std::endl;
  logFile << "|   Blackbird Bitcoin Arbitrage Log File   |" << std::endl;
  logFile << "--------------------------------------------\n" << std::endl;
  logFile << "Blackbird started on " << printDateTime() << "\n" << std::endl;
  logFile << "Connected to database \'" << params.dbFile << "\'\n" << std::endl;
  if (params.isDemoMode) {
    logFile << "Demo mode: trades won't be generated\n" << std::endl;
  }

  // Shows which pair we are trading
  logFile << "Pair traded: " << params.leg1 << "/" << params.leg2 << "\n" << std::endl;

  std::cout << "Log file generated: " << logFileName << "\nBlackbird is running... (pid " << getpid() << ")\n" << std::endl;
  int numExch = params.nbExch();

  // The btcVec vector contains details about every exchange, like fees, as specified in bitcoin.h
  NSMop::BitcoinContainer btcVec;
  btcVec.reserve(numExch);
  // Creates a new Bitcoin structure within btcVec for every exchange we want to trade on
  for (int i = 0; i < numExch; ++i) {
    btcVec.push_back(Bitcoin(i, params.exchName[i], params.fees[i], params.canShort[i], params.isImplemented[i]));
  }

  // Inits cURL connections
  params.curl = curl_easy_init();

  // Shows the spreads
  logFile << "[ Targets ]\n"
          << std::setprecision(2)
          << "   Spread Entry:  " << params.spreadEntry * 100.0 << "%\n"
          << "   Spread Target: " << params.spreadTarget * 100.0  << "%\n";

  // SpreadEntry and SpreadTarget have to be positive,
  // Otherwise we will loose money on every trade.
  if (params.spreadEntry <= 0.0) {
    logFile << "   WARNING: Spread Entry should be positive" << std::endl;
  }
  if (params.spreadTarget <= 0.0) {
    logFile << "   WARNING: Spread Target should be positive" << std::endl;
  }
  logFile << std::endl;
  logFile << "[ Current balances ]" << std::endl;

  //*************************************************************************************
  // Gets the the balances from every exchange. This is only done when not in Demo mode.
  //*************************************************************************************
  NSMop::Balances balance(numExch);
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
                   } );
  }

  std::string pair = NSMop::str_tolower(params.leg1 + params.leg2);
  // logFile << "pair = " << pair;

  // Checks for a restore.txt file, to see if the program exited with an open position.
  Result res;
  res.reset();
  bool inMarket = res.loadPartialResult("restore.txt");

  NSMop::logCurrentBalances(params, balance, numExch, inMarket, logFile);

  // Code implementing the loop function, that runs every 'Interval' seconds.
  time_t rawtime = time(nullptr);
  tm timeinfo = *localtime(&rawtime);

  // Waits for the next 'interval' seconds before starting the loop
  while ((int)timeinfo.tm_sec % params.interval != 0) {
    sleep_for(millisecs(100));
    time(&rawtime);
    timeinfo = *localtime(&rawtime);
  }
  if (!params.verbose) {
    logFile << "Running..." << std::endl;
  }

  int resultId = 0;
  unsigned currIteration = 0;
  bool stillRunning = true;
  time_t currTime;
  time_t diffTime;

  //***************************
  // Main analysis loop
  //***************************
  while (stillRunning)
  {
    currTime = mktime(&timeinfo);
    time(&rawtime);
    diffTime = difftime(rawtime, currTime);
    // Checks if we are already too late in the current iteration
    // If that's the case we wait until the next iteration
    // and we show a warning in the log file.
    if (diffTime > 0) {
      logFile << "WARNING: " << diffTime << " second(s) too late at " << printDateTime(currTime) << std::endl;
      timeinfo.tm_sec += (ceil(diffTime / params.interval) + 1) * params.interval;
      currTime = mktime(&timeinfo);
      sleep_for(secs(params.interval - (diffTime % params.interval)));
      logFile << std::endl;
    } else if (diffTime < 0) {
      sleep_for(secs(-diffTime));
    }
    // Header for every iteration of the loop
    if (params.verbose) {
      if (!inMarket) {
        logFile << "[ " << printDateTime(currTime) << " ]" << std::endl;
      } else {
        logFile << "[ " << printDateTime(currTime) << " IN MARKET: Long " << res.exchNameLong << " / Short " << res.exchNameShort << " ]" << std::endl;
      }
    }

    //*********************************************************************************
    // Gets the bid and ask of all the exchanges
    //*********************************************************************************
    for (int i = 0; i < numExch; ++i) {
      auto quote = exchanges[i]->getQuote(params, pair);
      double bid = quote.bid();
      double ask = quote.ask();

      // Saves the bid/ask into the SQLite database
      addBidAskToDb(dbTableName[i], printDateTimeDb(currTime), bid, ask, params);

      // If there is an error with the bid or ask (i.e. value is null),
      // we show a warning but we don't stop the loop.
      if (bid == 0.0) {
        logFile << "   WARNING: " << params.exchName[i] << " bid is null" << std::endl;
      }
      if (ask == 0.0) {
        logFile << "   WARNING: " << params.exchName[i] << " ask is null" << std::endl;
      }
      // Shows the bid/ask information in the log file
      if (params.verbose) {
        logFile << "   " << params.exchName[i] << ": \t"
                << std::setprecision(2)
                << bid << " / " << ask << std::endl;
      }
      // Updates the Bitcoin vector with the latest bid/ask data
      btcVec[i].updateData(quote);
      curl_easy_reset(params.curl);
    }
    if (params.verbose) {
      logFile << "   ----------------------------" << std::endl;
    }

    // Stores all the spreads in arrays to compute the volatility. The volatility is not used for the moment.
    if (params.useVolatility) {
      for (int i = 0; i < numExch; ++i) {
        for (int j = 0; j < numExch; ++j) {
          if (i != j) {
            if (btcVec[j].getHasShort()) {
              double longMidPrice = btcVec[i].getMidPrice();
              double shortMidPrice = btcVec[j].getMidPrice();
              if (longMidPrice > 0.0 && shortMidPrice > 0.0) {
                if (res.volatility[i][j].size() >= params.volatilityPeriod) {
                  res.volatility[i][j].pop_back();
                }
                res.volatility[i][j].push_front((longMidPrice - shortMidPrice) / longMidPrice);
              }
            }
          }
        }
      }
    }

    //*********************************************************************************
    // Looks for arbitrage opportunities on all the exchange combinations
    //*********************************************************************************
    if (!inMarket)
    {
      for (int i = 0; i < numExch; ++i) {
        for (int j = 0; j < numExch; ++j) {
          if (i != j) {
            if (checkEntry(&btcVec[i], &btcVec[j], res, params)) {
              //*********************************************************************************
              // An entry opportunity has been found!
              //*********************************************************************************
              res.exposure = std::min(balance[res.idExchLong].leg2, balance[res.idExchShort].leg2);
              if (params.isDemoMode) {
                logFile << "INFO: Opportunity found but no trade will be generated (Demo mode)" << std::endl;
                break;
              }
              if (res.exposure == 0.0) {
                logFile << "WARNING: Opportunity found but no cash available. Trade canceled" << std::endl;
                break;
              }
              if (params.useFullExposure == false && res.exposure <= params.testedExposure) {
                logFile << "WARNING: Opportunity found but no enough cash. Need more than TEST cash (min. $"
                        << std::setprecision(2) << params.testedExposure << "). Trade canceled" << std::endl;
                break;
              }
              if (params.useFullExposure) {
                // Removes 1% of the exposure to have
                // a little bit of margin.
                res.exposure -= 0.01 * res.exposure;
                if (res.exposure > params.maxExposure) {
                  logFile << "WARNING: Opportunity found but exposure ("
                          << std::setprecision(2)
                          << res.exposure << ") above the limit\n"
                          << "         Max exposure will be used instead (" << params.maxExposure << ")" << std::endl;
                  res.exposure = params.maxExposure;
                }
              } else {
                res.exposure = params.testedExposure;
              }
              // Checks the volumes and, based on that, computes the limit prices
              // that will be sent to the exchanges
              double volumeLong = res.exposure / btcVec[res.idExchLong].getAsk();
              double volumeShort = res.exposure / btcVec[res.idExchShort].getBid();
              double limPriceLong = exchanges[res.idExchLong]->getLimitPrice(params, volumeLong, false, pair);
              double limPriceShort = exchanges[res.idExchShort]->getLimitPrice(params, volumeShort, true, pair);
              if (limPriceLong == 0.0 || limPriceShort == 0.0) {
                logFile << "WARNING: Opportunity found but error with the order books (limit price is null). Trade canceled\n";
                logFile.precision(2);
                logFile << "         Long limit price:  " << limPriceLong << std::endl;
                logFile << "         Short limit price: " << limPriceShort << std::endl;
                res.trailing[res.idExchLong][res.idExchShort] = -1.0;
                break;
              }
              if (limPriceLong - res.priceLongIn > params.priceDeltaLim || res.priceShortIn - limPriceShort > params.priceDeltaLim) {
                logFile << "WARNING: Opportunity found but not enough liquidity. Trade canceled\n";
                logFile.precision(2);
                logFile << "         Target long price:  " << res.priceLongIn << ", Real long price:  " << limPriceLong << std::endl;
                logFile << "         Target short price: " << res.priceShortIn << ", Real short price: " << limPriceShort << std::endl;
                res.trailing[res.idExchLong][res.idExchShort] = -1.0;
                break;
              }
              // We are in market now, meaning we have positions on leg1 (the hedged on)
              // We store the details of that first trade into the Result structure.
              inMarket = true;
              resultId++;
              res.id = resultId;
              res.entryTime = currTime;
              res.priceLongIn = limPriceLong;
              res.priceShortIn = limPriceShort;
              res.printEntryInfo(*params.logFile);
              res.maxSpread[res.idExchLong][res.idExchShort] = -1.0;
              res.minSpread[res.idExchLong][res.idExchShort] = 1.0;
              res.trailing[res.idExchLong][res.idExchShort] = 1.0;

              // Send the orders to the two exchanges
              auto longOrderId = exchanges[res.idExchLong]->sendLongOrder(params, "buy", volumeLong, limPriceLong, pair);
              auto shortOrderId = exchanges[res.idExchShort]->sendShortOrder(params, "sell", volumeShort, limPriceShort, pair);
              logFile << "Waiting for the two orders to be filled... Long: " << longOrderId << " - Short: " << shortOrderId << std::endl;
              //TODO if orderId == 0 there was an error, we should rollback both

              sleep_for(millisecs(5000));
              bool isLongOrderComplete = exchanges[res.idExchLong]->isOrderComplete(params, longOrderId);
              bool isShortOrderComplete = exchanges[res.idExchShort]->isOrderComplete(params, shortOrderId);
              // Loops until both orders are completed
              while (!isLongOrderComplete || !isShortOrderComplete) {
                sleep_for(millisecs(3000));
                if (!isLongOrderComplete) {
                  logFile << "Long order on " << params.exchName[res.idExchLong] << " still open..." << std::endl;
                  isLongOrderComplete = exchanges[res.idExchLong]->isOrderComplete(params, longOrderId);
                }
                if (!isShortOrderComplete) {
                  logFile << "Short order on " << params.exchName[res.idExchShort] << " still open..." << std::endl;
                  isShortOrderComplete = exchanges[res.idExchShort]->isOrderComplete(params, shortOrderId);
                }
              }
              // Both orders are now fully executed
              logFile << "Done" << std::endl;

              // Stores the partial result to file in case
              // the program exits before closing the position.
              res.savePartialResult("restore.txt");

              // Resets the order ids
              longOrderId  = "0";
              shortOrderId = "0";
              break;
            }
          }
        } //end nested for
        if (inMarket) {
          break;
        }
      } // end first for
      if (params.verbose) {
        logFile << std::endl;
      }
    } else if (inMarket) {
      // We are in market and looking for an exit opportunity
      if (checkExit(&btcVec[res.idExchLong], &btcVec[res.idExchShort], res, params, currTime)) {
        //*********************************************************************************
        // An exit opportunity has been found!
        //*********************************************************************************
        // We check the current leg1 exposure
        std::vector<double> btcUsed(numExch);
        for (int i = 0; i < numExch; ++i) {
          btcUsed[i] = exchanges[i]->getActivePos(params, NSMop::str_tolower(params.leg1));
        }
        // Checks the volumes and computes the limit prices that will be sent to the exchanges
        double volumeLong = btcUsed[res.idExchLong];
        double volumeShort = btcUsed[res.idExchShort];
        double limPriceLong = exchanges[res.idExchLong]->getLimitPrice(params, volumeLong, true, pair);
        double limPriceShort = exchanges[res.idExchShort]->getLimitPrice(params, volumeShort, false, pair);
        if (limPriceLong == 0.0 || limPriceShort == 0.0) {
          logFile << "WARNING: Opportunity found but error with the order books (limit price is null). Trade canceled\n";
          logFile.precision(2);
          logFile << "         Long limit price:  " << limPriceLong << std::endl;
          logFile << "         Short limit price: " << limPriceShort << std::endl;
          res.trailing[res.idExchLong][res.idExchShort] = 1.0;
        } else if (res.priceLongOut - limPriceLong > params.priceDeltaLim || limPriceShort - res.priceShortOut > params.priceDeltaLim) {
          logFile << "WARNING: Opportunity found but not enough liquidity. Trade canceled\n";
          logFile.precision(2);
          logFile << "         Target long price:  " << res.priceLongOut << ", Real long price:  " << limPriceLong << std::endl;
          logFile << "         Target short price: " << res.priceShortOut << ", Real short price: " << limPriceShort << std::endl;
          res.trailing[res.idExchLong][res.idExchShort] = 1.0;
        } else {
          res.exitTime = currTime;
          res.priceLongOut = limPriceLong;
          res.priceShortOut = limPriceShort;
          res.printExitInfo(*params.logFile);

          logFile.precision(6);
          logFile << params.leg1 << " exposure on " << params.exchName[res.idExchLong] << ": " << volumeLong << '\n'
                  << params.leg1 << " exposure on " << params.exchName[res.idExchShort] << ": " << volumeShort << '\n'
                  << std::endl;
          auto longOrderId = exchanges[res.idExchLong]->sendLongOrder(params, "sell", fabs(btcUsed[res.idExchLong]), limPriceLong, pair);
          auto shortOrderId = exchanges[res.idExchShort]->sendShortOrder(params, "buy", fabs(btcUsed[res.idExchShort]), limPriceShort, pair);
          logFile << "Waiting for the two orders to be filled..." << std::endl;
          sleep_for(millisecs(5000));
          bool isLongOrderComplete = exchanges[res.idExchLong]->isOrderComplete(params, longOrderId);
          bool isShortOrderComplete = exchanges[res.idExchShort]->isOrderComplete(params, shortOrderId);
          // Loops until both orders are completed
          while (!isLongOrderComplete || !isShortOrderComplete) {
            sleep_for(millisecs(3000));
            if (!isLongOrderComplete) {
              logFile << "Long order on " << params.exchName[res.idExchLong] << " still open..." << std::endl;
              isLongOrderComplete = exchanges[res.idExchLong]->isOrderComplete(params, longOrderId);
            }
            if (!isShortOrderComplete) {
              logFile << "Short order on " << params.exchName[res.idExchShort] << " still open..." << std::endl;
              isShortOrderComplete = exchanges[res.idExchShort]->isOrderComplete(params, shortOrderId);
            }
          }
          logFile << "Done\n" << std::endl;
          longOrderId  = "0";
          shortOrderId = "0";
          inMarket = false;
          for (int i = 0; i < numExch; ++i) {
            std::string leg1AfterLower = NSMop::str_tolower(params.leg1);
            std::string leg2AfterLower = NSMop::str_tolower(params.leg2);

            balance[i].leg2After = exchanges[i]->getAvail(params, leg2AfterLower);
            balance[i].leg1After = exchanges[i]->getAvail(params, leg1AfterLower);
          }
          for (int i = 0; i < numExch; ++i) {
            logFile << "New balance on " << params.exchName[i] << ":  \t";
            logFile.precision(2);
            logFile << balance[i].leg2After << " " << params.leg2 << " (perf " << balance[i].leg2After - balance[i].leg2 << "), ";
            logFile << std::setprecision(6) << balance[i].leg1After << " " << params.leg1 << "\n";
          }
          logFile << std::endl;
          // Update total leg2 balance
          for (int i = 0; i < numExch; ++i) {
            res.leg2TotBalanceBefore += balance[i].leg2;
            res.leg2TotBalanceAfter  += balance[i].leg2After;
          }
          // Update current balances
          for (int i = 0; i < numExch; ++i) {
            balance[i].leg2 = balance[i].leg2After;
            balance[i].leg1 = balance[i].leg1After;
          }
          // Prints the result in the result CSV file
          logFile.precision(2);
          logFile << "ACTUAL PERFORMANCE: " << "$" << res.leg2TotBalanceAfter - res.leg2TotBalanceBefore << " (" << res.actualPerf() * 100.0 << "%)\n" << std::endl;
          csvFile.save(res);

          // Sends an email with the result of the trade
          if (params.sendEmail) {
            sendEmail(res, params);
            logFile << "Email sent" << std::endl;
          }
          res.reset();
          // Removes restore.txt since this trade is done.
          std::ofstream resFile("restore.txt", std::ofstream::trunc);
          resFile.close();
        }
      }
      if (params.verbose) logFile << '\n';
    }
    // Moves to the next iteration, unless
    // the maxmum is reached.
    timeinfo.tm_sec += params.interval;
    currIteration++;
    if (currIteration >= params.debugMaxIteration) {
      logFile << "Max iteration reached (" << params.debugMaxIteration << ")" <<std::endl;
      stillRunning = false;
    }
    // Exits if a 'stop_after_notrade' file is found
    // Warning: by default on GitHub the file has a underscore
    // at the end, so Blackbird is not stopped by default.
    std::ifstream infile("stop_after_notrade");
    if (infile && !inMarket) {
      logFile << "Exit after last trade (file stop_after_notrade found)\n";
      stillRunning = false;
    }
  }
  // Analysis loop exited, does some cleanup
  curl_easy_cleanup(params.curl);
  logFile.close();

  //clean exchanges
  for (NSExchange::Exchanges::iterator it(exchanges.begin()); it != exchanges.end(); ++it)
  {
    delete (*it);
  }

  return 0;
}

