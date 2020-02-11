local IServerPlayer = require "IServerPlayer"
local net_module = require "ccorenet"
local CoreTool = require "CoreTool"
local IServer = require "IServer"
local util = require "util"
require "class"

local IAgentGateNetFunc_OnLocalRecvCall = "OnLocalRecvCall"
local IAgentGateNetFunc_OnLocalRecvCommon = "OnLocalRecvCommon"
local IAgentGateNetFunc_OnRemoteRecv = "OnRemoteRecv"
local IAgentGateNetFunc_OnRemoteConnect = "OnRemoteConnect"
local IAgentGateNetFunc_OnRemoteDisConnect = "OnRemoteDisConnect"

local AgentGate = {}

local AgentGateEvent = {}
local AgentGateClassName = ""

local AgentGateRemoteSvrObj = nil
local AgentGateLocalSvrIPObj = nil
local AgentGateLocalSvrUnixObj = nil

-- 服务注册列表
local AgentGateRegSvrList = {}

-----------------------------------------------------------------
local ServerRemotEvent = {}

function ServerRemotEvent.OnConnect(IClientObj, socket, ip)
	AgentGateEvent[IAgentGateNetFunc_OnRemoteConnect](socket, ip)
end

function ServerRemotEvent.OnDisConnect(IClientObj, socket)
	AgentGateEvent[IAgentGateNetFunc_OnRemoteDisConnect](socket)
end

function ServerRemotEvent.OnRecv(IClientObj, socket, proto, data)
	AgentGateEvent[IAgentGateNetFunc_OnRemoteRecv](socket, proto, data)
end
-----------------------------------------------------------------


-----------------------------------------------------------------
local ServerLocalEvent = {}

function ServerLocalEvent.OnRecvCall(IServerObj, socket, targetName, proto, data)
	if targetName == AgentGateClassName then
		AgentGateEvent[IAgentGateNetFunc_OnLocalRecvCall](IServerObj, socket, proto, data)
		return
	end
	if not AgentGateRegSvrList[targetName] then
		print(debug.traceback(), "\n", string.format("AgentGate.OnRecvCall not find targetName=%s error", targetName))
		return
	end
	local IServerObj_, socket_ = table.unpack(AgentGateRegSvrList[targetName])
	local ret, sendData = IServerObj_:CallData(socket_, targetName, proto, data)
	if ret then
		IServerObj:RetCallData(socket, sendData)
	end
end

function ServerLocalEvent.OnRecvCommon(IServerObj, socket, targetName, proto, data)
	if targetName == AgentGateClassName then
		AgentGateEvent[IAgentGateNetFunc_OnLocalRecvCommon](IServerObj, socket, proto, data)
		return
	end
	if not AgentGateRegSvrList[targetName] then
		print(debug.traceback(), "\n", string.format("AgentGate.OnRecvCommon not find targetName=%s error", targetName))
		return
	end
	local IServerObj_, socket_ = table.unpack(AgentGateRegSvrList[targetName])
	IServerObj_:SendData(socket_, targetName, proto, data)
end

function ServerLocalEvent.OnRecvRemote(IServerObj, socket, remote_socket, proto, data)
	AgentGateRemoteSvrObj:SendData(remote_socket, proto, data)
end

function ServerLocalEvent.OnConnect(IServerObj, socket, ip)
end

function ServerLocalEvent.OnDisConnect(IServerObj, socket)
	local name = IServerObj:GetSocketRegName(socket)
	if name then
		local _, socket_ = table.unpack(AgentGateRegSvrList[name])
		if socket_ == socket then
			AgentGateRegSvrList[name] = nil
			print(string.format("AgentGate.OnRegister unregister name=%s ok", name))
		end
	end
end

function ServerLocalEvent.OnRegister(IServerObj, socket, name)
	if IServerObj:GetSocketRegName(socket) ~= name then
		IServerObj:DisConnect(socket)
		print(debug.traceback(), "\n", string.format("AgentGate.OnRegister name=%s error", name))
		return
	end
	if AgentGateRegSvrList[name] or AgentGateClassName == name then
		IServerObj:DisConnect(socket)
		print(debug.traceback(), "\n", string.format("AgentGate.OnRegister name=%s AgentGateClassName=%s is same", name, AgentGateClassName))
		return
	end
	AgentGateRegSvrList[name] = table.pack(IServerObj, socket)
	print(string.format("AgentGate.OnRegister register name=%s ok", name))
