/*
 * @file  tradeOutput.h
 * @brief Provides the interface to create the Trade results output file
 */

#ifndef TRADE_OUTPUT_H
#define TRADE_OUTPUT_H

#include <fstream>
#include "result.h"

namespace NSOutput
{

/** @brief Represents a file name */
typedef std::string FileName;

/**
 * @brief Responsible to create the CSV file that will collect the trade results
 */
class TradeOutput
{
public:

	/**
     * @brief Parametrized class constructor
     *
     * @param fileName[in] the output file name
     */
    TradeOutput(const FileName& fileName);

    /**
     * @brief Destructor class
     */
    ~TradeOutput();

    /**
     * @brief Saves a row
     *
     * @param result[in] contains the data to store in the row
     */
    void save(const Result& result);

private:

	/**
	 * @brief Writes the header file
	 */
    void generateHeader();

    /** @brief File to fill */
    std::ofstream _outputFile;
};

} //namespace NSOutput
#endif /* TRADE_OUTPUT_H */
