local IServerPlayer = require "IServerPlayer"
local reloadmodule = require "reloadmodule"
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
local IAgentGateNetFunc_OnSystem = "OnSystem"

local local_modulename = ...

local AgentGate = {}

local AgentGateEventMudleName = nil
local AgentGateClassName = ""
local AgentGateHotfixModuleName = nil

local AgentGateRemoteSvrObj = nil
local AgentGateLocalSvrIPObj = nil
local AgentGateLocalSvrUnixObj = nil

-- 服务注册列表
local AgentGateRegSvrList = {}

-----------------------------------------------------------------
function AgentGate.OnSPlayerConnect(IClientObj, socket, ip)
	package.loaded[AgentGateEventMudleName][IAgentGateNetFunc_OnRemoteConnect](socket, ip)
end

function AgentGate.OnSPlayerDisConnect(IClientObj, socket)
	package.loaded[AgentGateEventMudleName][IAgentGateNetFunc_OnRemoteDisConnect](socket)
end

function AgentGate.OnSPlayerRecv(IClientObj, socket, proto, data)
	package.loaded[AgentGateEventMudleName][IAgentGateNetFunc_OnRemoteRecv](socket, proto, data)
end
-----------------------------------------------------------------


-----------------------------------------------------------------
function AgentGate.OnSRecvCall(IServerObj, socket, targetName, proto, data)
	if targetName == AgentGateClassName then
		package.loaded[AgentGateEventMudleName][IAgentGateNetFunc_OnLocalRecvCall](IServerObj, socket, proto, data)
		return
	end
	if not AgentGateRegSvrList[targetName] then
		print(debug.traceback(), "\n", string.format("AgentGate.OnSRecvCall not find targetName=%s error", targetName))
		return
	end
	local IServerObj_, socket_ = table.unpack(AgentGateRegSvrList[targetName])
	local ret, sendData = IServerObj_:CallData(socket_, targetName, proto, data)
	if ret then
		IServerObj:RetCallData(socket, sendData)
	end
end

function AgentGate.OnSRecvCommon(IServerObj, socket, targetName, proto, data)
	if targetName == AgentGateClassName then
		package.loaded[AgentGateEventMudleName][IAgentGateNetFunc_OnLocalRecvCommon](IServerObj, socket, proto, data)
		return
	end
	if not AgentGateRegSvrList[targetName] then
		print(debug.traceback(), "\n", string.format("AgentGate.OnSRecvCommon not find targetName=%s error", targetName))
		return
	end
	local IServerObj_, socket_ = table.unpack(AgentGateRegSvrList[targetName])
	IServerObj_:SendData(socket_, targetName, proto, data)
end

function AgentGate.OnSSystem(IServerObj, socket, proto, data)
	package.loaded[AgentGateEventMudleName][IAgentGateNetFunc_OnSystem](IServerObj, socket, proto, data)
end

function AgentGate.OnSRecvRemote(IServerObj, socket, remote_socket, proto, data)
	AgentGateRemoteSvrObj:SendData(remote_socket, proto, data)
end

function AgentGate.OnSConnect(IServerObj, socket, ip)
end

function AgentGate.OnSDisConnect(IServerObj, socket)
	local name = IServerObj:GetSocketRegName(socket)
	if name then
		local _, socket_ = table.unpack(AgentGateRegSvrList[name])
		if socket_ == socket then
			AgentGateRegSvrList[name] = nil
			print(string.format("AgentGate.OnSDisConnect unregister name=%s ok", name))
		end
	end
end

function AgentGate.OnSRegister(IServerObj, socket, name)
	if IServerObj:GetSocketRegName(socket) ~= name then
		IServerObj:DisConnect(socket)
		print(debug.traceback(), "\n", string.format("AgentGate.OnSRegister name=%s error", name))
		return
	end
	if AgentGateRegSvrList[name] or AgentGateClassName == name then
		IServerObj:DisConnect(socket)
		print(debug.traceback(), "\n", string.format("AgentGate.OnSRegister name=%s AgentGateClassName=%s is same", name, AgentGateClassName))
		return
	end
	AgentGateRegSvrList[name] = table.pack(IServerObj, socket)
	print(string.format("AgentGate.OnSRegister register name=%s ok", name))
end
-----------------------------------------------------------------
function AgentGate.CloseRemote(remote_socket)
	return AgentGateRemoteSvrObj:DisConnect(remote_socket)
end

function AgentGate.SendRemote(remote_socket, proto, data)
	return AgentGateRemoteSvrObj:SendData(remote_socket, proto, data)
end

function AgentGate.BroadcastRemote(socketArray, proto, data)
	return AgentGateRemoteSvrObj:BroadcastData(socketArray, proto, data)
