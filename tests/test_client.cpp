//
// Created by netlabs on 11/14/19.
//
#include <thread>
#include <chrono>

#include "rtsplib/ClientPipeRTSP.h"
#include "spdlog/spdlog.h"

int main(int argc, const char *argv[]) {
    spdlog::set_level(spdlog::level::trace);

    if (argc < 2) {
        spdlog::error("Usage: test_client rtsp://<ip>:<port>/<stream>");
        return 1;
    }

    std::string rtspUrl(argv[1]);

    auto client = std::make_shared<rtsplib::ClientPipeRTSP>(rtspUrl);

    auto initCB = [](int width, int height, int bytesPerPixel) {
        spdlog::info("Received start of stream: width: {0}, height: {1}, bytesPerPixel: {2}", width, height, bytesPerPixel);
    };

    auto frameCB = [](uint8_t* data, size_t size, uint64_t timestamp) {
        spdlog::info("Received frame ts: {0}, data length: {1}", timestamp, size);
    };

    client->setFrameCallback(frameCB);
    client->setStartStreamCallback(initCB);

    spdlog::info("Starting client");
    client->startClient();

    spdlog::info("Receiving for 30 seconds.");
    std::this_thread::sleep_for (std::chrono::seconds(30));

    spdlog::info("Stopping client");
    client->stopClient();

    spdlog::info("Finished test");
    client.reset();

    return 0;
}