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

int main(int argc, char* argv[]){
	auto bus = sdbusplus::bus::new_default();
	string tarGPIO = argv[1];
	uint16_t genId = 0x20;
	bool assert = true;

	/* Formats are only XXXX_ASSERTED and XXXX_DEASSERTED
	 * rfind to avoid XXXX_ASSERTED_XXXX and XXXX_DEASSERTED_XXXX
	 */
	if(tarGPIO.rfind(DEKeyWord) != string::npos && 
	   tarGPIO.rfind(DEKeyWord) >= (tarGPIO.length() - DEKeyWord.length())){
		assert = false;
		tarGPIO = tarGPIO.substr(0, tarGPIO.rfind(DEKeyWord));
	}else if(tarGPIO.rfind(ASKeyWord) != string::npos && 
	   	 tarGPIO.rfind(ASKeyWord) >= (tarGPIO.length() - ASKeyWord.length())){
		assert = true;
		tarGPIO = tarGPIO.substr(0, tarGPIO.rfind(ASKeyWord));
	}else{
		cout << "ERROR: No ASSERTED or DEASSERTED within target." << endl;
		return 0;
	}

   	std::vector<uint8_t> eventData(selEvtDataMaxSize, 0xFF);
    	string errorSensorPath = "/xyz/openbmc_project/sensors/discrete/processor/";
	
	/* append sensor name */
	errorSensorPath +=  tarGPIO;
	cout << "Sensor path: " << errorSensorPath << endl;

	/* asseted or deasserted according state */
	if(assert){
		eventData[0] = 0x01;
	}else{
		eventData[0] = 0x00;
	}
    
        sdbusplus::message::message writeMcerrSEL = bus.new_method_call(
                     ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAdd");
        writeMcerrSEL.append(ipmiSELAddMessage,  errorSensorPath, eventData, true, genId);
        try{
            	bus.call(writeMcerrSEL);
        }catch (sdbusplus::exception_t& e){
            	std::cerr << "Failed to log SEL\n";
        }     
	
	return 0;
}
