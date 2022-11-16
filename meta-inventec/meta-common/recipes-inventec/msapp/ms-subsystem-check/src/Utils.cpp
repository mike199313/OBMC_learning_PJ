/*
// Copyright (c) 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include <Utils.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/container/flat_map.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/bus/match.hpp>

#include <filesystem>
#include <fstream>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

void ipmi_method_call(uint8_t& lun, uint8_t& netfn, uint8_t& cmd,
                      std::vector<uint8_t>& cmdParameter,
                      std::shared_ptr<DbusRspData> convey)
{
    try
    {

        boost::asio::io_service io;
        auto bus = std::make_shared<sdbusplus::asio::connection>(io);

        boost::asio::deadline_timer waitTimer(io);
        waitTimer.expires_from_now(boost::posix_time::milliseconds(5000));
        waitTimer.async_wait([&io, convey](const boost::system::error_code& ec) {
            fprintf(stderr, "ipmi_method_call timer expired ec=%d\n", ec.value());
            convey->retData.push_back(0xFE);
            convey->ec = ec;
            io.stop();
            fprintf(stderr, "%s:%d\n", __func__, __LINE__);
        });

        // do it as IPC, at channel 8.
        std::map<std::string, IpmiDbusValueType> options;

        bus->async_method_call(
            [&io, &waitTimer, convey](const boost::system::error_code& ec,
                                    const IpmiDbusRspType& response) {
                std::vector<uint8_t> payload;
                if (!ec)
                {
                    const uint8_t& cc = std::get<3>(response);
                    const std::vector<uint8_t>& responseData =
                        std::get<4>(response);
                    payload.reserve(1 + responseData.size());
                    payload.push_back(cc);
                    payload.insert(payload.end(), responseData.begin(),
                                responseData.end());
                }
                else
                {
                    fprintf(stderr, "ipmi_method_call ec=%d\n", ec.value());
                    // IPMI_CC_UNSPECIFIED_ERROR
                    payload.push_back(0xFF);
                }
                convey->retData = std::move(payload);
                convey->ec = ec;
                waitTimer.cancel();
                io.stop();
            },
            "xyz.openbmc_project.Ipmi.Host", "/xyz/openbmc_project/Ipmi",
            "xyz.openbmc_project.Ipmi.Server", "execute", netfn, lun, cmd,
            cmdParameter, options);

        io.run();
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "Exception:%s \n", e.what());
        convey->retData.clear();
        convey->retData.push_back(0xFF);
    }
}
