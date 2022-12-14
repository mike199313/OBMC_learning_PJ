#include "utils.hpp"
#include <iostream>
#include <string>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message.hpp>

using namespace std;

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


static const int MAX_SLEEP_TIME = 300;
static const int FULL_DATA = 0xFF;

static const int RESP_INDEX_POWER_STATUS = 1;
static const int RESP_INDEX_COMPLETE_CODE = 0;
static const int RESP_INDEX_SENSOR_NUMBER = 10;
static const int RESP_INDEX_SENSOR_TYPE = 15;
static const int RESP_INDEX_RECORD_ID_LSB = 1;
static const int RESP_INDEX_RECORD_ID_MSB = 2;
static const int RESP_INDEX_READING_STATUS = 2;
static const int RESP_INDEX_RECORD_TYPE = 6;
static const int RESP_INDEX_EVENT_STATUS = 3;
static const int RESP_INDEX_SENSOR_READING = 1;

static const int CMD_INDEX_RECORD_ID_LSB = 2;
static const int CMD_INDEX_RECORD_ID_MSB = 3;
static const int CMD_INDEX_DATA_LENGTH = 5;
static const int CMD_INDEX_SENSOR_NUMBER = 0;

static const int SENSOR_TYPE_VOLTAGE = 0x02;
static const int SENSOR_TYPE_CURRENT = 0x03;
static const int SENSOR_TYPE_FAN = 0x04;


