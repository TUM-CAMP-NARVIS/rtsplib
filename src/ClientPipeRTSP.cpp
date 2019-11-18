#include "rtsplib/rtsplib_common.h"
#include "rtsplib/ClientPipeRTSP.h"

#include "spdlog/spdlog.h"

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

using namespace std;
using namespace rtsplib;

ClientPipeRTSP::ClientPipeRTSP(std::string _rtspAddress)
	: m_rtspAddress(_rtspAddress) {}

ClientPipeRTSP::~ClientPipeRTSP() {
    if (m_frameBuffer != nullptr) {
        free(m_frameBuffer);
    }
}

bool ClientPipeRTSP::startClient() {
	m_player = std::make_shared<RK::RtspPlayer>([&](uint8_t *buffer, ssize_t bufferLength)
		{

			if (!m_isRunning) return;

			uint8_t frameCounter = buffer[3];

			// Get Nalu type (0 if header, 28 if datapackage)
			// Not pretty, since it is called in cvtBuffer again, but it does what it should.
			struct RK::Nalu nalu = *(struct RK::Nalu *)(buffer + RTP_OFFSET);
			int type = nalu.type;

			// New NAL package found, submit previous to decoder
			// There are two header packages per NAL, we need both for a complete package
			// Both header pkgs have a very destinct length. The thresholds seem to be randomly chosen
			// but are actually the min and max possible length of the first header package.
			// 2nd header pkg usually has 20 bytes
			if (bufferLength > 35 && bufferLength < 42 && type == 0)
			{

				// only decode, if the previous package is not corrupted because of missing subpackages.
				if (!m_pkgCorrupted && frameCounter == ((m_currentFrameCounter + 1) % 256) && m_prevPkgSize < MAX_RTP_PAYLOAD_SIZE + RTP_HEADER_SIZE)
				{
                    if (m_recv_cb != nullptr) {
                        m_recv_cb(m_frameBuffer, m_currentOffset, m_currentTimestamp);
                    } else {
                        spdlog::warn("Received frame, but no callback was registered.");
                    }
				}
				else
				{
					spdlog::warn("frame was skipped");
				}

				// Reset variables for new frame
				m_currentOffset = 0;
				m_pkgCorrupted = false;

				m_currentTimestamp = (buffer[4] << 24) | (buffer[5] << 16) | (buffer[6] << 8) | buffer[7];
			}
			else
			{
				// inbetween NAL package.
				if (m_pkgCorrupted) return;

				if (frameCounter != ((m_currentFrameCounter + 1) % 256))
				{
					m_pkgCorrupted = true;
					return;
				}
			}
			// Store subpackage into framebuffer
			m_currentFrameCounter = frameCounter;
			ssize_t rdLength;
			cvtBuffer(buffer, bufferLength, &m_frameBuffer[m_currentOffset], &rdLength);
			m_currentOffset += rdLength;
			m_prevPkgSize = bufferLength;

		}, "Stream");

	m_player->imgPropRdy_cb = [&](int width, int height, int bytesPerPixel)
	{
        if (width == 0 || height == 0 || bytesPerPixel == 0)
            return;

	    m_width = width;
	    m_height = height;
	    m_bytesPerPixel = bytesPerPixel;

        m_dataSize = m_width * m_height * m_bytesPerPixel;
        m_frameBuffer = (uint8_t *)malloc(m_dataSize);

        if (m_start_stream_cb != nullptr) {
            m_start_stream_cb(width, height, bytesPerPixel);
        } else {
            spdlog::warn("Client connected but no start-stream callback was registered");
        }
	};

    m_isRunning = true;
	m_player->Play(m_rtspAddress.c_str());
}



int ClientPipeRTSP::cvtBuffer(uint8_t *buf, ssize_t bufsize, uint8_t *outBuf, ssize_t *outLength)
{
	uint8_t header[] = { 0, 0, 0, 1 };
	struct RK::Nalu nalu = *(struct RK::Nalu *)(buf + RTP_OFFSET);
	if (nalu.type >= 0 && nalu.type < 24)
	{ //one nalu
		*outLength = bufsize - RTP_OFFSET;
		memcpy(outBuf, buf + RTP_OFFSET, bufsize - RTP_OFFSET);
	}
	else if (nalu.type == 28)
	{ //fu-a slice
		struct RK::FU fu;
		uint8_t in = buf[RTP_OFFSET + 1];
		fu.S = in >> 7;
		fu.E = (in >> 6) & 0x01;
		fu.R = (in >> 5) & 0x01;
		fu.type = in & 0x1f;
		if (fu.S == 1)
		{
			uint8_t naluType = nalu.forbidden_zero_bit << 7 | nalu.nal_ref_idc << 5 | fu.type;
			*outLength = 4 + 1 + bufsize - FU_OFFSET;
			memcpy(outBuf, header, 4);
			memcpy(&outBuf[4], &naluType, 1);
			memcpy(&outBuf[5], buf + FU_OFFSET, bufsize - FU_OFFSET);
		}
		else
		{
			*outLength = bufsize - FU_OFFSET;
			memcpy(outBuf, buf + FU_OFFSET, bufsize - FU_OFFSET);
		}
	}
	return nalu.type;
}

void ClientPipeRTSP::stopClient()
{
    if (!m_isRunning) {
        return;
    }
	m_isRunning = false;
	m_player->Stop();
	m_player.reset();
}