end
-----------------------------------------------------------------

function AgentGate.SendRemote(remote_socket, proto, data)
	return AgentGateRemoteSvrObj:SendData(remote_socket, proto, data)
end

function AgentGate.SendData(targetName, proto, data)
	if not AgentGateRegSvrList[targetName] then
		print(debug.traceback(), "\n", string.format("AgentGate.SendData not find targetName=%s error", targetName))
		return false
	end
	local IServerObj_, socket_ = table.unpack(AgentGateRegSvrList[targetName])
	return IServerObj_:SendData(socket_, targetName, proto, data)
end

function AgentGate:CallData(targetName, proto, data, timeout_millsec)
	if not AgentGateRegSvrList[targetName] then
		print(debug.traceback(), "\n", string.format("AgentGate.CallData not find targetName=%s error", targetName))
		return false, "send failed"
	end
	local IServerObj_, socket_ = table.unpack(AgentGateRegSvrList[targetName])
	return IServerObj_:CallData(socket_, targetName, proto, data, timeout_millsec)
end

function AgentGate.GetName()
	return AgentGateClassName..""
end

function AgentGate.IsServiceRegister(serviceName)
	return AgentGateRegSvrList[serviceName] and true or false
end

function AgentGate.Init(className, modulename, cRemoteIP, iRemotePort, iRemoteTimeOut, cLocalIP, iLocalPort, cUnixSocketName, iLocalTimeOut, bNoDelay)
	-- 模块modulename中必须是table，同时必须有下面的key

	if type(modulename) ~= type({}) then
		print(debug.traceback(), "\n", "AgentGate.Init modulename not a table")
		return false
	end

	if not next(modulename) then
		print(debug.traceback(), "\n", string.format("AgentGate.Init modulename is empty"))
		return false
	end

	local funtList = {IAgentGateNetFunc_OnLocalRecvCall, IAgentGateNetFunc_OnLocalRecvCommon, IAgentGateNetFunc_OnRemoteRecv, IAgentGateNetFunc_OnRemoteConnect, IAgentGateNetFunc_OnRemoteDisConnect}
	for _, funtname in pairs(funtList) do
		if not modulename[funtname] then
			print(string.format(debug.traceback(), "\n", "AgentGate.Init modulename not has key=%s", funtname))
			return false
		end
	end

	-- 赋值
	AgentGateEvent = modulename
	AgentGateClassName = className

	if net_module.getOS() == "Linux" then
		AgentGateLocalSvrUnixObj = IServer.new("UnixLocal-World", ServerLocalEvent, cUnixSocketName, 0, iLocalTimeOut, net_module.UnixLocal, false, bNoDelay)
		if not AgentGateLocalSvrUnixObj:Listen() then
			print(debug.traceback(), "\n", string.format("AgentGate.Init %s Listen Failed. cLocalIP=%s iLocalPort=%s", AgentGateLocalSvrUnixObj:GetName(), cUnixSocketName, 0))
			return false
		end
	end

	AgentGateLocalSvrIPObj = IServer.new("IpV4-World", ServerLocalEvent, cLocalIP, iLocalPort, iLocalTimeOut, net_module.IpV4, false, bNoDelay)
	if not AgentGateLocalSvrIPObj:Listen() then
		print(debug.traceback(), "\n", string.format("AgentGate.Init %s Listen Failed. cLocalIP=%s iLocalPort=%s", AgentGateLocalSvrIPObj:GetName(), cLocalIP, iLocalPort))
		return false
	end

	AgentGateRemoteSvrObj = IServerPlayer.new("IpV4-player", ServerRemotEvent, cRemoteIP, iRemotePort, iRemoteTimeOut, net_module.IpV4, false, bNoDelay)
	if not AgentGateRemoteSvrObj:Listen() then
		print(debug.traceback(), "\n", string.format("AgentGate.Init %s Listen Failed. cLocalIP=%s iLocalPort=%s", AgentGateRemoteSvrObj:GetName(), cRemoteIP, iRemotePort))
		return false
	end

	return true
end

return util.ReadOnlyTable(AgentGate)