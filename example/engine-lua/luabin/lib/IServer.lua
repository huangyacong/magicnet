local ccoroutine = require "ccoroutine"
local msgpack = require "msgpack53"
local CoreNet = require "CoreNet"
local util = require "util"
require "class"

local IServerNetFunc_OnRecv = "OnRecv"
local IServerNetFunc_OnConnect = "OnConnect"
local IServerNetFunc_OnDisConnect = "OnDisConnect"

local IServerClass = class()

function IServerClass:ctor(className, net_modulename, modulename, cIP, iPort, iTimeOut, bClinetFormat, iDomain, bNoDelay)
	self.hsocket = 0
	self.className = tostring(className)
	self.modulename = modulename
	self.net_modulename = net_modulename

	self.cIP = cIP
	self.iPort = iPort
	self.iTimeOut = iTimeOut
	self.bClinetFormat = bClinetFormat
	self.iDomain = iDomain
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

	local socket = CoreNet.TCPListen(self.cIP, self.iPort, self.iTimeOut, not self.bClinetFormat, self.iDomain, self.bNoDelay)
	if socket == 0 then 
		print(string.format("IServerClass modulename=%s Listen Failed. cIP=%s iPort=%s", self.modulename, self.cIP, self.iPort))
		return false 
	end

	self.hsocket = socket
	self.net_modulename.IServerList[self.hsocket] = self
	return true 
end

function IServerClass:SendData(socket, proto, data)
	local header, contents, PTYPE, session_id = self.net_modulename.pack(self.bClinetFormat, proto, data, self.net_modulename.PTYPE.PTYPE_COMMON, 0)
	return CoreNet.TCPSend(socket, header, contents)
end

function IServerClass:CallData(socket, proto, data)
	local header, contents, PTYPE, session_id = self.net_modulename.pack(self.bClinetFormat, proto, msgpack.pack(data), self.net_modulename.PTYPE.PTYPE_CALL, CoreNet.SysSessionId())
	local ret = CoreNet.TCPSend(socket, header, contents)
	if not ret then
		print(debug.traceback(), "\n", "CallData failed")
		return false, "send failed"
	end
	return ccoroutine.yield_call(self.net_modulename, session_id)
end

function IServerClass:RetCallData(socket, data)
	local header, contents, PTYPE, session_id = self.net_modulename.pack(self.bClinetFormat, "", msgpack.pack(data), self.net_modulename.PTYPE.PTYPE_RESPONSE, ccoroutine.get_session_coroutine_id())
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
	local proto, contents, PTYPE, session_id = self.net_modulename.unpack(self.bClinetFormat, data)
	ccoroutine.add_session_coroutine_id(session_id)
	if self.net_modulename.PTYPE.PTYPE_RESPONSE == PTYPE and PTYPE then
		local co = ccoroutine.get_session_id_coroutine(session_id)
		if not co then 
			print(debug.traceback(), "\n", "not find co PTYPE_RESPONSE", session_id)
			return
		end
		ccoroutine.resume(co, true, contents)
		return
	end
	self.modulename[IServerNetFunc_OnRecv](self, socket, proto, (self.net_modulename.PTYPE.PTYPE_CALL == PTYPE) and msgpack.unpack(contents) or contents)
end

return util.ReadOnlyTable(IServerClass)