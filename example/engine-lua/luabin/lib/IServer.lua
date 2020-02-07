local ccoroutine = require "ccoroutine"
local randomutil = require "randomutil"
local net_module = require "ccorenet"
local msgpack = require "msgpack53"
local CoreTool = require "CoreTool"
local CoreNet = require "CoreNet"
local util = require "util"
require "class"

local IServerNetFunc_OnRecv_Call = "OnRecvCall"
local IServerNetFunc_OnRecv_Common = "OnRecvCommon"
local IServerNetFunc_OnRecv_Remote = "OnRecvRemote"
local IServerNetFunc_OnConnect = "OnConnect"
local IServerNetFunc_OnDisConnect = "OnDisConnect"
local IServerNetFunc_OnRegister = "OnRegister"

local IServerClass = class()

function IServerClass:ctor(className, modulename, cIP, iPort, iTimeOut, iDomain, bReusePort, bNoDelay)
	self.hsocket = 0
	self.client_hsocket = {}
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
		print(debug.traceback(), "\n", "IServerClass Listen modulename not a table")
		return false
	end

	if not next(self.modulename) then
		print(debug.traceback(), "\n", string.format("IServerClass modulename=%s is empty", self.modulename))
		return false
	end

	local funtList = {IServerNetFunc_OnRecv_Call, IServerNetFunc_OnRecv_Common, IServerNetFunc_OnRecv_Remote, IServerNetFunc_OnConnect, IServerNetFunc_OnDisConnect, IServerNetFunc_OnRegister}
	for _, funtname in pairs(funtList) do
		if not self.modulename[funtname] then
			print(debug.traceback(), "\n", string.format("IServerClass modulename=%s not has key=%s", self.modulename, funtname))
			return false
		end
	end

	local socket = CoreNet.TCPListen(self.cIP, self.iPort, self.iTimeOut, true, self.iDomain, self.bReusePort, self.bNoDelay)
	if socket == 0 then 
		print(debug.traceback(), "\n", string.format("IServerClass modulename=%s Listen Failed. cIP=%s iPort=%s", self.modulename, self.cIP, self.iPort))
		return false 
	end

	self.hsocket = socket
	net_module.IServerList[self.hsocket] = self
	return true 
end

function IServerClass:SendData(socket, targetName, proto, data)
	local header, contents = net_module.pack(targetName, proto, msgpack.pack(data), net_module.PTYPE.PTYPE_COMMON, 0)
	return CoreNet.TCPSend(socket, header, contents)
end

function IServerClass:CallData(socket, targetName, proto, data, timeout_millsec)
	local header, contents, PTYPE, session_id = net_module.pack(targetName, proto, msgpack.pack(data), net_module.PTYPE.PTYPE_CALL, CoreNet.SysSessionId())
	local ret = CoreNet.TCPSend(socket, header, contents)
	if not ret then
		print(debug.traceback(), "\n", "CallData failed")
		return false, "send failed"
	end
	local succ, msg = ccoroutine.yield_call(net_module, session_id, timeout_millsec)
	return succ, (succ == true) and msgpack.unpack(msg) or msg
end

function IServerClass:RetCallData(socket, data)
	local header, contents = net_module.pack("", "", msgpack.pack(data), net_module.PTYPE.PTYPE_RESPONSE, ccoroutine.get_session_coroutine_id())
	return CoreNet.TCPSend(socket, header, contents)
end

function IServerClass:DisConnect(socket)
	CoreNet.TCPClose(socket)
end

function IServerClass:GetSocketRegName(socket)
	local clientSocketObj = self.client_hsocket[socket]
	return clientSocketObj and clientSocketObj.resgisterName or nil
end

function IServerClass:OnConnect(socket, ip)
	self.client_hsocket[socket] = {key = tostring(CoreTool.GetTickCount()) .. tostring(socket) .. tostring(randomutil.random_int(1, 0x7FFFFFFF)), resgisterName = nil}
	local header, sendData = net_module.pack("", "", msgpack.pack(table.pack(self.client_hsocket[socket].key)), net_module.PTYPE.PTYPE_REGISTER_KEY, 0)
	CoreNet.TCPSend(socket, header, sendData)
	self.modulename[IServerNetFunc_OnConnect](self, socket, ip)
end

function IServerClass:OnDisConnect(socket)
	self.modulename[IServerNetFunc_OnDisConnect](self, socket)
	self.client_hsocket[socket] = nil
end

function IServerClass:OnRecv(socket, data)
	local targetName, proto, contents, PTYPE, session_id = net_module.unpack(data)
	ccoroutine.add_session_coroutine_id(session_id)

	local clientSocketObj = self.client_hsocket[socket]
	if not clientSocketObj then
		print(debug.traceback(), "\n", string.format("IServerClass:OnRecv not find clientSocketObj=%s", socket))
		return
	end

	if net_module.PTYPE.PTYPE_REGISTER ~= PTYPE and not clientSocketObj.resgisterName then
		print(debug.traceback(), "\n", string.format("IServerClass:OnRecv clientSocketObj=%s please register", socket))
		return
	end

	if net_module.PTYPE.PTYPE_REGISTER == PTYPE and clientSocketObj.resgisterName then
		print(debug.traceback(), "\n", string.format("IServerClass:OnRecv clientSocketObj=%s register more", socket))
		return
	end

	if net_module.PTYPE.PTYPE_RESPONSE == PTYPE then
		local co = ccoroutine.get_session_id_coroutine(session_id)
		if not co then 
			print(debug.traceback(), "\n", "not find co PTYPE_RESPONSE", session_id)
			return
		end
		ccoroutine.resume(co, true, contents)
	elseif net_module.PTYPE.PTYPE_CALL == PTYPE then
		self.modulename[IServerNetFunc_OnRecv_Call](self, socket, targetName, proto, msgpack.unpack(contents))
	elseif net_module.PTYPE.PTYPE_REMOTE == PTYPE then
		local remote_socket, sendData = table.unpack(msgpack.unpack(contents))
		self.modulename[IServerNetFunc_OnRecv_Remote](self, socket, remote_socket, tonumber(proto), sendData)
	elseif net_module.PTYPE.PTYPE_COMMON == PTYPE then
		self.modulename[IServerNetFunc_OnRecv_Common](self, socket, targetName, proto, msgpack.unpack(contents))
	elseif net_module.PTYPE.PTYPE_REGISTER == PTYPE then
		local name, md5str = table.unpack(msgpack.unpack(contents))
		if net_module.genToken(clientSocketObj.key, name) == md5str then
			clientSocketObj.resgisterName = name
			self.modulename[IServerNetFunc_OnRegister](self, socket, name)
			print(string.format("IServerClass:OnRecv clientSocketObj=%s register ok. name=%s md5str=%s", socket, name, md5str))
		else
			self:DisConnect(socket)
			print(string.format("IServerClass:OnRecv clientSocketObj=%s register failed. name=%s md5str=%s", socket, name, md5str))
		end
	elseif net_module.PTYPE.PTYPE_PING == PTYPE then
		local header, sendData = net_module.pack("", "", "", net_module.PTYPE.PTYPE_PING, 0)
		CoreNet.TCPSend(socket, header, sendData)
	end
end

return util.ReadOnlyTable(IServerClass)