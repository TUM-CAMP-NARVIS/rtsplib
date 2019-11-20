/* **********************************************************************************
#                                                                                   #
# Copyright (c) 2019,                                                               #
# Research group CAMP                                                               #
# Technical University of Munich                                                    #
#                                                                                   #
# All rights reserved.                                                              #
# Ulrich Eck - ulrich.eck@tum.de                                                    #
#                                                                                   #
# Redistribution and use in source and binary forms, with or without                #
# modification, are restricted to the following conditions:                         #
#                                                                                   #
#  * The software is permitted to be used internally only by the research group     #
#    CAMP and any associated/collaborating groups and/or individuals.               #
#  * The software is provided for your internal use only and you may                #
#    not sell, rent, lease or sublicense the software to any other entity           #
#    without specific prior written permission.                                     #
#    You acknowledge that the software in source form remains a confidential        #
#    trade secret of the research group CAMP and therefore you agree not to         #
#    attempt to reverse-engineer, decompile, disassemble, or otherwise develop      #
#    source code for the software or knowingly allow others to do so.               #
#  * Redistributions of source code must retain the above copyright notice,         #
#    this list of conditions and the following disclaimer.                          #
#  * Redistributions in binary form must reproduce the above copyright notice,      #
#    this list of conditions and the following disclaimer in the documentation      #
#    and/or other materials provided with the distribution.                         #
#  * Neither the name of the research group CAMP nor the names of its               #
#    contributors may be used to endorse or promote products derived from this      #
#    software without specific prior written permission.                            #
#                                                                                   #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND   #
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED     #
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE            #
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR   #
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES    #
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;      #
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND       #
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT        #
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS     #
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                      #
#                                                                                   #
*************************************************************************************/


#include "gtest/gtest.h"
#include "spdlog/spdlog.h"

#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <tuple>

#include "rtsplib/ClientPipeRTSP.h"
#include "rtsplib/ServerPipeRTSP.h"



TEST(RtsplibTestSuite, SimpleConnectMultipleClientsTest){
    using namespace std::chrono;
    spdlog::set_level(spdlog::level::debug);


    spdlog::set_level(spdlog::level::trace);

// reads example stream from nvpipe nvExampleFile (stream.bin)
    const uint32_t width = 3840;
    const uint32_t height = 2160;
    const uint32_t bytesPerPixel = 4;
    std::vector<uint8_t> compressed(width * height * bytesPerPixel);

    std::string fname("../tests/stream.bin");
    std::string rtspUrl("rtsp://127.0.0.1:44444/live");

    auto server = std::make_shared<rtsplib::ServerPipeRTSP>("127.0.0.1", 44444);
    server->startServer(width, height, bytesPerPixel);

    // @todo: server segfaults if sending starts immediately ??
    std::this_thread::sleep_for(std::chrono::seconds(1));


    int num_clients{12};
    std::vector<std::shared_ptr<rtsplib::ClientPipeRTSP>> clients;

    std::atomic_int num_init_callbacks{0};
    std::atomic_int num_frame_callbacks{0};

    for (int i=0; i<num_clients; i++) {
        auto client = std::make_shared<rtsplib::ClientPipeRTSP>(rtspUrl);

        auto initCB = [&](int width, int height, int bytesPerPixel) {
            num_init_callbacks++;
        };

        auto frameCB = [&](uint8_t* data, size_t size, uint64_t timestamp) {
            num_frame_callbacks++;
        };

        client->setFrameCallback(frameCB);
        client->setStartStreamCallback(initCB);

        spdlog::info("Starting client");
        client->startClient();

        clients.push_back(client);
    }


    std::this_thread::sleep_for(std::chrono::seconds(1));

    spdlog::info("Enter Mainloop.");
    int frame_counter{0};

    while (frame_counter < 100) {

        std::ifstream in(fname, std::ios::in | std::ios::binary);
        ASSERT_FALSE(!in);

        for (uint32_t i = 0; i < 10; ++i) {
            uint64_t size;
            in.read((char *) &size, sizeof(uint64_t));
            in.read((char *) compressed.data(), size);

            server->sendFrame(compressed.data(), size, 0);
            ++frame_counter;
        }

        in.close();
    }

    // @todo: segfaults if sending starts immediately ??
    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT_EQ(num_init_callbacks, num_clients);
    EXPECT_EQ(num_frame_callbacks, num_clients*frame_counter);

    for (auto& client : clients) {
        spdlog::info("Stopping client");
        client->stopClient();
        client.reset();
    }

    spdlog::info("Stopping server");
    server->stopServer();

    server.reset();
    spdlog::info("Finished test");


}


