local ccoroutine = require "ccoroutine"
local net_module = require "ccorenet"
local msgpack = require "msgpack53"
local CoreTool = require "CoreTool"
local CoreNet = require "CoreNet"
local util = require "util"
require "class"

local IClientNetFunc_OnRecv_Call = "OnRecvCall"
local IClientNetFunc_OnRecv_Common = "OnRecvCommon"
local IClientNetFunc_OnConnect = "OnConnect"
local IClientNetFunc_OnDisConnect = "OnDisConnect"
local IClientNetFunc_OnConnectFailed = "OnConnectFailed"

-- 重连间隔时间,没次无法连接，就加上这个时间，几次之后，立马重连
local iReConnectDelayTime = 1000
-- 重连循环次数
local iReConnectCount = 10
-- ping的时间间隔
local iPingTimeDelay  = 1000 * 2
-- ping的函数名
local pingFuncName = "pingFunc"

local IClientClass = class()

function IClientClass:ctor(className, modulename, cIP, iPort, iTimeOut, iConnectTimeOut, iDomain, bNoDelay)
	self.hsocket = 0
	self.className = tostring(className)
	self.modulename = modulename

	self.cIP = cIP
	self.iPort = iPort
	self.iTimeOut = iTimeOut
	self.iConnectTimeOut = iConnectTimeOut
	self.iDomain = iDomain
	self.bNoDelay = bNoDelay

	self.m_iReConnectNum = 0
	self.m_ullReConnectTime = CoreTool.GetTickCount()
	self.m_ullPingTIme = CoreTool.GetTickCount()

	self.pingTimerId = 0
	self.pingFunc = function(obj) obj:TimeToPingPing() end
end

function IClientClass:del()-- 剔除各个变量
	assert(self.hsocket == 0)
	
	self.hsocket = 0
	self.className = 0
	self.modulename = 0

	self.cIP = 0
	self.iPort = 0
	self.iTimeOut = 0
	self.iConnectTimeOut = 0
	self.iDomain = iDomain
	self.bNoDelay = 0

	self.m_iReConnectNum = 0
	self.m_ullReConnectTime = 0
	self.m_ullPingTIme = 0

	self.pingTimerId = 0
	self.pingFunc = nil
end

function IClientClass:ResetSocketData(cIP, iPort, iTimeOut, iConnectTimeOut, bNoDelay)
	self.cIP = cIP
	self.iPort = iPort
	self.iTimeOut = iTimeOut
	self.iConnectTimeOut = iConnectTimeOut
	self.bNoDelay = bNoDelay
end

function IClientClass:GetName()
	return self.className
end

function IClientClass:Connect()
	-- 模块modulename中必须是table，同时必须有下面的key

	if type(self.modulename) ~= type({}) then
		print("IClientClass Listen modulename not a table")
		return false
	end

	if not next(self.modulename) then
		print(string.format("IClientClass modulename=%s is empty", self.modulename))
		return false
	end

	if not self[pingFuncName] then
		print(string.format("IClientClass not has ping func=%s", pingFuncName))
		return false
	end

	local funtList = {IClientNetFunc_OnRecv_Call, IClientNetFunc_OnRecv_Common, IClientNetFunc_OnConnect, IClientNetFunc_OnDisConnect, IClientNetFunc_OnConnectFailed}
	for _, funtname in pairs(funtList) do
		if not self.modulename[funtname] then
			print(string.format("IClientClass modulename=%s not has key=%s", self.modulename, funtname))
			return false
		end
	end

	local socket = CoreNet.TCPClient(self.cIP, self.iPort, self.iTimeOut, self.iConnectTimeOut, true, self.iDomain, self.bNoDelay)
	if socket == 0 then 
		print(string.format("IClientClass modulename=%s Client Connect Failed. cIP=%s iPort=%s", self.modulename, self.cIP, self.Port))
		return false 
	end

	self.hsocket = socket
	net_module.IClientList[self.hsocket] = self
	return true 
end

function IClientClass:TryReConnect()
	if self.hsocket ~= 0 then return false end
	local timeCnt = CoreTool.GetTickCount()
	if self.m_ullReConnectTime > timeCnt then return end
	self.m_ullReConnectTime = timeCnt
	return self:Connect()
end

function IClientClass:SendData(targetName, proto, data)
	local header, contents = net_module.pack(targetName, proto, msgpack.pack(data), net_module.PTYPE.PTYPE_COMMON, 0)
	return CoreNet.TCPSend(self.hsocket, header, contents)
end

