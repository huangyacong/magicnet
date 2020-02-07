local net_module = require "ccorenet"
local CoreTool = require "CoreTool"
local CoreNet = require "CoreNet"
local util = require "util"
require "class"

local IClientNetFunc_OnPing = "OnPing"
local IClientNetFunc_OnRecv = "OnRecv"
local IClientNetFunc_OnConnect = "OnConnect"
local IClientNetFunc_OnDisConnect = "OnDisConnect"
local IClientNetFunc_OnConnectFailed = "OnConnectFailed"
local IClientNetFunc_OnSendPacketAttach = "OnSendPacketAttach"

-- 重连间隔时间,没次无法连接，就加上这个时间，几次之后，立马重连
local iReConnectDelayTime = 1000
-- 重连循环次数
local iReConnectCount = 10
-- ping的时间间隔
local iPingTimeDelay  = 1000 * 2

local IClientPlayerClass = class()

function IClientPlayerClass:ctor(className, modulename, cIP, iPort, iTimeOut, iConnectTimeOut, bClinetFormat, iDomain, bNoDelay)
	self.hsocket = 0
	self.className = tostring(className)
	self.modulename = modulename

	self.cIP = cIP
	self.iPort = iPort
	self.iTimeOut = iTimeOut
	self.iConnectTimeOut = iConnectTimeOut
	self.bClinetFormat = bClinetFormat
	self.iDomain = iDomain
	self.bNoDelay = bNoDelay

	self.m_iReConnectNum = 0
	self.m_ullReConnectTime = CoreTool.GetTickCount()
	self.m_ullPingTIme = CoreTool.GetTickCount()
end

function IClientPlayerClass:del()-- 剔除各个变量
	assert(self.hsocket == 0)
	
	self.hsocket = 0
	self.className = 0
	self.modulename = 0

	self.cIP = 0
	self.iPort = 0
	self.iTimeOut = 0
	self.iConnectTimeOut = 0
	self.bClinetFormat = 0
	self.iDomain = iDomain
	self.bNoDelay = 0

	self.m_iReConnectNum = 0
	self.m_ullReConnectTime = 0
	self.m_ullPingTIme = 0
end

function IClientPlayerClass:ResetSocketData(cIP, iPort, iTimeOut, iConnectTimeOut, bClinetFormat, bNoDelay)
	self.cIP = cIP
	self.iPort = iPort
	self.iTimeOut = iTimeOut
	self.iConnectTimeOut = iConnectTimeOut
	self.bClinetFormat = bClinetFormat
	self.bNoDelay = bNoDelay
end

function IClientPlayerClass:GetName()
	return self.className
end

function IClientPlayerClass:Connect()
	-- 模块modulename中必须是table，同时必须有下面的key

	if type(self.modulename) ~= type({}) then
		print("IClientPlayerClass Listen modulename not a table")
		return false
	end

	if not next(self.modulename) then
		print(string.format("IClientPlayerClass modulename=%s is empty", self.modulename))
		return false
	end

	local funtList = {IClientNetFunc_OnRecv, IClientNetFunc_OnConnect, IClientNetFunc_OnDisConnect, IClientNetFunc_OnConnectFailed, IClientNetFunc_OnPing, IClientNetFunc_OnSendPacketAttach}
	for _, funtname in pairs(funtList) do
		if not self.modulename[funtname] then
			print(string.format("IClientPlayerClass modulename=%s not has key=%s", self.modulename, funtname))
			return false
		end
	end

	local socket = CoreNet.TCPClient(self.cIP, self.iPort, self.iTimeOut, self.iConnectTimeOut, not self.bClinetFormat, self.iDomain, self.bNoDelay)
	if socket == 0 then 
		print(string.format("IClientPlayerClass modulename=%s Client Connect Failed. cIP=%s iPort=%s", self.modulename, self.cIP, self.Port))
		return false 
	end

	self.hsocket = socket
	net_module.IClientList[self.hsocket] = self
	return true 
end

function IClientPlayerClass:TryReConnect()
	if self.hsocket ~= 0 then return false end
	local timeCnt = CoreTool.GetTickCount()
	if self.m_ullReConnectTime > timeCnt then return end
	self.m_ullReConnectTime = timeCnt
	return self:Connect()
end

function IClientPlayerClass:SendData(proto, data)
	local header = string.pack(">H", proto)
	return CoreNet.TCPSend(self.hsocket, header, data)
end

function IClientPlayerClass:DisConnect()
	CoreNet.TCPClose(self.hsocket)
end

function IClientPlayerClass:TimeToPingPing()
	if self.hsocket == 0 then return end
	local timeCnt = CoreTool.GetTickCount()
	if iPingTimeDelay + self.m_ullPingTIme > timeCnt then return end
	self.m_ullPingTIme = timeCnt
	self.modulename[IClientNetFunc_OnPing](self)
end

function IClientPlayerClass:OnConnect(ip)
	self.m_ullPingTIme = CoreTool.GetTickCount()
	self.m_iReConnectNum = 0
	self.m_ullReConnectTime = CoreTool.GetTickCount()
	local isOK, ret = pcall(function () self.modulename[IClientNetFunc_OnSendPacketAttach](self) end)
	if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end
	self.modulename[IClientNetFunc_OnConnect](self, ip)
end

function IClientPlayerClass:OnConnectFailed()
	self.m_iReConnectNum = self.m_iReConnectNum + 1
	if self.m_iReConnectNum > iReConnectCount then self.m_iReConnectNum = 0 end
	self.m_ullReConnectTime = CoreTool.GetTickCount() + self.m_iReConnectNum*iReConnectDelayTime

	local isOK, ret = pcall(function () self.modulename[IClientNetFunc_OnConnectFailed](self) end)
	if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end

	net_module.IClientList[self.hsocket] = nil
	self.hsocket = 0
end

function IClientPlayerClass:OnDisConnect()
	self.m_iReConnectNum = self.m_iReConnectNum + 1
	if self.m_iReConnectNum > iReConnectCount then self.m_iReConnectNum = 0 end
	self.m_ullReConnectTime = CoreTool.GetTickCount() + self.m_iReConnectNum*iReConnectDelayTime;

	local isOK, ret = pcall(function () self.modulename[IClientNetFunc_OnDisConnect](self) end)
	if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end

	net_module.IClientList[self.hsocket] = nil
	self.hsocket = 0
end

function IClientPlayerClass:OnRecv(data)
	local proto, len = string.unpack(">H", data)
	local contents = string.sub(data, len, string.len(data))
	self.modulename[IClientNetFunc_OnRecv](self, proto, contents)
end

return util.ReadOnlyTable(IClientPlayerClass)