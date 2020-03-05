local reloadmodule = require "reloadmodule"
local net_module = require "ccorenet"
local CoreTool = require "CoreTool"
local IClient = require "IClient"
local util = require "util"
require "class"

local IAgentServiceCrossNetFunc_OnCrossRecvCall = "OnCrossRecvCall"
local IAgentServiceCrossNetFunc_OnCrossRecvCommon = "OnCrossRecvCommon"
local IAgentServiceCrossNetFunc_OnCrossRegister = "OnCrossRegister"

local local_modulename = ...

local AgentServiceCross = {}

local AgentServiceCrossIsLocalService = false
local AgentServiceCrossEventMudleName = nil
local AgentServiceCrossClassName = ""
local AgentServiceCrossHotfixModuleName = nil

local AgentServiceCrossList = {}
local function GetAgentServiceCrossObj(serviceName)
	return AgentServiceCrossList[serviceName]
end

-----------------------------------------------------------------
function AgentServiceCross.OnCRecvCall(IClientObj, targetName, proto, data)
	package.loaded[AgentServiceCrossEventMudleName][IAgentServiceCrossNetFunc_OnRecvCall](IClientObj:GetPrivateData(), proto, data)
end

function AgentServiceCross.OnCRecvCommon(IClientObj, targetName, proto, data)
	package.loaded[AgentServiceCrossEventMudleName][IAgentServiceCrossNetFunc_OnRecvCommon](IClientObj:GetPrivateData(), proto, data)
end

function AgentServiceCross.OnCRegister(IClientObj)
	package.loaded[AgentServiceCrossEventMudleName][IAgentServiceCrossNetFunc_OnCrossRegister](IClientObj:GetPrivateData())
end

function AgentServiceCross.OnCRemoteConnect(IClientObj)
	
end

function AgentServiceCross.OnCRemoteDisConnect(IClientObj)
	
end

function AgentServiceCross.OnCRemoteRecvData(IClientObj, proto, data)
	
end

function AgentServiceCross.OnCConnect(IClientObj, ip)
end

function AgentServiceCross.OnCDisConnect(IClientObj)
end

function AgentServiceCross.OnCConnectFailed(IClientObj)
end

-----------------------------------------------------------------

function AgentServiceCross.SendRemote(serviceName, remote_socket, proto, data)
	return GetAgentServiceCrossObj(serviceName):SendRemoteData(remote_socket, proto, data)
end

function AgentServiceCross.SendData(serviceName, targetName, proto, data)
	return GetAgentServiceCrossObj(serviceName):SendData(targetName, proto, data)
end

function AgentServiceCross.CallData(serviceName, targetName, proto, data, timeout_millsec)
	return GetAgentServiceCrossObj(serviceName):CallData(targetName, proto, data, timeout_millsec)
end

function AgentServiceCross.RetCallData(serviceName, data)
	return GetAgentServiceCrossObj(serviceName):RetCallData(data)
end

function AgentServiceCross.GetName()
	return AgentServiceCrossClassName..""
end

function AgentServiceCross.Hotfix()
	if not AgentServiceCrossHotfixModuleName then
		print(debug.traceback(), "\n", "AgentServiceCross.Hotfix modulename is nil")
		return false
	end
	return reloadmodule.reload(AgentServiceCrossHotfixModuleName)
end

function AgentServiceCross.IsLocalService()
	return AgentServiceCrossIsLocalService
end

function AgentServiceCross.AddService(serviceName, cRemoteIP, iRemotePort, cUnixSocketName, iTimeOut, iConnectTimeOut, bNoDelay)
	if not AgentServiceCrossEventMudleName then
		print(debug.traceback(), "\n", string.format("AgentServiceCross.AddService service=%s not run AgentServiceCross.Init", serviceName))
		return false
	end
	if AgentServiceCrossList[serviceName] then
		print(debug.traceback(), "\n", string.format("AgentServiceCross.AddService service=%s is reg", serviceName))
		return false
	end

	if AgentServiceCrossIsLocalService and next(AgentServiceCrossList) then
		print(debug.traceback(), "\n", string.format("AgentServiceCross.AddService local service only open one service"))
		return false
	end

	local obj = nil
	if not AgentServiceCrossIsLocalService or (AgentServiceCrossIsLocalService and net_module.getOS() ~= "Linux") then
		obj = IClient.IClientClass.new(AgentServiceCrossClassName, local_modulename, cRemoteIP, iRemotePort, iTimeOut, iConnectTimeOut, net_module.IpV4, true, bNoDelay)
	else
		obj = IClient.IClientClass.new(AgentServiceCrossClassName, local_modulename, cUnixSocketName, 0, iTimeOut, iConnectTimeOut, net_module.UnixLocal, true, bNoDelay)
	end

	if not obj:Connect() then
		print(debug.traceback(), "\n", string.format("AgentServiceCross.AddService %s Failed. cLocalIP=%s iLocalPort=%s", serviceName, cRemoteIP, iRemotePort))
		return false
	end

	obj:SetPrivateData(serviceName)
	AgentServiceCrossList[serviceName] = obj
	return true
end

function AgentServiceCross.DelService(serviceName)
	if not AgentServiceCrossList[serviceName] then
		return
	end
	AgentServiceCrossList[serviceName]:DisConnect()
	AgentServiceCrossList[serviceName]:SetPrivateData()
	AgentServiceCrossList[serviceName] = nil
end

function AgentServiceCross.Init(className, modulename, hotfixModuleName, bLocalService, serviceConfArray, iTimeOut, iConnectTimeOut, bNoDelay)
	-- 模块modulename中必须是table，同时必须有下面的key
	local packageName = package.loaded[modulename]

	if type(packageName) ~= type({}) then
		print(debug.traceback(), "\n", "AgentServiceCross.Init modulename not a table")
		return false
	end

	local bEmpty = true
	local funtList = {IAgentServiceCrossNetFunc_OnCrossRecvCall, IAgentServiceCrossNetFunc_OnCrossRecvCommon, IAgentServiceCrossNetFunc_OnCrossRegister}
	for _, funtname in pairs(funtList) do
		if not packageName[funtname] then
			print(debug.traceback(), "\n", string.format("AgentServiceCross.Init modulename not has key=%s", funtname))
			return false
		end
		bEmpty = false
	end

	if bEmpty then
		print(debug.traceback(), "\n", string.format("AgentServiceCross.Init modulename is empty"))
		return false
	end

	if hotfixModuleName then
		if not reloadmodule.reloadtest(hotfixModuleName) then
			return false
		end
	end

	if bLocalService and #serviceConfArray ~= 1 then
		print(debug.traceback(), "\n", string.format("AgentServiceCross.Init local service serviceConfArray len == 1"))
		return false
	end

	-- 赋值
	AgentServiceCrossIsLocalService = bLocalService
	AgentServiceCrossEventMudleName = tostring(modulename)
	AgentServiceCrossClassName = tostring(className)
	AgentServiceCrossHotfixModuleName = tostring(hotfixModuleName)

	for _, value in ipairs(serviceConfArray) do
		local serviceName, cRemoteIP, iRemotePort, cUnixSocketName = table.unpack(value)
		if not AgentServiceCross.AddService(serviceName, cRemoteIP, iRemotePort, cUnixSocketName, iTimeOut, iConnectTimeOut, bNoDelay) then
			return false
		end
	end

	print(string.format("AgentServiceCross.Init is localservice=%s", AgentServiceCrossIsLocalService and "true" or "false"))
	return true
end

return util.ReadOnlyTable(AgentServiceCross)