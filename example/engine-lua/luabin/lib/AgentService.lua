local reloadmodule = require "reloadmodule"
local net_module = require "ccorenet"
local CoreTool = require "CoreTool"
local IClient = require "IClient"
local util = require "util"
require "class"

local IAgentServiceNetFunc_OnRecvCall = "OnRecvCall"
local IAgentServiceNetFunc_OnRecvCommon = "OnRecvCommon"
local IAgentServiceNetFunc_OnSystem = "OnSystem"

local local_modulename = ...

local AgentService = {}

local AgentServiceEventMudleName = nil
local AgentServiceClassName = ""
local AgentServiceHotfixModuleName = nil

local AgentServiceRemoteSvrObj = nil

-----------------------------------------------------------------
function AgentService.OnCRecvCall(IClientObj, targetName, proto, data)
	package.loaded[AgentServiceEventMudleName][IAgentServiceNetFunc_OnRecvCall](proto, data)
end

function AgentService.OnCRecvCommon(IClientObj, targetName, proto, data)
	package.loaded[AgentServiceEventMudleName][IAgentServiceNetFunc_OnRecvCommon](proto, data)
end

function AgentService.OnCSystem(IClientObj, proto, data)
	package.loaded[AgentServiceEventMudleName][IAgentServiceNetFunc_OnSystem](proto, data)
end

function AgentService.OnCConnect(IClientObj, ip)
end

function AgentService.OnCDisConnect(IClientObj)
end

function AgentService.OnCConnectFailed(IClientObj)
end
-----------------------------------------------------------------

function AgentService.SendRemote(remote_socket, proto, data)
	return AgentServiceRemoteSvrObj:SendRemoteData(remote_socket, proto, data)
end

function AgentService.SendData(targetName, proto, data)
	return AgentServiceRemoteSvrObj:SendData(targetName, proto, data)
end

function AgentService.SendSystemData(proto, data)
	return AgentServiceRemoteSvrObj:SendSystemData(proto, data)
end

function AgentService.CallData(targetName, proto, data, timeout_millsec)
	return AgentServiceRemoteSvrObj:CallData(targetName, proto, data, timeout_millsec)
end

function AgentService.RetCallData(data)
	return AgentServiceRemoteSvrObj:RetCallData(data)
end

function AgentService.GetName()
	return AgentServiceClassName..""
end

function AgentService.Hotfix()
	if not AgentServiceHotfixModuleName then
		print(debug.traceback(), "\n", "AgentService.Hotfix modulename is nil")
		return false
	end
	return reloadmodule.reload(AgentServiceHotfixModuleName)
end

function AgentService.Init(className, modulename, hotfixModuleName, cRemoteIP, iRemotePort, cUnixSocketName, iTimeOut, iConnectTimeOut, bNoDelay)
	-- 模块modulename中必须是table，同时必须有下面的key
	local packageName = package.loaded[modulename]

	if type(packageName) ~= type({}) then
		print(debug.traceback(), "\n", "AgentService.Init modulename not a table")
		return false
	end

	local bEmpty = true
	local funtList = {IAgentServiceNetFunc_OnRecvCall, IAgentServiceNetFunc_OnRecvCommon, IAgentServiceNetFunc_OnSystem}
	for _, funtname in pairs(funtList) do
		if not packageName[funtname] then
			print(debug.traceback(), "\n", string.format("AgentService.Init modulename not has key=%s", funtname))
			return false
		end
		bEmpty = false
	end

	if not bEmpty then
		print(debug.traceback(), "\n", string.format("AgentService.Init modulename is empty"))
		return false
	end

	if hotfixModuleName then
		if not reloadmodule.reloadtest(hotfixModuleName) then
			return false
		end
	end

	-- 赋值
	AgentServiceEventMudleName = tostring(modulename)
	AgentServiceClassName = tostring(className)
	AgentServiceHotfixModuleName = tostring(hotfixModuleName)

	local iDomain = (net_module.getOS() == "Linux") and net_module.UnixLocal or net_module.IpV4

	if iDomain == net_module.IpV4 then
		AgentServiceRemoteSvrObj = IClient.IClientClass.new(AgentServiceClassName, local_modulename, cRemoteIP, iRemotePort, iTimeOut, iConnectTimeOut, iDomain, true, bNoDelay)
	elseif iDomain == net_module.UnixLocal then
		AgentServiceRemoteSvrObj = IClient.IClientClass.new(AgentServiceClassName, local_modulename, cUnixSocketName, 0, iTimeOut, iConnectTimeOut, iDomain, true, bNoDelay)
	else
		print(debug.traceback(), "\n", string.format("AgentService.Init iDomain=%s error", iDomain))
		return false
	end

	if not AgentServiceRemoteSvrObj:Connect() then
		print(debug.traceback(), "\n", string.format("AgentService.Connect %s Listen Failed. cLocalIP=%s iLocalPort=%s", AgentServiceRemoteSvrObj:GetName(), cRemoteIP, iRemotePort))
		return false
	end

	return true
end

return util.ReadOnlyTable(AgentService)