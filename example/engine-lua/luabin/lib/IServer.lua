﻿local ccoroutine = require "ccoroutine"
local randomutil = require "randomutil"
local net_module = require "ccorenet"
local msgpack = require "msgpack53"
local CoreTool = require "CoreTool"
local CoreNet = require "CoreNet"
local util = require "util"
require "class"

local IServerNetFunc_OnRecv_Call = "OnSRecvCall"
local IServerNetFunc_OnRecv_Common = "OnSRecvCommon"
local IServerNetFunc_OnRecv_Remote = "OnSRecvRemote"
local IServerNetFunc_OnConnect = "OnSConnect"
local IServerNetFunc_OnDisConnect = "OnSDisConnect"
local IServerNetFunc_OnRegister = "OnSRegister"
local IServerNetFunc_OnSystem = "OnSSystem"

local clientSocket = class()

function clientSocket:ctor(ip, toeknkey, resgisterName)
	self.ip = ip .. ""
	self.toeknkey = toeknkey
	self.resgisterName = resgisterName
end

function clientSocket:get_ip()
	return self.ip
end

function clientSocket:get_key()
	return self.toeknkey
end

function clientSocket:get_name()
	return self.resgisterName
end

function clientSocket:set_name(resgisterName)
	self.resgisterName = resgisterName
end

local IServerClass = class()

function IServerClass:ctor(className, modulename, cIP, iPort, iTimeOut, iDomain, bReusePort, bNoDelay)
	self.hsocket = 0
	self.client_hsocket = {}
	self.className = tostring(className)
	self.modulename = tostring(modulename)

	self.cIP = cIP
	self.iPort = iPort
	self.iTimeOut = iTimeOut
	self.iDomain = iDomain
	self.bReusePort = bReusePort
	self.bNoDelay = bNoDelay
end

function IServerClass:GetModule()
	return package.loaded[self.modulename]
end

function IServerClass:GetName()
	return self.className
end

function IServerClass:Listen()
	-- 模块modulename中必须是table，同时必须有下面的key

	if type(self:GetModule()) ~= type({}) then
		print(debug.traceback(), "\n", "IServerClass Listen modulename not a table")
		return false
	end

	local bEmpty = true
	local funtList = {IServerNetFunc_OnRecv_Call, IServerNetFunc_OnRecv_Common, IServerNetFunc_OnRecv_Remote, IServerNetFunc_OnConnect, IServerNetFunc_OnDisConnect, IServerNetFunc_OnRegister, IServerNetFunc_OnSystem}
	for _, funtname in pairs(funtList) do
		if not self:GetModule()[funtname] then
			print(debug.traceback(), "\n", string.format("IServerClass modulename not has key=%s", funtname))
			return false
		end
		bEmpty = false
	end

	if bEmpty then
		print(debug.traceback(), "\n", string.format("IServerClass modulename is empty"))
		return false
	end

	if self.hsocket ~= 0 then 
		print(debug.traceback(), "\n", "IServerClass is Listen")
		return false 
	end

	local socket = CoreNet.TCPListen(self.cIP, self.iPort, self.iTimeOut, true, self.iDomain, self.bReusePort, self.bNoDelay)
	if socket == 0 then 
		print(debug.traceback(), "\n", string.format("IServerClass modulename Listen Failed. cIP=%s iPort=%s", self.cIP, self.iPort))
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

function IServerClass:SendSystemData(socket, proto, data)
	local header, contents = net_module.pack("", proto, msgpack.pack(data), net_module.PTYPE.PTYPE_SYSTEM, 0)
	return CoreNet.TCPSend(socket, header, contents)
end

function IServerClass:CallData(socket, targetName, proto, data, timeout_millsec)
	local header, contents, PTYPE, session_id = net_module.pack(targetName, proto, msgpack.pack(data), net_module.PTYPE.PTYPE_CALL, CoreTool.SysSessionId())
	local ret = CoreNet.TCPSend(socket, header, contents)
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

function IServerClass:RetCallData(socket, data)
	local header, contents = net_module.pack("", "", msgpack.pack(data), net_module.PTYPE.PTYPE_RESPONSE, ccoroutine.get_session_coroutine_id())
	return CoreNet.TCPSend(socket, header, contents)