end

function AgentGate.SendData(targetName, proto, data)
	if not AgentGateRegSvrList[targetName] then
		print(debug.traceback(), "\n", string.format("AgentGate.SendData not find targetName=%s error", targetName))
		return false
	end
	local IServerObj_, socket_ = table.unpack(AgentGateRegSvrList[targetName])
	return IServerObj_:SendData(socket_, targetName, proto, data)
end

function AgentGate.SendSystemData(targetName, proto, data)
	if not AgentGateRegSvrList[targetName] then
		print(debug.traceback(), "\n", string.format("AgentGate.SendSystemData not find targetName=%s error", targetName))
		return false
	end
	local IServerObj_, socket_ = table.unpack(AgentGateRegSvrList[targetName])
	return IServerObj_:SendSystemData(socket_, proto, data)
end

function AgentGate.CallData(targetName, proto, data, timeout_millsec)
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

function AgentGate.Hotfix()
	if not AgentGateHotfixModuleName then
		print(debug.traceback(), "\n", "AgentGate.Hotfix modulename is nil")
		return false
	end
	return reloadmodule.reload(AgentGateHotfixModuleName)
end

function AgentGate.IsServiceRegister(serviceName)
	return AgentGateRegSvrList[serviceName] and true or false
end

-- 注册服务器列表
AgentGate.AgentGateRegSvrList = AgentGateRegSvrList

function AgentGate.Init(className, modulename, hotfixModuleName, cRemoteIP, iRemotePort, iRemoteTimeOut, cLocalIP, iLocalPort, cUnixSocketName, iLocalTimeOut, bNoDelay)
	-- 模块modulename中必须是table，同时必须有下面的key
	local packageName = package.loaded[modulename]

	if type(packageName) ~= type({}) then
		print(debug.traceback(), "\n", "AgentGate.Init modulename not a table")
		return false
	end

	local bEmpty = true
	local funtList = {IAgentGateNetFunc_OnLocalRecvCall, IAgentGateNetFunc_OnLocalRecvCommon, IAgentGateNetFunc_OnRemoteRecv, IAgentGateNetFunc_OnRemoteConnect, IAgentGateNetFunc_OnRemoteDisConnect, IAgentGateNetFunc_OnSystem}
	for _, funtname in pairs(funtList) do
		if not packageName[funtname] then
			print(debug.traceback(), "\n", string.format("AgentGate.Init modulename not has key=%s", funtname))
			return false
		end
		bEmpty = false
	end

	if not bEmpty then
		print(debug.traceback(), "\n", string.format("AgentGate.Init modulename is empty"))
		return false
	end

	if hotfixModuleName then
		if not reloadmodule.reloadtest(hotfixModuleName) then
			return false
		end
	end

	-- 赋值
	AgentGateEventMudleName = tostring(modulename)
	AgentGateClassName = tostring(className)
	AgentGateHotfixModuleName = tostring(hotfixModuleName)

	if net_module.getOS() == "Linux" then
		AgentGateLocalSvrUnixObj = IServer.new("UnixLocal-World", local_modulename, cUnixSocketName, 0, iLocalTimeOut, net_module.UnixLocal, false, bNoDelay)
		if not AgentGateLocalSvrUnixObj:Listen() then
			print(debug.traceback(), "\n", string.format("AgentGate.Init %s Listen Failed. cLocalIP=%s iLocalPort=%s", AgentGateLocalSvrUnixObj:GetName(), cUnixSocketName, 0))
			return false
		end
	end

	AgentGateLocalSvrIPObj = IServer.new("IpV4-World", local_modulename, cLocalIP, iLocalPort, iLocalTimeOut, net_module.IpV4, false, bNoDelay)
	if not AgentGateLocalSvrIPObj:Listen() then
		print(debug.traceback(), "\n", string.format("AgentGate.Init %s Listen Failed. cLocalIP=%s iLocalPort=%s", AgentGateLocalSvrIPObj:GetName(), cLocalIP, iLocalPort))
		return false
	end

	AgentGateRemoteSvrObj = IServerPlayer.new("IpV4-player", local_modulename, cRemoteIP, iRemotePort, iRemoteTimeOut, net_module.IpV4, false, bNoDelay)
	if not AgentGateRemoteSvrObj:Listen() then
		print(debug.traceback(), "\n", string.format("AgentGate.Init %s Listen Failed. cLocalIP=%s iLocalPort=%s", AgentGateRemoteSvrObj:GetName(), cRemoteIP, iRemotePort))
		return false
	end

	return true
end

return util.ReadOnlyTable(AgentGate)