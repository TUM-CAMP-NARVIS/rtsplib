﻿// PHZ
// 2018-9-30

#include "rtsplib/server/xop/MediaSession.h"
#include "rtsplib/server/xop/RtpConnection.h"
#include <cstring>
#include <ctime>
#include <map>
#include <forward_list>
#include "rtsplib/server/net/Logger.h"
#include "rtsplib/server/net/NetInterface.h"
#include "rtsplib/server/net/SocketUtil.h"

#define GOP_CACHE_ON 1

using namespace xop;
using namespace std;

std::atomic_uint MediaSession::_lastMediaSessionId(1);

MediaSession::MediaSession(std::string rtspUrlSuffxx)
    : _suffix(rtspUrlSuffxx)
    , _mediaSources(2)
    , _buffer(2)
{
    _gopCacheEnabled = false;
    _hasNewClient = false;
    _sessionId = ++_lastMediaSessionId;

    for(int n=0; n<MAX_MEDIA_CHANNEL; n++)
    {
        _multicastPort[n] = 0;
    }
}

MediaSession* MediaSession::createNew(std::string rtspUrlSuffxx)
{
    return new MediaSession(std::move(rtspUrlSuffxx));
}

MediaSession::~MediaSession()
{
	if (_multicastIp != "")
	{
		MulticastAddr::instance().release(_multicastIp);
	}
}

bool MediaSession::addMediaSource(MediaChannelId channelId, MediaSource* source)
{
    source->setSendFrameCallback([this](MediaChannelId channelId, RtpPacket pkt) {
        std::forward_list<std::shared_ptr<RtpConnection>> clients;
        std::map<int, RtpPacket> packets;
        {
            std::lock_guard<std::mutex> lock(_mtxMap);
            for (auto iter = _clients.begin(); iter != _clients.end();)
            {
                auto conn = iter->second.lock();
                if (conn == nullptr)
                {
                    _clients.erase(iter++);
                }
                else
                {				
                    int id = conn->getId();
                    if (packets.find(id) == packets.end())
                    {
                        RtpPacket tmpPkt;
                        if (packets.size() != 0)
                        {
                            memcpy(tmpPkt.data.get(), pkt.data.get(), pkt.size);
                        }
                        else
                        {
                            tmpPkt.data = pkt.data;
                        }
                            
                        tmpPkt.size = pkt.size;
                        tmpPkt.last = pkt.last;
                        tmpPkt.timestamp = pkt.timestamp;
                        tmpPkt.type = pkt.type;
                        packets.emplace(id, tmpPkt);
                    }
                    clients.emplace_front(conn);
                    iter++;
                }
            }
        }
        
        int count = 0;
        for(auto iter : clients)
        {
            int ret = 0;
            int id = iter->getId();
            auto iter2 = packets.find(id);
            if (iter2 != packets.end())
            {
                if (_gopCacheEnabled)
                {
                    if (iter->hasGOPFrame() || iter->hasIDRFrame())
                        continue;
                }

                count++;
                ret = iter->sendRtpPacket(channelId, iter2->second, _gopCacheEnabled);
                if (_isMulticast && ret==0)
                    break; 
            }						
        }

        if (_gopCacheEnabled && count==0)
            return false; 

        return true;
    });

    _mediaSources[channelId].reset(source);
    return true;
}

bool MediaSession::removeMediaSource(MediaChannelId channelId)
{
    _mediaSources[channelId] = nullptr;
    return true;
}

bool MediaSession::startMulticast()
{  
    if (_isMulticast)
    {
        return true;
    }

    _multicastIp = MulticastAddr::instance().getAddr();
    if (_multicastIp == "")
    {
        return false;
    }

    std::random_device rd;
    _multicastPort[channel_0] = htons(rd() & 0xfffe);
    _multicastPort[channel_1] = htons(rd() & 0xfffe);

    _isMulticast = true;
    return true;
}