end

function IServerClass:DisConnect(socket)
	CoreNet.TCPClose(socket)
end

function IServerClass:GetSocketRegName(socket)
	local clientSocketObj = self.client_hsocket[socket]
	if clientSocketObj then return clientSocketObj:get_name() end
	return  nil
end

function IServerClass:GetSocketIP(socket)
	local clientSocketObj = self.client_hsocket[socket]
	if clientSocketObj then return clientSocketObj:get_ip() end
	return nil
end

function IServerClass:OnConnect(socket, ip)
	local clientSocketObj = clientSocket.new(ip, tostring(CoreTool.GetTickCount()) .. tostring(socket) .. tostring(randomutil.random_int(1, 0x7FFFFFFF)), nil)
	self.client_hsocket[socket] = clientSocketObj
	local header, sendData = net_module.pack("", "", msgpack.pack(table.pack(clientSocketObj:get_key())), net_module.PTYPE.PTYPE_REGISTER_KEY, 0)
	CoreNet.TCPSend(socket, header, sendData)
	self:GetModule()[IServerNetFunc_OnConnect](self, socket, ip)
end

function IServerClass:OnDisConnect(socket)
	local isOK, ret = pcall(function () self:GetModule()[IServerNetFunc_OnDisConnect](self, socket) end)
	if not isOK then pcall(function () print(debug.traceback(), "\n", ret) end) end
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

	if net_module.PTYPE.PTYPE_REGISTER ~= PTYPE and not clientSocketObj:get_name() then
		print(debug.traceback(), "\n", string.format("IServerClass:OnRecv clientSocketObj=%s please register", socket))
		return
	end

	if net_module.PTYPE.PTYPE_REGISTER == PTYPE and clientSocketObj:get_name() then
		print(debug.traceback(), "\n", string.format("IServerClass:OnRecv clientSocketObj=%s register more", socket))
		return
	end

	if net_module.PTYPE.PTYPE_RESPONSE == PTYPE then
		local co = ccoroutine.get_session_id_coroutine(session_id)
		if not co then 
			print(debug.traceback(), "\n", "not find co PTYPE_RESPONSE", session_id)
			return
		end
		local ret, err = ccoroutine.resume(co, true, contents)
		if not ret then print(debug.traceback(), "\n", string.format("IServerClass:OnRecv %s", err)) end
	elseif net_module.PTYPE.PTYPE_CALL == PTYPE then
		self:GetModule()[IServerNetFunc_OnRecv_Call](self, socket, targetName, proto, msgpack.unpack(contents))
	elseif net_module.PTYPE.PTYPE_REMOTE == PTYPE then
		local remote_socket, sendData = table.unpack(msgpack.unpack(contents))
		self:GetModule()[IServerNetFunc_OnRecv_Remote](self, socket, remote_socket, tonumber(proto), sendData)
	elseif net_module.PTYPE.PTYPE_COMMON == PTYPE then
		self:GetModule()[IServerNetFunc_OnRecv_Common](self, socket, targetName, proto, msgpack.unpack(contents))
	elseif net_module.PTYPE.PTYPE_SYSTEM == PTYPE then
		self:GetModule()[IServerNetFunc_OnSystem](self, socket, proto, msgpack.unpack(contents))
	elseif net_module.PTYPE.PTYPE_REGISTER == PTYPE then
		local name, md5str = table.unpack(msgpack.unpack(contents))
		if net_module.genToken(clientSocketObj:get_key(), name) == md5str and string.len(name) > 0 then
			clientSocketObj:set_name(name)
			self:GetModule()[IServerNetFunc_OnRegister](self, socket, name)
		else
			self:DisConnect(socket)
			print(debug.traceback(), "\n", string.format("IServerClass:OnRecv clientSocketObj=%s register failed. name=%s md5str=%s", socket, name, md5str))
		end
	elseif net_module.PTYPE.PTYPE_PING == PTYPE then
		local header, sendData = net_module.pack("", "", "", net_module.PTYPE.PTYPE_PING, 0)
		CoreNet.TCPSend(socket, header, sendData)
	end
end

return util.ReadOnlyTable(IServerClass)