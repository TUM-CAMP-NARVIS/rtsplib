#pragma once

#include "rtsplib/rtsplib_common.h"
#include "rtsplib/client/RtspPlayer.h"
#include <thread>
#include <memory>
#include <functional>
#include <queue>
#include <tuple>

#include "rtsplib/rtsplib_config.h"

/* **********************************************************************************
#                                                                                   #
# Copyright (c) 2019,                                                               #
# Research group MITI                                                               #
# Technical University of Munich                                                    #
#                                                                                   #
# All rights reserved.                                                              #
# Kevin Yu - kevin.yu@tum.de                                                        #
#                                                                                   #
# Redistribution and use in source and binary forms, with or without                #
# modification, are restricted to the following conditions:                         #
#                                                                                   #
#  * The software is permitted to be used internally only by the research group     #
#    MITI and CAMPAR and any associated/collaborating groups and/or individuals.    #
#  * The software is provided for your internal use only and you may                #
#    not sell, rent, lease or sublicense the software to any other entity           #
#    without specific prior written permission.                                     #
#    You acknowledge that the software in source form remains a confidential        #
#    trade secret of the research group MITI and therefore you agree not to         #
#    attempt to reverse-engineer, decompile, disassemble, or otherwise develop      #
#    source code for the software or knowingly allow others to do so.               #
#  * Redistributions of source code must retain the above copyright notice,         #
#    this list of conditions and the following disclaimer.                          #
#  * Redistributions in binary form must reproduce the above copyright notice,      #
#    this list of conditions and the following disclaimer in the documentation      #
#    and/or other materials provided with the distribution.                         #
#  * Neither the name of the research group MITI nor the names of its               #
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

namespace rtsplib
{


    using RecvCallFn = std::function<void(uint8_t* data, size_t size, uint64_t timestamp)>;
    using StartStreammCallFn = std::function<void(int width, int height, int bytesPerPixel)>;

	class RTSPLIB_EXPORT ClientPipeRTSP
	{
	public:
		ClientPipeRTSP(std::string rtspAddress);
		~ClientPipeRTSP();

		bool startClient(int rtp_port = 12000, int rtcp_port = 13000);
        void stopClient();


        void setFrameCallback(RecvCallFn cb)
        {
            m_recv_cb = cb;
        }

        void setStartStreamCallback(StartStreammCallFn cb)
        {
            m_start_stream_cb = cb;
        }


        int getWidth() {
            return m_width;
        }

        int getHeight() {
            return m_height;
        }

        int getBytesPerPixel() {
            return m_bytesPerPixel;
        }

        bool isRunning() {
            return m_isRunning;
        }

	private:

		int cvtBuffer(uint8_t *buf, ssize_t bufsize);

        RecvCallFn m_recv_cb{nullptr};
        StartStreammCallFn m_start_stream_cb{nullptr};
        std::string m_rtspAddress;

        int m_height{0};
        int m_width{0};
        int m_bytesPerPixel{0};
        int m_dataSize{0};
        uint8_t* m_frameBuffer{nullptr};

        bool m_isRunning = false;

        uint32_t m_currentTimestamp = 0;
		uint8_t m_currentFrameCounter = 0;
		bool m_pkgCorrupted = false;
		int m_currentOffset = 0;

		RK::RtspPlayer::Ptr m_player;

	};
}