std::string MediaSession::getSdpMessage(std::string sessionName)
{
    if(_sdp != "")
        return _sdp;

    if(_mediaSources.empty())
        return "";
            
    std::string ip = NetInterface::getLocalIPAddress();
    char buf[2048] = {0};

    snprintf(buf, sizeof(buf),
            "v=0\r\n"
			"o=- 9%ld 1 IN IP4 %s\r\n"
            "t=0 0\r\n"
            "a=control:*\r\n" ,
            (long)std::time(NULL), ip.c_str()); 

    if(sessionName != "")
    {
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), 
                "s=%s\r\n",
                sessionName.c_str());
    }
    
    if(_isMulticast)
    {
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf),
                 "a=type:broadcast\r\n"
                 "a=rtcp-unicast: reflection\r\n");
    }
		
    for (uint32_t chn=0; chn<_mediaSources.size(); chn++)
    {
        if(_mediaSources[chn])
        {	
            if(_isMulticast)		 
            {
                snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), 
                        "%s\r\n",
                        _mediaSources[chn]->getMediaDescription(_multicastPort[chn]).c_str()); 
                     
                snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), 
                        "c=IN IP4 %s/255\r\n",
                        _multicastIp.c_str()); 
            }
            else 
            {
                snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), 
                        "%s\r\n",
                        _mediaSources[chn]->getMediaDescription(0).c_str());
            }
            
			snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), 
					"%s\r\n",
					_mediaSources[chn]->getAttribute().c_str());
                     
			snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf),											
					"a=control:track%d\r\n", chn);	

            snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf),                                          
                    "%s\r\n", _sdp_addon.c_str());  
        }
    }

    _sdp = buf;
    return _sdp;
}

MediaSource* MediaSession::getMediaSource(MediaChannelId channelId)
{
	if (_mediaSources[channelId])
	{
		return _mediaSources[channelId].get();
	}

    return nullptr;
}

bool MediaSession::handleFrame(MediaChannelId channelId, AVFrame frame)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(_mediaSources[channelId])
    {
        _mediaSources[channelId]->handleFrame(channelId, frame);
#if GOP_CACHE_ON
        if (!_isMulticast)
        {
            uint32_t mediaType = _mediaSources[channelId]->getMediaType();
            if (mediaType == H264 || mediaType == H265)
            {
                if (frame.type == VIDEO_FRAME_I)
                {					
                    _gopCache = frame;
                    _gopCache.type = kGOP;
                }

                if (_hasNewClient  && _gopCache.size>0)
                {
                    _hasNewClient = false;
                    _gopCacheEnabled = true;
                    _gopCache.timestamp = H264Source::getTimeStamp();
                    _mediaSources[channelId]->handleFrame(channelId, _gopCache);
                    _gopCacheEnabled = false;
                }
            }
        }
#endif
    }
    else
    {
        return false;
    }

    return true;
}

bool MediaSession::addClient(SOCKET rtspfd, std::shared_ptr<RtpConnection> rtpConnPtr)
{
    std::lock_guard<std::mutex> lock(_mtxMap);
    auto iter = _clients.find (rtspfd);
    if(iter == _clients.end())
    {
        std::weak_ptr<RtpConnection> rtpConnWeakPtr = rtpConnPtr;
        _clients.emplace(rtspfd, rtpConnWeakPtr);
        if (_notifyCallback)
        {
            _notifyCallback(_sessionId, _clients.size());
        }
        
        _hasNewClient = true;
        return true;
    }
            
    return false;
}

void MediaSession::removeClient(SOCKET rtspfd)
{  
    std::lock_guard<std::mutex> lock(_mtxMap);
    if (_clients.find(rtspfd) != _clients.end())
    {
        _clients.erase(rtspfd);
        if (_notifyCallback)
        {
            _notifyCallback(_sessionId, _clients.size()); 
        }
    }
}

