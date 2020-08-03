local CoreNetAgent = require "CoreNetAgent"
local ccoroutine = require "ccoroutine"
local net_module = require "ccorenet"
local msgpack = require "msgpack53"
local CoreTool = require "CoreTool"
local CoreNet = require "CoreNet"
local timer = require "timer"
local util = require "util"
require "class"

local IClientNetFunc_OnRecv_Call = "OnCRecvCall"
local IClientNetFunc_OnRecv_Common = "OnCRecvCommon"
local IClientNetFunc_OnConnect = "OnCConnect"
local IClientNetFunc_OnDisConnect = "OnCDisConnect"
local IClientNetFunc_OnConnectFailed = "OnCConnectFailed"
local IClientNetFunc_Register = "OnCRegister"
local IClientNetFunc_OnRemoteConnect = "OnCRemoteConnect"
local IClientNetFunc_OnRemoteDisConnect = "OnCRemoteDisConnect"
local IClientNetFunc_OnRemoteRecvData = "OnCRemoteRecvData"

local local_modulename = ...
timer.register(local_modulename)

-- 重连间隔时间,没次无法连接，就加上这个时间，几次之后，立马重连
local iReConnectDelayTime = 1000
-- 重连循环次数
local iReConnectCount = 10
-- 缓存计数器
local IClientClassPoolCount = 1
-- 缓存
local IClientClassPool = setmetatable({}, { __mode = "kv" })

local IClient = {}

function IClient.GetPoolCount()
	local count = 0
	for _, value in pairs(IClientClassPool) do
		count = count + 1
	end
	return count
end

-- 定时器回调
function IClient.pingFunc_callback(IClientClassObj)
	IClientClassObj:AddPingTimer()
	IClientClassObj:TimeToPingPing()
end
function IClient.reconnectFunc_callback(IClientClassObj)
	IClientClassObj:TryReConnect()
end

local IClientClass = class()

function IClientClass:ctor(className, modulename, cIP, iPort, iTimeOut, iConnectTimeOut, iDomain, bReConnect, bNoDelay)
	self.hsocket = 0
	self.className = tostring(className)
	self.modulename = tostring(modulename)

	self.cIP = cIP
	self.iPort = iPort
	self.iTimeOut = iTimeOut
	self.iConnectTimeOut = iConnectTimeOut
	self.iDomain = iDomain
	self.bNoDelay = bNoDelay

	self.pingTimerId = 0
	self.iPingTimeDelay  = 1000 * 2

	self.m_iReConnectNum = 0
	self.reconnectTimerId = 0
	self.bReConnect = bReConnect

	self.regServiceList = {}

	self.privateData = nil
	IClientClassPoolCount = IClientClassPoolCount + 1
	IClientClassPool[IClientClassPoolCount] = self
end

function IClientClass:ResetSocketData(cIP, iPort, iTimeOut, iConnectTimeOut, bNoDelay)
	self.cIP = cIP
	self.iPort = iPort
	self.iTimeOut = iTimeOut
	self.iConnectTimeOut = iConnectTimeOut
	self.bNoDelay = bNoDelay
end

function IClientClass:IsServiceReg(name)
	return self.regServiceList[name] ~= nil
end

function IClientClass:GetRegServiceList()
	return self.regServiceList
end

function IClientClass:SetPingTimeDelay(iPingTimeDelay)
	self.iPingTimeDelay = iPingTimeDelay
end

function IClientClass:SetPrivateData(data)
	self.privateData = data
end

function IClientClass:GetPrivateData()
	return self.privateData
end

function IClientClass:GetModule()
	return package.loaded[self.modulename]
end

function IClientClass:GetName()
	return self.className
end

function IClientClass:HSocket()
	return self.hsocket
end

