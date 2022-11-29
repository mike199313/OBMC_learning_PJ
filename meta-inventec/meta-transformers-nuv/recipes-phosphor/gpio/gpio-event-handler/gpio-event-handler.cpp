#include "utils.hpp"
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message.hpp>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>
#include <map>

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/bind/bind.hpp>
#include <boost/ref.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/bus/match.hpp>
#include <phosphor-logging/log.hpp>
#include <ipmid/api.hpp>
#include <ipmid/types.hpp>
#include <ipmid/utils.hpp>


#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
#define dbg(...) do{ {dbg_impl_v1(__FILE__, __LINE__, __func__, __VA_ARGS__);} }while(0)

[[maybe_unused]]inline static void dbg_impl_v1(const char* srcname, int linenum, const char* funcname, const char* fmt, ...)
{
		va_list ap;
		char *filename = basename(strdup(srcname));

		fprintf(stderr, "{%s:%d:%s}:",filename, linenum, funcname);
		va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fflush(stderr);
}
#else
#define dbg(...) do{}while(0)
#endif



#ifdef __cplusplus
}
#endif

using namespace std;
using namespace phosphor::logging;

static constexpr char const* ipmiSELService = "xyz.openbmc_project.Logging.IPMI";
static constexpr char const* ipmiSELPath = "/xyz/openbmc_project/Logging/IPMI";
static constexpr char const* ipmiSELAddInterface = "xyz.openbmc_project.Logging.IPMI";

static const std::string ipmiSELAddMessage = "SEL Entry";
static constexpr size_t selEvtDataMaxSize = 3;
static const std::string ASKeyWord = "_ASSERTED";
static const std::string DEKeyWord = "_DEASSERTED";


static const std::string DBUS_OBJPATH_FAST_THROTTLING = 	"/xyz/openbmc_project/sensors/oem_event_00h/oem_d1h/Fast_throttling";
static const std::string DBUS_OBJPATH_FAN_RPM_REPORT = 		"/xyz/openbmc_project/sensors/discrete_6fh/fan/FanRPMReport";
static const std::string DBUS_OBJPATH_SMI_TIMEOUT_CHECK = 	"/xyz/openbmc_project/sensors/oem_event_00h/oem_d5h/SMITimeoutCheck";


static constexpr int16_t maxInt10 = 0x1FF;
static constexpr int16_t minInt10 = -0x200;
static constexpr int8_t maxInt4 = 7;
static constexpr int8_t minInt4 = -8;


static inline bool baseInRange(double base)
{
    auto min10 = static_cast<double>(minInt10);
    auto max10 = static_cast<double>(maxInt10);

    return ((base >= min10) && (base <= max10));
}

static inline bool scaleFloatExp(double& base, int8_t& expShift)
{
    // Comparing with zero should be OK, zero is special in floating-point
    // If base is exactly zero, no adjustment of the exponent is necessary
    if (base == 0.0)
    {
        return true;
    }

    // As long as base value is within allowed range, expand precision
    // This will help to avoid loss when later rounding to integer
    while (baseInRange(base))
    {
        if (expShift <= minInt4)
        {
            // Already at the minimum expShift, can not decrement it more
            break;
        }

        // Multiply by 10, but shift decimal point to the left, no net change
        base *= 10.0;
        --expShift;
    }

    // As long as base value is *not* within range, shrink precision
    // This will pull base value closer to zero, thus within range
    while (!(baseInRange(base)))
    {
        if (expShift >= maxInt4)
        {
            // Already at the maximum expShift, can not increment it more
            break;
        }

        // Divide by 10, but shift decimal point to the right, no net change
        base /= 10.0;
        ++expShift;
    }

    // If the above loop was not able to pull it back within range,
    // the base value is beyond what expShift can represent, return false.
    return baseInRange(base);
}


static inline void normalizeIntExp(int16_t& ibase, int8_t& expShift,
                                   double& dbase)
{
    for (;;)
    {
        // If zero, already normalized, ensure exponent also zero
        if (ibase == 0)
        {
            expShift = 0;
            break;
        }

        // If not cleanly divisible by 10, already normalized
        if ((ibase % 10) != 0)
        {
            break;
        }

        // If exponent already at max, already normalized
        if (expShift >= maxInt4)
        {
            break;
        }

        // Bring values closer to zero, correspondingly shift exponent,
        // without changing the underlying number that this all represents,
        // similar to what is done by scaleFloatExp().
        // The floating-point base must be kept in sync with the integer base,
        // as both floating-point and integer share the same exponent.
        ibase /= 10;
        dbase /= 10.0;
        ++expShift;
    }
}

