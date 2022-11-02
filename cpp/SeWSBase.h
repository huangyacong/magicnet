#ifndef __SE_WSBASE_H__
#define __SE_WSBASE_H__

#include "SeNetEngine.h"
#include <vector>

using namespace std;

class SeWSBase
{
public:

	enum 
	{
		WEB_SOCKET_VERSION = 13,		// 版本
		MIN_FRAME_LEN = 2,				// 最小帧大小
		MIN_FRAME_MASK_LEN = 4,			// 掩码最小帧大小
		MIN_FRAME_ENGTH_16_LEN = 2,		// 之后的2个字节,最小帧大小
		MIN_FRAME_ENGTH_64_LEN = 8,		// 之后的8个字节,最小帧大小
		PAYLOAD_LENGTH_16_LIMIT = 126,	// 之后的2个字节
		PAYLOAD_LENGTH_64_LIMIT = 127,	// 之后的8个字节
	};

	enum BASE_FRAME_BIT
	{
		FIN = 0x80,
		RSV1 = 0x40,
		RSV2 = 0x20,
		RSV3 = 0x10,
		OPCODE = 0xF00,
		MASK = 0x80,
		PAYLOAD_LEN = 0x7F,
	};

	enum OP_CODE
	{
		OP_CONTINUATION = 0x0,			// 继续帧，表示消息分片模式
		OP_TEXT = 0x1,					// 文本帧，表示文本格式传输
		OP_BINARY = 0x2,				// 二进制帧，表示二进制格式传输
		OP_CLOSE = 0x8,					// 关闭帧，表示关闭连接
		OP_PING = 0x9,					// Ping帧，一般主动发送ping给对方，确认对方状态
		OP_PONG = 0xA,					// Pong帧，一般发送了ping给对方,对方就回复pong
	};

	struct WSFrame
	{
		int iFin;
		int iRSV1;
		int iRSV2;
		int iRSV3;
		int iOpcode;
		int iMask;
		int iPayloadLen;
		int iRealPayloadLen;
		unsigned int uiMaskingKey;
	};

public:
	SeWSBase(SeNetEngine* pkSeNetEngine, HSOCKET kHSocket, const string& strIP);
	~SeWSBase();
public:
	HSOCKET GetHSocket() { return m_kHSocket; }
	bool IsHandShake() { return m_bHandShake; }
	void SetHandShake() { m_bHandShake = true; }
	const string& GetIP() { return m_strIP; }
public:
	bool ServerHandShake(bool& bHandShakeOK);
public:
	bool PushRecvData(const char *pcBuf, int iLen);
	void PushFrameHeader(const char *pcBuf, int iLen);
	bool IsFrameHeaderCompelet(WSFrame &rkWsFrame, int& riNeedLen);
private:
	bool __GetFin(unsigned char ucHeader) { return ((unsigned int)ucHeader & BASE_FRAME_BIT::FIN) > 0; }
	bool __GetRsv1(unsigned char ucHeader) { return ((unsigned int)ucHeader & BASE_FRAME_BIT::RSV1) > 0; }
	bool __GetRsv2(unsigned char ucHeader) { return ((unsigned int)ucHeader & BASE_FRAME_BIT::RSV2) > 0; }
	bool __GetRsv3(unsigned char ucHeader) { return ((unsigned int)ucHeader & BASE_FRAME_BIT::RSV3) > 0; }
	unsigned char __GetOpcode(unsigned char ucHeader) { return (unsigned char)((unsigned int)ucHeader & BASE_FRAME_BIT::OPCODE); }
	bool __GetMask(unsigned char ucHeader) { return ((unsigned int)ucHeader & BASE_FRAME_BIT::MASK) > 0; }
	unsigned char __GetPayloadLen(unsigned char ucHeader) { return (unsigned char)((unsigned int)ucHeader & BASE_FRAME_BIT::PAYLOAD_LEN); }
public:
	string m_strIP;
	HSOCKET m_kHSocket;
	bool m_bHandShake;
	SENETSTREAM m_kRecvNetStream;
	SENETSTREAM m_kPacketNetStream;
	SeNetEngine* m_pkSeNetEngine;
	vector<unsigned char> m_vecFrame;
	map<string, string> m_mapHeadrs;
};

#endif