function IClientClass:CallData(targetName, proto, data, timeout_millsec)
	local header, contents, PTYPE, session_id = net_module.pack(targetName, proto, msgpack.pack(data), net_module.PTYPE.PTYPE_CALL, CoreNet.SysSessionId())
	local ret = CoreNet.TCPSend(self.hsocket, header, contents)
	if not ret then
		print(debug.traceback(), "\n", "CallData failed")
		return false, "send failed"
	end
	local succ, msg = ccoroutine.yield_call(net_module, session_id, timeout_millsec)
	return succ, (succ == true) and msgpack.unpack(msg) or msg
end

function IClientClass:RetCallData(data)
	local header, contents = net_module.pack("", "", msgpack.pack(data), net_module.PTYPE.PTYPE_RESPONSE, ccoroutine.get_session_coroutine_id())
	return CoreNet.TCPSend(self.hsocket, header, contents)
end

function IClientClass:DisConnect()
	CoreNet.TCPClose(self.hsocket)
end

function IClientClass:TimeToPingPing()
	self.pingTimerId = net_module.addtimer(self, pingFuncName, iPingTimeDelay, self)
	if self.hsocket == 0 then return end
	local timeCnt = CoreTool.GetTickCount()
	if iPingTimeDelay + self.m_ullPingTIme > timeCnt then return end
	self.m_ullPingTIme = timeCnt
	local header, contents = net_module.pack("", "", "", net_module.PTYPE.PTYPE_PING, 0)
	return CoreNet.TCPSend(self.hsocket, header, contents)
end

function IClientClass:OnConnect(ip)
	self.m_ullPingTIme = CoreTool.GetTickCount()
	self.m_iReConnectNum = 0
	self.m_ullReConnectTime = CoreTool.GetTickCount()
	self.pingTimerId = net_module.addtimer(self, pingFuncName, iPingTimeDelay, self)
	local header, contents = net_module.pack("", "", msgpack.pack(self:GetName()), net_module.PTYPE.PTYPE_REGISTER, 0)
	CoreNet.TCPSend(self.hsocket, header, contents)
	self.modulename[IClientNetFunc_OnConnect](self, ip)
end

function IClientClass:OnConnectFailed()
	self.m_iReConnectNum = self.m_iReConnectNum + 1
	if self.m_iReConnectNum > iReConnectCount then self.m_iReConnectNum = 0 end
	self.m_ullReConnectTime = CoreTool.GetTickCount() + self.m_iReConnectNum*iReConnectDelayTime
	net_module.deltimer(self.pingTimerId)
	self.pingTimerId = 0

	local isOK, ret = pcall(function () self.modulename[IClientNetFunc_OnConnectFailed](self) end)
	if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end

	net_module.IClientList[self.hsocket] = nil
	self.hsocket = 0
end

function IClientClass:OnDisConnect()
	self.m_iReConnectNum = self.m_iReConnectNum + 1
	if self.m_iReConnectNum > iReConnectCount then self.m_iReConnectNum = 0 end
	self.m_ullReConnectTime = CoreTool.GetTickCount() + self.m_iReConnectNum*iReConnectDelayTime
	net_module.deltimer(self.pingTimerId)
	self.pingTimerId = 0

	local isOK, ret = pcall(function () self.modulename[IClientNetFunc_OnDisConnect](self) end)
	if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end

	net_module.IClientList[self.hsocket] = nil
	self.hsocket = 0
end

function IClientClass:OnRecv(data)
	local targetName, proto, contents, PTYPE, session_id = net_module.unpack(data)
	ccoroutine.add_session_coroutine_id(session_id)

	if net_module.PTYPE.PTYPE_RESPONSE == PTYPE then
		local co = ccoroutine.get_session_id_coroutine(session_id)
		if not co then 
			print(debug.traceback(), "\n", "not find co PTYPE_RESPONSE", session_id)
			return
		end
		ccoroutine.resume(co, true, contents)
	elseif net_module.PTYPE.PTYPE_CALL == PTYPE then
		self.modulename[IClientNetFunc_OnRecv_Call](self, targetName, proto, msgpack.unpack(contents))
	elseif net_module.PTYPE.PTYPE_COMMON == PTYPE then
		self.modulename[IClientNetFunc_OnRecv_Common](self, targetName, proto, msgpack.unpack(contents))
	elseif net_module.PTYPE.PTYPE_REGISTER == PTYPE then
		
	elseif net_module.PTYPE.PTYPE_PING == PTYPE then
		
	end
end

return util.ReadOnlyTable(IClientClass)