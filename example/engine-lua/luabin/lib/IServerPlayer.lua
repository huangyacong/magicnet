local net_module = require "ccorenet"
local CoreNet = require "CoreNet"
local util = require "util"
require "class"

local IServerNetFunc_OnRecv = "OnSPlayerRecv"
local IServerNetFunc_OnConnect = "OnSPlayerConnect"
local IServerNetFunc_OnDisConnect = "OnSPlayerDisConnect"

local IServerPlayerClass = class()

function IServerPlayerClass:ctor(className, modulename, cIP, iPort, iTimeOut, bReusePort, bNoDelay)
	self.hsocket = 0
	self.className = tostring(className)
	self.modulename = tostring(modulename)

	self.cIP = cIP
	self.iPort = iPort
	self.iTimeOut = iTimeOut
	self.bReusePort = bReusePort
	self.bNoDelay = bNoDelay
end

function IServerPlayerClass:GetModule()
	return package.loaded[self.modulename]
end

function IServerPlayerClass:GetName()
	return self.className
end

function IServerPlayerClass:Listen()
	-- 模块modulename中必须是table，同时必须有下面的key

	if type(self:GetModule()) ~= type({}) then
		print(debug.traceback(), "\n", "IServerPlayerClass Listen modulename not a table")
		return false
	end

	if not next(self:GetModule()) then
		print(debug.traceback(), "\n", string.format("IServerPlayerClass modulename is empty"))
		return false
	end

	local funtList = {IServerNetFunc_OnRecv, IServerNetFunc_OnConnect, IServerNetFunc_OnDisConnect}
	for _, funtname in pairs(funtList) do
		if not self:GetModule()[funtname] then
			print(debug.traceback(), "\n", string.format("IServerPlayerClass modulename not has key=%s", funtname))
			return false
		end
	end

	if self.hsocket ~= 0 then 
		print(debug.traceback(), "\n", "IServerPlayerClass is Listen")
		return false 
	end

	local socket = CoreNet.TCPListen(self.cIP, self.iPort, self.iTimeOut, false, net_module.IpV4, self.bReusePort, self.bNoDelay)
	if socket == 0 then 
		print(debug.traceback(), "\n", string.format("IServerPlayerClass modulename Listen Failed. cIP=%s iPort=%s", self.cIP, self.iPort))
		return false 
	end

	self.hsocket = socket
	net_module.IServerList[self.hsocket] = self
	return true 
end

function IServerPlayerClass:BroadcastData(socketArray, proto, data)
	local header = string.pack(">H", proto)
	return CoreNet.TCPBroadcast(socketArray, header, data)
end

function IServerPlayerClass:SendData(socket, proto, data)
	local header = string.pack(">H", proto)
	return CoreNet.TCPSend(socket, header, data)
end

function IServerPlayerClass:DisConnect(socket)
	CoreNet.TCPClose(socket)
end

function IServerPlayerClass:OnConnect(socket, ip)
	self:GetModule()[IServerNetFunc_OnConnect](self, socket, ip)
end

function IServerPlayerClass:OnDisConnect(socket)
	self:GetModule()[IServerNetFunc_OnDisConnect](self, socket)
end

function IServerPlayerClass:OnRecv(socket, data)
	local proto, len = string.unpack(">H", data)
	local contents = string.sub(data, len, string.len(data))
	self:GetModule()[IServerNetFunc_OnRecv](self, socket, proto, contents)
end

return util.ReadOnlyTable(IServerPlayerClass)