static inline bool getSensorAttributes(const double max, const double min,
                                       int16_t& mValue, int8_t& rExp,
                                       int16_t& bValue, int8_t& bExp,
                                       bool& bSigned)
{
    if (!(std::isfinite(min)))
    {
        std::cerr << "getSensorAttributes: Min value is unusable\n";
        return false;
    }
    if (!(std::isfinite(max)))
    {
        std::cerr << "getSensorAttributes: Max value is unusable\n";
        return false;
    }

    // Because NAN has already been tested for, this comparison works
    if (max <= min)
    {
        std::cerr << "getSensorAttributes: Max must be greater than min\n";
        return false;
    }

    double fullRange = max - min;
    double lowestX;

    rExp = 0;
    bExp = 0;

    if (min < 0.0)
    {
        bSigned = true;
        lowestX = -128.0;
    }
    else
    {
        bSigned = false;
        lowestX = 0.0;
    }

    // Step 1: Set y to (max - min), set x to 255, set B to 0, solve for M
    // This works, regardless of signed or unsigned,
    // because total range is the same.
    double dM = fullRange / 255.0;

    // Step 2: Constrain M, and set rExp accordingly
    if (!(scaleFloatExp(dM, rExp)))
    {
        std::cerr << "getSensorAttributes: Multiplier range exceeds scale (M="
                  << dM << ", rExp=" << (int)rExp << ")\n";
        return false;
    }

    mValue = static_cast<int16_t>(std::round(dM));

    normalizeIntExp(mValue, rExp, dM);

    // The multiplier can not be zero, for obvious reasons
    if (mValue == 0)
    {
        std::cerr << "getSensorAttributes: Multiplier range below scale\n";
        return false;
    }

    // Step 3: set y to min, set x to min, keep M and rExp, solve for B
    // If negative, x will be -128 (the most negative possible byte), not 0

    // Solve the IPMI equation for B, instead of y
    // https://www.wolframalpha.com/input/?i=solve+y%3D%28%28M*x%29%2B%28B*%2810%5EE%29%29%29*%2810%5ER%29+for+B
    // B = 10^(-rExp - bExp) (y - M 10^rExp x)
    // TODO(): Compare with this alternative solution from SageMathCell
    // https://sagecell.sagemath.org/?z=eJyrtC1LLNJQr1TX5KqAMCuATF8I0xfIdIIwnYDMIteKAggPxAIKJMEFkiACxfk5Zaka0ZUKtrYKGhq-CloKFZoK2goaTkCWhqGBgpaWAkilpqYmQgBklmasjoKTJgDAECTH&lang=sage&interacts=eJyLjgUAARUAuQ==
    double dB = std::pow(10.0, ((-rExp) - bExp)) *
                (min - ((dM * std::pow(10.0, rExp) * lowestX)));

    // Step 4: Constrain B, and set bExp accordingly
    if (!(scaleFloatExp(dB, bExp)))
    {
        std::cerr << "getSensorAttributes: Offset (B=" << dB
                  << ", bExp=" << (int)bExp
                  << ") exceeds multiplier scale (M=" << dM
                  << ", rExp=" << (int)rExp << ")\n";
        return false;
    }

    bValue = static_cast<int16_t>(std::round(dB));

    normalizeIntExp(bValue, bExp, dB);

    // Unlike the multiplier, it is perfectly OK for bValue to be zero
    return true;
}


