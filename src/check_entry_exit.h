#ifndef CHECK_ENTRY_EXIT_H
#define CHECK_ENTRY_EXIT_H

#include <string>
#include <ctime>


class  Bitcoin;
struct Result;
struct Parameters;

std::string percToStr(double perc);

/**
 * @brief Checks for entry opportunity between two exchanges
 *
 * @param btcLong[in]: bitcoin pointer that represents one exchange
 * @param btcShort[in]: bitcoin pointer that represents another exchange
 * @param res[out] to fill with the operation information
 * @param params[in|out]: contains the blackbird configuration
 *
 * @return True if an opporunity is found.
 *
 * NOTE: the bitcoin pointers represent just long exchange. We maintain the names
 *       because in a short future we will reuse short trading.
 */
bool checkEntry(const Bitcoin* const btcLong, const Bitcoin* const btcShort, Result& res, Parameters& params);

// Checks for exit opportunity between two exchanges
// and returns True if an opporunity is found.
bool checkExit(Bitcoin* btcLong, Bitcoin* btcShort, Result& res, Parameters& params, std::time_t period);

#endif

