//
// Created by netlabs on 11/14/19.
//


//
// Created by netlabs on 11/14/19.
//

//
// Created by netlabs on 11/14/19.
//
#include <thread>
#include <chrono>
#include <csignal>

#include <iostream>
#include <vector>
#include <fstream>

#include "rtsplib/ServerPipeRTSP.h"
#include "spdlog/spdlog.h"


namespace
{
    volatile std::sig_atomic_t gSignalStatus{0};
}

void signal_handler(int signal)
{
    gSignalStatus = signal;
}

int main(int argc, const char *argv[]) {
    spdlog::set_level(spdlog::level::trace);

    // reads example stream from nvpipe nvExampleFile (stream.bin)
    const uint32_t width = 3840;
    const uint32_t height = 2160;
    const uint32_t bytesPerPixel = 4;
    std::vector<uint8_t> compressed(width * height * bytesPerPixel);

    if (argc < 2) {
        spdlog::error("Usage: test_server <inputfile>");
        return 1;
    }
    std::string fname(argv[1]);

    std::signal(SIGINT, signal_handler);

    auto server = std::make_shared<rtsplib::ServerPipeRTSP>("127.0.0.1", 44444);
    server->startServer(width, height, bytesPerPixel);

    // @todo: segfaults if sending starts immediately ??
    std::this_thread::sleep_for(std::chrono::seconds(1));

    spdlog::info("Enter Mainloop.");
    while (gSignalStatus == 0) {

        std::ifstream in(fname, std::ios::in | std::ios::binary);
        if (!in)
        {
            spdlog::error("Error: Failed to open input file: {0}", fname);
            return 1;
        }

        for (uint32_t i = 0; i < 10; ++i)
        {
            uint64_t size;
            in.read((char*) &size, sizeof(uint64_t));
            in.read((char*) compressed.data(), size);

            server->sendFrame(compressed.data(), size, 0);
        }

        in.close();
    }

    spdlog::info("Stopping server");
    server->stopServer();

    spdlog::info("Finished test");
    server.reset();

    return 0;
}

