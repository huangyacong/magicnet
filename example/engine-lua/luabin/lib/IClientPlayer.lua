﻿local net_module = require "ccorenet"
local CoreTool = require "CoreTool"
local CoreNet = require "CoreNet"
local timer = require "timer"
local util = require "util"
require "class"

local IClientNetFunc_OnPing = "OnCPlayerPing"
local IClientNetFunc_OnRecv = "OnCPlayerRecv"
local IClientNetFunc_OnConnect = "OnCPlayerConnect"
local IClientNetFunc_OnDisConnect = "OnCPlayerDisConnect"
local IClientNetFunc_OnConnectFailed = "OnCPlayerConnectFailed"
local IClientNetFunc_OnSendPacketAttach = "OnCPlayerSendPacketAttach"

local local_modulename = ...
timer.register(local_modulename)

-- 重连间隔时间,没次无法连接，就加上这个时间，几次之后，立马重连
local iReConnectDelayTime = 1000
-- 重连循环次数
local iReConnectCount = 10
-- ping的时间间隔
local iPingTimeDelay  = 1000 * 2

local IClientPlayer = {}

function IClientPlayer.pingFunc_callback(IClientPlayerClassObj)
	IClientPlayerClassObj:AddPingTimer()
	IClientPlayerClassObj:TimeToPingPing()
end
function IClientPlayer.reconnectFunc_callback(IClientPlayerClassObj)
	IClientPlayerClassObj:AddReConnectTimer()
	IClientPlayerClassObj:TryReConnect()
end

local IClientPlayerClass = class()

function IClientPlayerClass:ctor(className, modulename, cIP, iPort, iTimeOut, iConnectTimeOut, bReConnect, bNoDelay)
	self.hsocket = 0
	self.className = tostring(className)
	self.modulename = tostring(modulename)

	self.cIP = cIP
	self.iPort = iPort
	self.iTimeOut = iTimeOut
	self.iConnectTimeOut = iConnectTimeOut
	self.bNoDelay = bNoDelay

	self.m_iReConnectNum = 0
	self.m_ullReConnectTime = CoreTool.GetTickCount()
	self.m_ullPingTIme = CoreTool.GetTickCount()

	self.pingTimerId = 0
	self.reconnectTimerId = 0
	self.bReConnect = bReConnect
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
	self.bNoDelay = 0

	self.m_iReConnectNum = 0
	self.m_ullReConnectTime = 0
	self.m_ullPingTIme = 0

	self.pingTimerId = 0
	self.reconnectTimerId = 0
	self.bReConnect = false
end

function IClientPlayerClass:ResetSocketData(cIP, iPort, iTimeOut, iConnectTimeOut, bNoDelay)
	self.cIP = cIP
	self.iPort = iPort
	self.iTimeOut = iTimeOut
	self.iConnectTimeOut = iConnectTimeOut
	self.bNoDelay = bNoDelay
end

function IClientPlayerClass:GetModule()
	return package.loaded[self.modulename]
end

function IClientPlayerClass:GetName()
	return self.className
end

function IClientPlayerClass:Connect()
	-- 模块modulename中必须是table，同时必须有下面的key

	if type(self:GetModule()) ~= type({}) then
		print(debug.traceback(), "\n", "IClientPlayerClass modulename not a table")
		return false
	end

	if not next(self:GetModule()) then
		print(debug.traceback(), "\n", string.format("IClientPlayerClass modulename is empty"))
		return false
	end

	local funtList = {IClientNetFunc_OnRecv, IClientNetFunc_OnConnect, IClientNetFunc_OnDisConnect, IClientNetFunc_OnConnectFailed, IClientNetFunc_OnPing, IClientNetFunc_OnSendPacketAttach}
	for _, funtname in pairs(funtList) do
		if not self:GetModule()[funtname] then
			print(debug.traceback(), "\n", string.format("IClientPlayerClass modulename not has key=%s", funtname))
			return false
		end
	end

	if self.hsocket ~= 0 then 
		print(debug.traceback(), "\n", "IClientPlayerClass is connect")
		return false 
	end

	local socket = CoreNet.TCPClient(self.cIP, self.iPort, self.iTimeOut, self.iConnectTimeOut, false, net_module.IpV4, self.bNoDelay)
	if socket == 0 then 
		self:AddReConnectTimer()
		print(debug.traceback(), "\n", string.format("IClientPlayerClass modulename Client Connect Failed. cIP=%s iPort=%s", self.cIP, self.iPort))
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
	self:GetModule()[IClientNetFunc_OnPing](self)
end

function IClientPlayerClass:AddPingTimer()
	self.pingTimerId = timer.addtimer(local_modulename, "pingFunc_callback", iPingTimeDelay, self)
end

function IClientPlayerClass:DelPingTimer()
	timer.deltimer(self.pingTimerId)
	self.pingTimerId = 0
end

function IClientPlayerClass:AddReConnectTimer()
	if not self.bReConnect then 
		return
	end
	self.reconnectTimerId = timer.addtimer(local_modulename, "reconnectFunc_callback", self.m_ullReConnectTime - CoreTool.GetTickCount(), self)
end

function IClientPlayerClass:DelReConnectTimer()
	timer.deltimer(self.reconnectTimerId)
	self.reconnectTimerId = 0
end

function IClientPlayerClass:OnConnect(ip)
	self.m_ullPingTIme = CoreTool.GetTickCount()
	self.m_iReConnectNum = 0
	self.m_ullReConnectTime = CoreTool.GetTickCount()
	self:AddPingTimer()
	self:DelReConnectTimer()
	local isOK, ret = pcall(function () self:GetModule()[IClientNetFunc_OnSendPacketAttach](self) end)
	if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end
	self:GetModule()[IClientNetFunc_OnConnect](self, ip)
end

function IClientPlayerClass:OnConnectFailed()
	self.m_iReConnectNum = self.m_iReConnectNum + 1
	if self.m_iReConnectNum > iReConnectCount then self.m_iReConnectNum = 0 end
	self.m_ullReConnectTime = CoreTool.GetTickCount() + self.m_iReConnectNum*iReConnectDelayTime
	self:AddReConnectTimer()
	self:DelPingTimer()

	local isOK, ret = pcall(function () self:GetModule()[IClientNetFunc_OnConnectFailed](self) end)
	if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end

	net_module.IClientList[self.hsocket] = nil
	self.hsocket = 0
end

function IClientPlayerClass:OnDisConnect()
	self.m_iReConnectNum = self.m_iReConnectNum + 1
	if self.m_iReConnectNum > iReConnectCount then self.m_iReConnectNum = 0 end
	self.m_ullReConnectTime = CoreTool.GetTickCount() + self.m_iReConnectNum*iReConnectDelayTime
	self:AddReConnectTimer()
	self:DelPingTimer()

	local isOK, ret = pcall(function () self:GetModule()[IClientNetFunc_OnDisConnect](self) end)
	if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end

	net_module.IClientList[self.hsocket] = nil
	self.hsocket = 0
end

function IClientPlayerClass:OnRecv(data)
	local proto, len = string.unpack(">H", data)
	local contents = string.sub(data, len, string.len(data))
	self:GetModule()[IClientNetFunc_OnRecv](self, proto, contents)
end

IClientPlayer.IClientPlayerClass = IClientPlayerClass

return util.ReadOnlyTable(IClientPlayer)