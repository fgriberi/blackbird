#include "tradeOutput.h"
#include "time_fun.h"

namespace NSOutput
{

TradeOutput::TradeOutput(const FileName& fileName) :
	_outputFile(fileName, std::ofstream::trunc)
{
	generateHeader();
}

TradeOutput::~TradeOutput()
{
    _outputFile.close();
}

void TradeOutput::save(const Result& result)
{
	_outputFile << result.id << ","
              	<< result.exchNameLong << ","
              	<< result.exchNameShort << ","
              	<< printDateTimeCsv(result.entryTime) << ","
              	<< printDateTimeCsv(result.exitTime) << ","
              	<< result.getTradeLengthInMinute() << ","
              	<< result.exposure * 2.0 << ","
              	<< result.leg2TotBalanceBefore << ","
              	<< result.leg2TotBalanceAfter << ","
              	<< result.actualPerf() << std::endl;
}

void TradeOutput::generateHeader()
{
	_outputFile << "TRADE_ID,EXCHANGE_LONG,EXHANGE_SHORT,ENTRY_TIME,"
				<< "EXIT_TIME,DURATION,TOTAL_EXPOSURE,BALANCE_BEFORE,"
				<< "BALANCE_AFTER,RETURN"
				<< std::endl;
}

} //namespace NSOutput