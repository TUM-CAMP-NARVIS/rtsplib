// PHZ
// 2018-6-11

#ifndef XOP_RTP_H
#define XOP_RTP_H

#include <memory>
#include <cstdint>

#include "rtsplib/rtsplib_network.h"

namespace xop
{

enum TransportMode
{
    RTP_OVER_TCP = 1,
    RTP_OVER_UDP = 2,
    RTP_OVER_MULTICAST = 3,
};

typedef struct _RTP_header
{
    unsigned char csrc:4;
    unsigned char extension:1;
    unsigned char padding:1;
    unsigned char version:2;
    unsigned char payload:7;
    unsigned char marker:1;

    unsigned short seq;
    unsigned int   ts;
    unsigned int   ssrc;
} RtpHeader;

struct MediaChannelInfo
{
    RtpHeader rtpHeader;

    // tcp
    uint16_t rtpChannel;
    uint16_t rtcpChannel;

    // udp
    uint16_t rtpPort;
    uint16_t rtcpPort;
    uint16_t packetSeq;
    uint32_t clockRate;

    // rtcp
    uint64_t packetCount;
    uint64_t octetCount;
    uint64_t lastRtcpNtpTime;

    bool isSetup;
    bool isPlay;
    bool isRecord;
};

struct RtpPacket
{
	RtpPacket()
		: data(new uint8_t[1600])
	{
		type = 0;
	}

	std::shared_ptr<uint8_t> data;
	uint32_t size;
	uint32_t timestamp;
	uint8_t type;
	uint8_t last;
};

}

#endif