static inline uint8_t
    scaleIPMIValueFromDouble(const double value, const int16_t mValue,
                             const int8_t rExp, const int16_t bValue,
                             const int8_t bExp, const bool bSigned)
{
    // Avoid division by zero below
    if (mValue == 0)
    {
        throw std::out_of_range("Scaling multiplier is uninitialized");
    }

    auto dM = static_cast<double>(mValue);
    auto dB = static_cast<double>(bValue);

    // Solve the IPMI equation for x, instead of y
    // https://www.wolframalpha.com/input/?i=solve+y%3D%28%28M*x%29%2B%28B*%2810%5EE%29%29%29*%2810%5ER%29+for+x
    // x = (10^(-rExp) (y - B 10^(rExp + bExp)))/M and M 10^rExp!=0
    // TODO(): Compare with this alternative solution from SageMathCell
    // https://sagecell.sagemath.org/?z=eJyrtC1LLNJQr1TX5KqAMCuATF8I0xfIdIIwnYDMIteKAggPxAIKJMEFkiACxfk5Zaka0ZUKtrYKGhq-CloKFZoK2goaTkCWhqGBgpaWAkilpqYmQgBklmasDlAlAMB8JP0=&lang=sage&interacts=eJyLjgUAARUAuQ==
    double dX =
        (std::pow(10.0, -rExp) * (value - (dB * std::pow(10.0, rExp + bExp)))) /
        dM;

    auto scaledValue = static_cast<int32_t>(std::round(dX));

    int32_t minClamp;
    int32_t maxClamp;

    // Because of rounding and integer truncation of scaling factors,
    // sometimes the resulting byte is slightly out of range.
    // Still allow this, but clamp the values to range.
    if (bSigned)
    {
        minClamp = std::numeric_limits<int8_t>::lowest();
        maxClamp = std::numeric_limits<int8_t>::max();
    }
    else
    {
        minClamp = std::numeric_limits<uint8_t>::lowest();
        maxClamp = std::numeric_limits<uint8_t>::max();
    }

    auto clampedValue = std::clamp(scaledValue, minClamp, maxClamp);

    // This works for both signed and unsigned,
    // because it is the same underlying byte storage.
    return static_cast<uint8_t>(clampedValue);
}