static const int SDR_RECORD_TYPE_FULL = 0x01;


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

	fprintf(stderr, "tarGPIO=%s \n", tarGPIO.c_str());

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
		cerr << "ERROR: type not support." << endl;
		return 0;
	}

	if(doSystemSEL)
	{
		/* append sensor name */
		errorSensorPath += tarGPIO;
		cout << "Sensor path: " << errorSensorPath << endl;

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
			std::cerr << "Failed to log SEL\n";
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
		uint8_t lun = 0;
		uint8_t netfn = 0;
		uint8_t cmd = 0;
		std::vector<uint8_t> cmdData;
		std::vector<uint8_t> TranslatedRPMValue(256, 0xFF);
		auto retConvey = std::make_shared<DbusRspData>();

		netfn = 0x0a; // Storage
		cmd = 0x23;	  // GET SDR
		cmdData.resize(6, 0);

		cmdData.at(CMD_INDEX_RECORD_ID_LSB) = 0x00;
		cmdData.at(CMD_INDEX_RECORD_ID_MSB) = 0x00;
		cmdData.at(CMD_INDEX_DATA_LENGTH) = FULL_DATA; // all the data

		for (int i = 0, index=0; i < 255; i++)
		{
			// Get SDR record of specified record id
			ipmi_method_call(lun, netfn, cmd, cmdData, retConvey);
			
			if (retConvey->retData.at(RESP_INDEX_COMPLETE_CODE) == 0x00 
				&& retConvey->retData.at(RESP_INDEX_RECORD_TYPE) == SDR_RECORD_TYPE_FULL 
				&& (retConvey->retData.at(RESP_INDEX_SENSOR_TYPE) == SENSOR_TYPE_FAN) 
			   )
			{
				//call IPMI GET SENSOR DATA READING to get the sensor value
				uint8_t lun2 = 0x00;
				uint8_t netfn2 = 0x04;
				uint8_t cmd2 = 0x2d;
				std::vector<uint8_t> cmdData2(1);
				auto retConvey2 = std::make_shared<DbusRspData>();
				cmdData2.at(CMD_INDEX_SENSOR_NUMBER) = retConvey->retData.at(RESP_INDEX_SENSOR_NUMBER);
				ipmi_method_call(lun2, netfn2, cmd2, cmdData2, retConvey2);
				if(retConvey2->retData.at(RESP_INDEX_COMPLETE_CODE) == 0x00){
					TranslatedRPMValue.at(index) = (retConvey2->retData.at(RESP_INDEX_SENSOR_READING));
				}else{
					TranslatedRPMValue.at(index) = 0xFF;
				}
				index++;
			}
			else
			{
				if(retConvey->ec.value()!= 0){
					fprintf(stderr, "index %02X is failed ec=%d\n", i, retConvey->ec.value());
				}
			}

			// At the end, get all SDR records done.
			if ((retConvey->retData[RESP_INDEX_RECORD_ID_LSB] == 0xFF) &&
				(retConvey->retData[RESP_INDEX_RECORD_ID_MSB] == 0xFF))
			{
				break;
			}
			else
			{
				// setup cmdData to get next SDR record
				cmdData.at(CMD_INDEX_RECORD_ID_LSB) = retConvey->retData.at(RESP_INDEX_RECORD_ID_LSB);
				cmdData.at(CMD_INDEX_RECORD_ID_MSB) = retConvey->retData.at(RESP_INDEX_RECORD_ID_MSB);
				cmdData.at(CMD_INDEX_DATA_LENGTH) = FULL_DATA; // all the data
			}
		}
		
		//do PWM event log as system event SEL
		std::vector<uint8_t> eventData = {0xA0, 0x01, 0x01};
		sdbusplus::message::message writeSEL = bus.new_method_call(
			ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAdd");
		writeSEL.append(std::string("FAN RPM report PWM event"),
						DBUS_OBJPATH_FAN_RPM_REPORT,
						eventData, gpioState, genId);
		try
		{
			bus.call(writeSEL);
		}
		catch (sdbusplus::exception_t &e)
		{
			std::cerr << "Failed to log SEL\n";
		}

		//do Tachometer oem SEL
		std::vector<uint8_t> recData = {0xa9, 0x19, 0x00};
		uint8_t recordType = 0xD0;

		
		sdbusplus::message::message writeSELOEM1 = bus.new_method_call(
			ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAddOem");
		
		recData.insert(recData.end(), TranslatedRPMValue.begin(), TranslatedRPMValue.begin()+6);
		writeSELOEM1.append(std::string("FAN RPM report TachoMeter event 1"),
						recData, recordType);
		

		sdbusplus::message::message writeSELOEM2 = bus.new_method_call(
			ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAddOem");

		recData.erase(recData.begin()+3, recData.end());
		recData.insert(recData.begin()+3, TranslatedRPMValue.begin()+6, TranslatedRPMValue.begin()+12);
		
		
		writeSELOEM2.append(std::string("FAN RPM report TachoMeter event 2"),
						recData, recordType);

				
		try
		{
			bus.call(writeSELOEM1);
		}
		catch (sdbusplus::exception_t &e)
		{
			std::cerr << "Failed FAN RPM report TachoMeter event 1 SEL errMsg=>" << e.what() << std::endl;
		}


		try
		{
			bus.call(writeSELOEM2);
		}
		catch (sdbusplus::exception_t &e)
		{
			std::cerr << "Failed FAN RPM report TachoMeter event 1 SEL errMsg=>" << e.what() << std::endl;	
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
				std::cerr << "Failed to log SEL\n";
			}
		}
	}

	//MS-BMC-LOG-0004 Fast-Throttling CPU/PROC HOST Asserted
	if( (gpioState == true) 
		&& ( (tarGPIO.find("CPU") != string::npos) 
			|| (tarGPIO.find("PROC") != string::npos) ))
	{
		uint8_t lun = 0;
		uint8_t netfn = 0;
		uint8_t cmd = 0;
		std::vector<uint8_t> cmdData;
		auto retConvey = std::make_shared<DbusRspData>();

		netfn = 0x0a; //Storage
		cmd = 0x23;	  // GET SDR
		cmdData.resize(6, 0);

		cmdData.at(CMD_INDEX_RECORD_ID_LSB) = 0x00;
		cmdData.at(CMD_INDEX_RECORD_ID_MSB) = 0x00;
		cmdData.at(CMD_INDEX_DATA_LENGTH) = FULL_DATA; // all the data

		for (int i = 0; i < 255; i++)
		{
			// Get SDR record of specified record id
			ipmi_method_call(lun, netfn, cmd, cmdData, retConvey);
			
			if (retConvey->retData.at(RESP_INDEX_COMPLETE_CODE) == 0x00 
				&& retConvey->retData.at(RESP_INDEX_RECORD_TYPE) == SDR_RECORD_TYPE_FULL 
				&& (retConvey->retData.at(RESP_INDEX_SENSOR_TYPE) == SENSOR_TYPE_VOLTAGE) 
					|| (retConvey->retData.at(RESP_INDEX_SENSOR_TYPE) == SENSOR_TYPE_CURRENT))
			{
				// using IPMI GET SENSOR EVENT STATUS (netfn=0x04, cmd=0x2b with sensor number to get reading status)
				uint8_t lun2 = 0;
				uint8_t netfn2 = 0x04;
				uint8_t cmd2 = 0x2b;
				std::vector<uint8_t> cmdData2(1);
				auto retConvey2 = std::make_shared<DbusRspData>();
				cmdData2.at(CMD_INDEX_SENSOR_NUMBER) = retConvey->retData.at(RESP_INDEX_SENSOR_NUMBER); // set SensorNumber;

				ipmi_method_call(lun2, netfn2, cmd2, cmdData2, retConvey2);

				if (retConvey2->retData.at(RESP_INDEX_COMPLETE_CODE) == 0x00)
				{
					if (retConvey2->retData.at(RESP_INDEX_EVENT_STATUS) & 0x02)
					{
						// Event set do SEL MS M-BMC-LOG-0004
						std::string sensorId(retConvey->retData.begin()+51, retConvey->retData.end());
						fprintf(stderr, "%s do SEL MS M-BMC-LOG-0004 Asserted\n", sensorId.c_str());
						uint8_t value;
						if (retConvey->retData.at(RESP_INDEX_SENSOR_TYPE) == SENSOR_TYPE_VOLTAGE)
						{
							value = 0xA5;
						}
						else
						{
							value = 0xA6;
						}
						std::vector<uint8_t> eventData = {value, 0x00, 0x00};

						sdbusplus::message::message writeSEL = bus.new_method_call(
							ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAdd");
						writeSEL.append(std::string("FAST THROTTLING Asserted"),
										DBUS_OBJPATH_FAST_THROTTLING,
										eventData, gpioState, genId);
						try
						{
							bus.call(writeSEL);
						}
						catch (sdbusplus::exception_t &e)
						{
							std::cerr << "Failed to log SEL\n";
						}
					}
				}
			}
			else
			{
				if(retConvey->ec.value() != 0){
					fprintf(stderr, "index %02X is failed ec=%d\n", i, retConvey->ec.value());	
				}
			}

			// At the end, get all SDR records done.
			if ((retConvey->retData[RESP_INDEX_RECORD_ID_LSB] == 0xFF) &&
				(retConvey->retData[RESP_INDEX_RECORD_ID_MSB] == 0xFF))
			{
				break;
			}
			else
			{
				// setup cmdData to get next SDR record
				cmdData.at(CMD_INDEX_RECORD_ID_LSB) = retConvey->retData.at(RESP_INDEX_RECORD_ID_LSB);
				cmdData.at(CMD_INDEX_RECORD_ID_MSB) = retConvey->retData.at(RESP_INDEX_RECORD_ID_MSB);
				cmdData.at(CMD_INDEX_DATA_LENGTH) = FULL_DATA; // all the data
			}
		}
	}else if( (gpioState == false) 
		&& ( (tarGPIO.find("CPU") != string::npos) 
			|| (tarGPIO.find("PROC") != string::npos) ))
	{
		//CPU/PROC HOST DeAsserted
		uint8_t value;
		std::vector<uint8_t> eventData = {0x00, 0x00, 0x00};

		sdbusplus::message::message writeSEL = bus.new_method_call(
			ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAdd");
		writeSEL.append(std::string("FAST THROTTLING DeAsserted"),
						DBUS_OBJPATH_FAST_THROTTLING,
						eventData, gpioState, genId);
		try
		{
			bus.call(writeSEL);
		}
		catch (sdbusplus::exception_t &e)
		{
			std::cerr << "Failed to log SEL\n";
		}
	}


	return 0;
}
