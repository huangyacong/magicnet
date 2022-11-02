#ifndef __SE_WSFRAME_H__
#define __SE_WSFRAME_H__

#include "SeNetEngine.h"
#include <string>

using namespace std;

class SeWSFrame
{
	enum 
	{
		MIN_FRAME_LEN = 2,				// 最小帧大小
		MIN_FRAME_ENGTH_16_LEN = 2,		// 之后的2个字节,最小帧大小
		MIN_FRAME_ENGTH_64_LEN = 8,		// 之后的8个字节,最小帧大小
		MIN_FRAME_MASK_LEN = 4,			// 掩码最小帧大小
		PAYLOAD_LENGTH_16_LIMIT = 126,	// 之后的2个字节
		PAYLOAD_LENGTH_64_LIMIT = 127,	// 之后的8个字节
	};

	enum FRAME_STATE
	{
		STATE_BASE_HEADER_ING = 0,		// 正在获取基础头部状态
		STATE_OTHER_HEADER_ING,			// 正在获取其他头部状态
		STATE_HEADER_COMPELET,			// 获取了完整的帧头部数据状态
		STATE_FRAME_COMPELET,			// 已经获了完整的帧数据
	};

	enum BASE_FRAME_BIT
	{
		FIN = 0x80,						// 如果是1，表示这是消息的最后一个分片
		RSV1 = 0x40,					// 目前需要填充零
		RSV2 = 0x20,					// 目前需要填充零
		RSV3 = 0x10,					// 目前需要填充零
		OPCODE = 0xF00,					// Opcode值含义（16进制）
		MASK = 0x80,					// 掩码
		PAYLOAD_LEN = 0x7F,				// 负载长度
	};

public:

	enum OP_CODE
	{
		OP_CONTINUATION = 0x0,			// 继续帧，表示消息分片模式
		OP_TEXT = 0x1,					// 文本帧，表示文本格式传输
		OP_BINARY = 0x2,				// 二进制帧，表示二进制格式传输
		OP_CLOSE = 0x8,					// 关闭帧，表示关闭连接
		OP_PING = 0x9,					// Ping帧，一般主动发送ping给对方，确认对方状态
		OP_PONG = 0xA,					// Pong帧，一般发送了ping给对方,对方就回复pong
	};

public:
	SeWSFrame(SeNetEngine* pkSeNetEngine);
	virtual ~SeWSFrame();
public:
	bool PushData(const char *pcBuf, int iLen);
	bool IsFrameCompelet();
private:
	bool __GetFin(unsigned char ucHeader);
	bool __GetRsv1(unsigned char ucHeader);
	bool __GetRsv2(unsigned char ucHeader);
	bool __GetRsv3(unsigned char ucHeader);
	unsigned char __GetOpcode(unsigned char ucHeader);
	bool __GetMask(unsigned char ucHeader);
	unsigned char __GetPayloadLen(unsigned char ucHeader);
private:
	void SetFrameBaseHeader();
	bool GetExtendPayloadLen(int& riLen);
	string GetMaskKey();
private:
	int m_iFin;
	int m_iRSV1;
	int m_iRSV2;
	int m_iRSV3;
	int m_iOpcode;
	int m_iMask;
	int m_iPayloadLen;
	int m_iRealPayloadLen;
	string m_strMaskingKey;
	FRAME_STATE m_eState;
private:
	string m_strFrame;
	SENETSTREAM m_kRecvNetStream;
	SeNetEngine* m_pkSeNetEngine;
};

#endif