function IClientClass:Connect()
	-- 模块modulename中必须是table，同时必须有下面的key

	if type(self:GetModule()) ~= type({}) then
		print(debug.traceback(), "\n", "IClientClass modulename not a table")
		return false
	end

	local bEmpty = true
	local funtList = {IClientNetFunc_OnRecv_Call, IClientNetFunc_OnRecv_Common, IClientNetFunc_OnConnect, IClientNetFunc_OnDisConnect, IClientNetFunc_OnConnectFailed, IClientNetFunc_Register, IClientNetFunc_OnRemoteConnect, IClientNetFunc_OnRemoteDisConnect, IClientNetFunc_OnRemoteRecvData}
	for _, funtname in pairs(funtList) do
		if not self:GetModule()[funtname] then
			print(debug.traceback(), "\n", string.format("IClientClass modulename not has key=%s", funtname))
			return false
		end
		bEmpty = false
	end

	if bEmpty then
		print(debug.traceback(), "\n", string.format("IClientClass modulename is empty"))
		return false
	end

	if self.hsocket ~= 0 then 
		print(debug.traceback(), "\n", "IClientClass is connect")
		return false 
	end

	local socket = CoreNet.TCPClient(self.cIP, self.iPort, self.iTimeOut, self.iConnectTimeOut, true, self.iDomain, self.bNoDelay)
	if socket == 0 then 
		self:AddReConnectTimer()
		print(debug.traceback(), "\n", string.format("IClientClass modulename Client Connect Failed. cIP=%s iPort=%s", self.cIP, self.iPort))
		return false 
	end

	self.hsocket = socket
	net_module.IClientList[self.hsocket] = self
	return true 
end

function IClientClass:TryReConnect()
	if self.hsocket ~= 0 then return false end
	return self:Connect()
end

function IClientClass:ExitAgent()
	local header, contents = net_module.netPack(self:GetName(), "", "", CoreNetAgent.PTYPE_EXIT, 0, "")
	return CoreNet.TCPSend(self.hsocket, header, contents)
end

function IClientClass:SendData(targetName, proto, data)
	local header, contents = net_module.netPack(self:GetName(), targetName, proto, CoreNetAgent.PTYPE_COMMON, 0, msgpack.pack(data))
	return CoreNet.TCPSend(self.hsocket, header, contents)
end

function IClientClass:SendRemoteData(remote_socket, proto, data)
	assert(type(proto) == type(0))
	assert(type(remote_socket) == type(0))
	local header, contents = net_module.netPack(self:GetName(), "", proto, CoreNetAgent.PTYPE_REMOTE, remote_socket, data)
	return CoreNet.TCPSend(self.hsocket, header, contents)
end

function IClientClass:CloseRemote(remote_socket)
	assert(type(remote_socket) == type(0))
	local header, contents = net_module.netPack(self:GetName(), "", "", CoreNetAgent.PTYPE_REMOTE_CLOSE, remote_socket, "")
	return CoreNet.TCPSend(self.hsocket, header, contents)
end

function IClientClass:CallData(targetName, proto, data, timeout_millsec)
	local session_id = CoreTool.SysSessionId()
	local header, contents = net_module.netPack(self:GetName(), targetName, proto, CoreNetAgent.PTYPE_CALL, session_id, msgpack.pack(data))
	local ret = CoreNet.TCPSend(self.hsocket, header, contents)
	if not ret then
		print(debug.traceback(), "\n", "CallData failed")
		return false, "send failed"
	end
	local succ, msg = ccoroutine.yield_call(session_id, timeout_millsec)
	if succ then
		msg = msgpack.unpack(msg)
	end
	return succ, msg
end

function IClientClass:RetCallData(data)
	local sessionId, srcName, proto = ccoroutine.get_session_coroutine_id()
	local header, contents = net_module.netPack(self:GetName(), srcName, proto, CoreNetAgent.PTYPE_RESPONSE, sessionId, msgpack.pack(data))
	return CoreNet.TCPSend(self.hsocket, header, contents)
end

function IClientClass:DisConnect()
	CoreNet.TCPClose(self.hsocket)
end

function IClientClass:TimeToPingPing()
	if self.hsocket == 0 then return end
	local header, contents = net_module.netPack(self:GetName(), "", "", CoreNetAgent.PTYPE_PING, 0, "")
	return CoreNet.TCPSend(self.hsocket, header, contents)
end

function IClientClass:AddPingTimer()
	self.pingTimerId = timer.addtimer(local_modulename, "pingFunc_callback", self.iPingTimeDelay, self)
end

function IClientClass:DelPingTimer()
	timer.deltimer(self.pingTimerId)
	self.pingTimerId = 0
end

function IClientClass:AddReConnectTimer()
	if not self.bReConnect then
		return
	end
	local timeCount = self.m_iReConnectNum * iReConnectDelayTime
	self.reconnectTimerId = timer.addtimer(local_modulename, "reconnectFunc_callback", timeCount, self)
