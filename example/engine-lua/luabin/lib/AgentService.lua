local net_module = require "ccorenet"
local CoreTool = require "CoreTool"
local IClient = require "IClient"
local util = require "util"
require "class"

local IAgentServiceNetFunc_OnRecvCall = "OnRecvCall"
local IAgentServiceNetFunc_OnRecvCommon = "OnRecvCommon"

local AgentService = {}

local AgentServiceEvent = {}
local AgentServiceClassName = ""

local AgentServiceRemoteSvrObj = nil

-----------------------------------------------------------------
local ServerClientEvent = {}

function ServerClientEvent.OnRecvCall(IClientObj, targetName, proto, data)
	AgentServiceEvent[IAgentServiceNetFunc_OnRecvCall](proto, data)
end

function ServerClientEvent.OnRecvCommon(IClientObj, targetName, proto, data)
	AgentServiceEvent[IAgentServiceNetFunc_OnRecvCommon](proto, data)
end

function ServerClientEvent.OnConnect(IClientObj, ip)
end

function ServerClientEvent.OnDisConnect(IClientObj)
end

function ServerClientEvent.OnConnectFailed(IClientObj)
end
-----------------------------------------------------------------

function AgentService.SendRemote(remote_socket, proto, data)
	return AgentServiceRemoteSvrObj:SendRemoteData(remote_socket, proto, data)
end

function AgentService.SendData(targetName, proto, data)
	return AgentServiceRemoteSvrObj:SendData(targetName, proto, data)
end

function AgentService:CallData(targetName, proto, data, timeout_millsec)
	return AgentServiceRemoteSvrObj:CallData(targetName, proto, data, timeout_millsec)
end

function AgentService.RetCallData(data)
	return AgentServiceRemoteSvrObj:RetCallData(data)
end

function AgentService.GetName()
	return AgentServiceClassName..""
end

function AgentService.Init(className, modulename, cRemoteIP, iRemotePort, cUnixSocketName, iTimeOut, iConnectTimeOut, bNoDelay)
	-- 模块modulename中必须是table，同时必须有下面的key

	if type(modulename) ~= type({}) then
		print(debug.traceback(), "\n", "AgentService.Init modulename not a table")
		return false
	end

	if not next(modulename) then
		print(debug.traceback(), "\n", string.format("AgentService.Init modulename is empty"))
		return false
	end

	local funtList = {IAgentServiceNetFunc_OnRecvCall, IAgentServiceNetFunc_OnRecvCommon}
	for _, funtname in pairs(funtList) do
		if not modulename[funtname] then
			print(string.format(debug.traceback(), "\n", "AgentService.Init modulename not has key=%s", funtname))
			return false
		end
	end

	-- 赋值
	AgentServiceEvent = modulename
	AgentServiceClassName = className

	local iDomain = (net_module.getOS() == "Linux") and net_module.UnixLocal or net_module.IpV4

	if iDomain == net_module.IpV4 then
		AgentServiceRemoteSvrObj = IClient.new(AgentServiceClassName, ServerClientEvent, cRemoteIP, iRemotePort, iTimeOut, iConnectTimeOut, iDomain, true, bNoDelay)
	elseif iDomain == net_module.UnixLocal then
		AgentServiceRemoteSvrObj = IClient.new(AgentServiceClassName, ServerClientEvent, cUnixSocketName, 0, iTimeOut, iConnectTimeOut, iDomain, true, bNoDelay)
	else
		print(string.format(debug.traceback(), "\n", "AgentService.Init iDomain=%s error", iDomain))
		return false
	end

	if not AgentServiceRemoteSvrObj:Connect() then
		print(debug.traceback(), "\n", string.format("AgentService.Connect %s Listen Failed. cLocalIP=%s iLocalPort=%s", AgentGateRemoteSvrObj:GetName(), cRemoteIP, iRemotePort))
		return false
	end

	return true
end

return util.ReadOnlyTable(AgentService)