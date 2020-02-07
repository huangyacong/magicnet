local ccoroutine = require "ccoroutine"
local net_module = require "ccorenet"
local msgpack = require "msgpack53"
local CoreNet = require "CoreNet"
local util = require "util"
require "class"

local IServerNetFunc_OnRecv = "OnRecv"
local IServerNetFunc_OnConnect = "OnConnect"
local IServerNetFunc_OnDisConnect = "OnDisConnect"

local IServerClass = class()

function IServerClass:ctor(className, modulename, cIP, iPort, iTimeOut, iDomain, bReusePort, bNoDelay)
	self.hsocket = 0
	self.className = tostring(className)
	self.modulename = modulename

	self.cIP = cIP
	self.iPort = iPort
	self.iTimeOut = iTimeOut
	self.iDomain = iDomain
	self.bReusePort = bReusePort
	self.bNoDelay = bNoDelay
end

function IServerClass:GetName()
	return self.className
end

function IServerClass:Listen()
	-- 模块modulename中必须是table，同时必须有下面的key

	if type(self.modulename) ~= type({}) then
		print("IServerClass Listen modulename not a table")
		return false
	end

	if not next(self.modulename) then
		print(string.format("IServerClass modulename=%s is empty", self.modulename))
		return false
	end

	local funtList = {IServerNetFunc_OnRecv, IServerNetFunc_OnConnect, IServerNetFunc_OnDisConnect}
	for _, funtname in pairs(funtList) do
		if not self.modulename[funtname] then
			print(string.format("IServerClass modulename=%s not has key=%s", self.modulename, funtname))
			return false
		end
	end

	local socket = CoreNet.TCPListen(self.cIP, self.iPort, self.iTimeOut, true, self.iDomain, self.bReusePort, self.bNoDelay)
	if socket == 0 then 
		print(string.format("IServerClass modulename=%s Listen Failed. cIP=%s iPort=%s", self.modulename, self.cIP, self.iPort))
		return false 
	end

	self.hsocket = socket
	net_module.IServerList[self.hsocket] = self
	return true 
end

function IServerClass:SendData(socket, proto, data)
	local header, contents, PTYPE, session_id = net_module.pack(proto, data, net_module.PTYPE.PTYPE_COMMON, 0)
	return CoreNet.TCPSend(socket, header, contents)
end

function IServerClass:CallData(socket, proto, data, timeout_millsec)
	local header, contents, PTYPE, session_id = net_module.pack(proto, msgpack.pack(data), net_module.PTYPE.PTYPE_CALL, CoreNet.SysSessionId())
	local ret = CoreNet.TCPSend(socket, header, contents)
	if not ret then
		print(debug.traceback(), "\n", "CallData failed")
		return false, "send failed"
	end
	local succ, msg = ccoroutine.yield_call(net_module, session_id, timeout_millsec)
	return succ, (succ == true) and msgpack.unpack(msg) or msg
end

function IServerClass:RetCallData(socket, data)
	local header, contents, PTYPE, session_id = net_module.pack("", msgpack.pack(data), net_module.PTYPE.PTYPE_RESPONSE, ccoroutine.get_session_coroutine_id())
	return CoreNet.TCPSend(socket, header, contents)
end

function IServerClass:DisConnect(socket)
	CoreNet.TCPClose(socket)
end

function IServerClass:OnConnect(socket, ip)
	self.modulename[IServerNetFunc_OnConnect](self, socket, ip)
end

function IServerClass:OnDisConnect(socket)
	self.modulename[IServerNetFunc_OnDisConnect](self, socket)
end

function IServerClass:OnRecv(socket, data)
	local proto, contents, PTYPE, session_id = net_module.unpack(data)
	ccoroutine.add_session_coroutine_id(session_id)
	if net_module.PTYPE.PTYPE_RESPONSE == PTYPE and PTYPE then
		local co = ccoroutine.get_session_id_coroutine(session_id)
		if not co then 
			print(debug.traceback(), "\n", "not find co PTYPE_RESPONSE", session_id)
			return
		end
		ccoroutine.resume(co, true, contents)
		return
	end
	self.modulename[IServerNetFunc_OnRecv](self, socket, proto, (net_module.PTYPE.PTYPE_CALL == PTYPE) and msgpack.unpack(contents) or contents, PTYPE, session_id)
end

return util.ReadOnlyTable(IServerClass)