int main(int argc, char* argv[]){
	auto bus = sdbusplus::bus::new_default();

	if(argc < 1){
		cout << "Parameter error, run like gpio-event-handler CPU0_PROCHOT_N_ASSERTED\n\n";
	}

	string tarGPIO = argv[1];
	uint16_t genId = 0x20;
    	bool assert = true, gpioState = true;

	bool doSystemSEL = false;

	/* Formats are only XXXX_ASSERTED and XXXX_DEASSERTED
	 * rfind to avoid XXXX_ASSERTED_XXXX and XXXX_DEASSERTED_XXXX
	 */

	dbg("tarGPIO=%s \n", tarGPIO.c_str());

	if(tarGPIO.rfind(DEKeyWord) != string::npos && 
	   tarGPIO.rfind(DEKeyWord) >= (tarGPIO.length() - DEKeyWord.length())){
		gpioState = false;
		tarGPIO = tarGPIO.substr(0, tarGPIO.rfind(DEKeyWord));
	}else if(tarGPIO.rfind(ASKeyWord) != string::npos && 
	   	 tarGPIO.rfind(ASKeyWord) >= (tarGPIO.length() - ASKeyWord.length())){
		gpioState = true;
		tarGPIO = tarGPIO.substr(0, tarGPIO.rfind(ASKeyWord));
	}else{
		cout << "ERROR: No ASSERTED or DEASSERTED within target." << endl;
		return 0;
	}

   	std::vector<uint8_t> eventData(selEvtDataMaxSize, 0xFF);
    	string errorSensorPath = "/xyz/openbmc_project/sensors/discrete/";
	
	/* append tpye according key word in name */
	if((tarGPIO.find("INTRUDER") != string::npos) ||
		       	(tarGPIO.find("DIMM_MODULE") != string::npos) ||
		       	(tarGPIO.find("PWR") != string::npos) ||
		       	(tarGPIO.find("NMI_SW") != string::npos) ||
		       	(tarGPIO.find("CPU") != string::npos) ||
		       	(tarGPIO.find("PROC") != string::npos) ||
		       	(tarGPIO.find("PCH_HOT_N") != string::npos) )
	{
		doSystemSEL = true;
    	errorSensorPath += "processor/";
    }else if ((tarGPIO.find("HOT") != string::npos) || (tarGPIO.find("THERM") != string::npos)){
        doSystemSEL = true;
		errorSensorPath += "temperature/";
	
	}else if(tarGPIO.find("PCH_SMI_ACTIVE_N") != string::npos){
		doSystemSEL = false;
	}else{
		return 0;
	}

	if(doSystemSEL)
	{
		/* append sensor name */
		errorSensorPath += tarGPIO;
		dbg("Sensor path: %s", errorSensorPath.c_str());

		/* asseted or deasserted according state */
		if (gpioState)
		{
			eventData[0] = 0x01;
		}
		else
		{
			eventData[0] = 0x00;
		}

		sdbusplus::message::message writeMcerrSEL = bus.new_method_call(
			ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAdd");
		writeMcerrSEL.append(ipmiSELAddMessage, errorSensorPath, eventData, assert, genId);
		try
		{
			bus.call(writeMcerrSEL);
		}
		catch (sdbusplus::exception_t &e)
		{
			
			log<level::ERR>("MCErr SEL log error. ",
							entry("MSG=%s", e.what()));
		}
	}

	//MS-BMC-LOG-0006 FanRPM when PROCHOT/MEMHOT
	if( (gpioState == true)
		&& (
				(tarGPIO.find("DIMM_MODULE") != string::npos) ||
		       	(tarGPIO.find("CPU") != string::npos) ||
		       	(tarGPIO.find("PROC") != string::npos)
		   ) 
	  )
	{
	    //do PWM event log as system event SEL
        std::vector<uint8_t> eventData = {0xA0, 0x01, 0x01};

        sdbusplus::message::message writeSEL = bus.new_method_call(
        		ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAdd");
        writeSEL.append(std::string("FAN RPM report PWM event"),
                        DBUS_OBJPATH_FAN_RPM_REPORT,
                    	eventData, gpioState, genId);
		try{
			bus.call(writeSEL);
		}catch(sdbusplus::exception_t &e){
			log<level::ERR>("FAN RPMReport TachoMeter Event error. ",
							entry("MSG=%s", e.what()));
		}

		constexpr auto SENSOR_VALUE_INTERFACE = "xyz.openbmc_project.Sensor.Value";
		constexpr auto SERVICE_ROOT = "/";
		
		std::vector<uint8_t> recData = {0xa9, 0x19, 0x00};
		uint8_t recordType = 0xD0;
		std::vector<uint8_t> TranslatedRPMValue;
		auto objTree = ipmi::getAllDbusObjects(bus, SERVICE_ROOT, SENSOR_VALUE_INTERFACE, {"fan_tach"});
		for (const auto &[objPath, objMap] : objTree)
		{
			for (const auto &[objService, objIntfs] : objMap)
			{
				auto propMAX = ipmi::getDbusProperty(bus, objService, objPath, SENSOR_VALUE_INTERFACE, "MaxValue");
				auto propMIN = ipmi::getDbusProperty(bus, objService, objPath, SENSOR_VALUE_INTERFACE, "MinValue");

				auto max = std::fmax(std::get<double>(propMAX), 127);
				auto min = std::fmax(std::get<double>(propMIN), -128);
				int16_t mValue = 0;
				int8_t rExp = 0;
				int16_t bValue = 0;
				int8_t bExp = 0;
				bool bSigned = false;

				if (!getSensorAttributes(max, min, mValue, rExp, bValue, bExp, bSigned))
				{
					phosphor::logging::log<phosphor::logging::level::ERR>(
						"getSensorDataRecord: getSensorAttributes error");
					continue;
				}
				auto property = ipmi::getDbusProperty(bus, objService, objPath, SENSOR_VALUE_INTERFACE, "Value");
				double value = std::get<double>(property);
				

    			uint8_t ipmiConvertedValue = scaleIPMIValueFromDouble(value, mValue, rExp, bValue, bExp, bSigned);
				TranslatedRPMValue.push_back(ipmiConvertedValue);
				dbg("objService=%s objPath=%s max=%.2f min=%.2f rpmValue=%.2f ipmiConvertedValue=%02X\n", 
						objService.c_str(), objPath.c_str(), max, min, value, ipmiConvertedValue);
				
			}
		}

		sdbusplus::message::message writeSELOEM1 = bus.new_method_call(
			ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAddOem");
		recData.insert(recData.end(), TranslatedRPMValue.begin(), TranslatedRPMValue.begin()+6);
		writeSELOEM1.append(std::string("FAN RPM Report TachoMeter event 1"), recData, recordType);
		try{
			bus.call(writeSELOEM1);
		}catch(sdbusplus::exception_t &e){
			log<level::ERR>("FAN RPMReport TachoMeter Event error. ",
							entry("MSG=%s", e.what()));
		}
		
		recData.erase(recData.begin()+3, recData.end());
		sdbusplus::message::message writeSELOEM2 = bus.new_method_call(
			ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAddOem");
		recData.insert(recData.end(), TranslatedRPMValue.begin()+6, TranslatedRPMValue.begin()+12);
		writeSELOEM2.append(std::string("FAN RPM Report TachoMeter event 2"), recData, recordType);
		try{
			bus.call(writeSELOEM2);
		}catch(sdbusplus::exception_t &e){
			log<level::ERR>("FAN RPMReport TachoMeter Event error. ",
							entry("MSG=%s", e.what()));
		}
		

	}

	//MS-BMC-LOG0010 SMI Timout SEL
	if((tarGPIO.find("PCH_SMI_ACTIVE_N") != string::npos))
	{

		if(gpioState){
			//ASSERTED, do SEL after 360 seconds
			fprintf(stderr,"Waiting SMI timeout 360 seconds\n");
			startSystemdUnit(bus, "smitimeout-handler@360.service");

		}else{
			//DEASSERTED, do SEL immediately
			stopSystemdUnit(bus, "smitimeout-handler@360.service");
			std::vector<uint8_t> eventData = {0xFF, 0xFF, 0xFF};
			sdbusplus::message::message writeSEL = bus.new_method_call(
													ipmiSELService, 
													ipmiSELPath, 
													ipmiSELAddInterface, 
													"IpmiSelAdd");
			writeSEL.append(std::string("SMI Timeout SEL"),
							DBUS_OBJPATH_SMI_TIMEOUT_CHECK,
							eventData, gpioState, genId);
			try{
				bus.call(writeSEL);
			}catch (sdbusplus::exception_t &e){
				log<level::ERR>("SMITimoutHandler error : ",
								entry("MSG=%s", e.what()));
			}
		}
	}

	//MS-BMC-LOG-0004 Fast-Throttling CPU/PROC HOST Asserted
	if( (gpioState == true) 
		&& ( (tarGPIO.find("CPU") != string::npos) 
			|| (tarGPIO.find("PROC") != string::npos) ))
	{
		constexpr auto SENSOR_VALUE_INTERFACE = "xyz.openbmc_project.Sensor.Threshold.Critical";
		constexpr auto SERVICE_ROOT = "/";
		auto objTree = ipmi::getAllDbusObjects(bus, SERVICE_ROOT, SENSOR_VALUE_INTERFACE, {"PSU"});

		for (const auto &[objPath, objMap] : objTree)
		{
			for (const auto &[objService, objIntfs] : objMap)
			{
				uint8_t EventData1_bit4_to_bit7 = 0xA0;
				uint8_t VOLTAGE_ALERT_N = 0x05 | EventData1_bit4_to_bit7;
				uint8_t CURRENT_ALERT_N = 0x06 | EventData1_bit4_to_bit7;
				auto property = ipmi::getDbusProperty(bus, objService, objPath, SENSOR_VALUE_INTERFACE, "CriticalAlarmHigh");
				bool value = std::get<bool>(property);
				if (value == true)
				{
					uint8_t eventData1 = 0x00;
					// Getting value is NaN. do SEL
					if(objPath.find("current") != string::npos){
						eventData1 = CURRENT_ALERT_N;
					}else if(objPath.find("voltage") != string::npos){
						eventData1 = VOLTAGE_ALERT_N;
					}

					dbg("objService=%s objPath=%s eventData1=%02X  \n", objService.c_str(), objPath.c_str(), eventData1);
					std::vector<uint8_t> eventData = {eventData1, 0x00, 0x00};
                    sdbusplus::message::message writeSEL = bus.new_method_call(ipmiSELService, 
																				ipmiSELPath, 
																				ipmiSELAddInterface, 
																				"IpmiSelAdd");
                    writeSEL.append(std::string("Fast-Throttling Asserted"),
                                                DBUS_OBJPATH_FAST_THROTTLING,
                                                eventData, gpioState, genId);
                    try{
                    	bus.call(writeSEL);
                    }catch (sdbusplus::exception_t &e){
						log<level::ERR>("Fast-Throttling SEL error : ",
								entry("MSG=%s", e.what()));
                    }

				}
			}
		}
	}

	return 0;
}