end

function IClientClass:DelReConnectTimer()
	timer.deltimer(self.reconnectTimerId)
	self.reconnectTimerId = 0
end

function IClientClass:OnConnect(ip)
	self.m_iReConnectNum = 0
	self:AddPingTimer()
	self:DelReConnectTimer()
	self:GetModule()[IClientNetFunc_OnConnect](self, ip)
end

function IClientClass:OnConnectFailed()
	self.m_iReConnectNum = self.m_iReConnectNum + 1
	if self.m_iReConnectNum > iReConnectCount then self.m_iReConnectNum = 0 end
	self:AddReConnectTimer()
	self:DelPingTimer()

	local isOK, ret = pcall(function () self:GetModule()[IClientNetFunc_OnConnectFailed](self) end)
	if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end

	net_module.IClientList[self.hsocket] = nil
	self.regServiceList = {}
	self.hsocket = 0
end

function IClientClass:OnDisConnect()
	self.m_iReConnectNum = self.m_iReConnectNum + 1
	if self.m_iReConnectNum > iReConnectCount then self.m_iReConnectNum = 0 end
	self:AddReConnectTimer()
	self:DelPingTimer()

	local isOK, ret = pcall(function () self:GetModule()[IClientNetFunc_OnDisConnect](self) end)
	if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end

	net_module.IClientList[self.hsocket] = nil
	self.regServiceList = {}
	self.hsocket = 0
end

function IClientClass:OnRecv(data)
	local srcName, targetName, PTYPE, session_id, proto, contents = net_module.netUnPack(data)

	ccoroutine.add_session_coroutine_id(session_id, srcName, proto)

	local funcRet, funcErr = pcall(function() 
		if CoreNetAgent.PTYPE_RESPONSE == PTYPE then
			local co = ccoroutine.get_session_id_coroutine(session_id)
			if co then ccoroutine.resume(co, true, contents) end
			if not co then print(debug.traceback(), "\n", "not find co PTYPE_RESPONSE", session_id) end
		elseif CoreNetAgent.PTYPE_REGISTER_KEY == PTYPE then
			local md5str = net_module.genToken(srcName, self:GetName())
			local header, sendData = net_module.netPack(self:GetName(), md5str, "sendregname", CoreNetAgent.PTYPE_REGISTER, 0, "")
			CoreNet.TCPSend(self.hsocket, header, sendData)
			self:GetModule()[IClientNetFunc_Register](self)
			print(string.format("IClientClass:OnRecv Name=%s recv register key=%s md5str=%s", self:GetName(), srcName, md5str))
		elseif CoreNetAgent.PTYPE_REG_ADD_SERVICE == PTYPE then
			self.regServiceList[srcName] = srcName
			print(string.format("IClientClass:OnRecv Name=%s recv add service [%s] ", self:GetName(), srcName))
		elseif CoreNetAgent.PTYPE_REG_DEL_SERVICE == PTYPE then
			self.regServiceList[srcName] = nil
			print(string.format("IClientClass:OnRecv Name=%s recv del service [%s] ", self:GetName(), srcName))
		elseif CoreNetAgent.PTYPE_CALL == PTYPE then
			self:GetModule()[IClientNetFunc_OnRecv_Call](self, proto, msgpack.unpack(contents))
		elseif CoreNetAgent.PTYPE_COMMON == PTYPE then
			self:GetModule()[IClientNetFunc_OnRecv_Common](self, proto, msgpack.unpack(contents))
		elseif CoreNetAgent.PTYPE_REMOTE_CONNECT == PTYPE then
			self:GetModule()[IClientNetFunc_OnRemoteConnect](self, session_id, srcName)
		elseif CoreNetAgent.PTYPE_REMOTE_DISCONNECT == PTYPE then
			self:GetModule()[IClientNetFunc_OnRemoteDisConnect](self, session_id)
		elseif CoreNetAgent.PTYPE_REMOTE_RECV_DATA == PTYPE then
			self:GetModule()[IClientNetFunc_OnRemoteRecvData](self, session_id, proto, contents)
		end
	end)

	if not funcRet then
		print(debug.traceback(), "\n", "IClientClass:OnRecv", funcErr)
	end

	ccoroutine.del_session_coroutine_id()
end

IClient.IClientClass = IClientClass

return util.ReadOnlyTable(IClient)