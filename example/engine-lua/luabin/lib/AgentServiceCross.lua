local reloadmodule = require "reloadmodule"
local net_module = require "CCoreNet"
local CoreTool = require "CoreTool"
local IClient = require "IClient"
local util = require "util"
require "class"

local IAgentServiceCrossNetFunc_OnCrossRecvCall = "OnCrossRecvCall"
local IAgentServiceCrossNetFunc_OnCrossRecvCommon = "OnCrossRecvCommon"
local IAgentServiceCrossNetFunc_OnCrossRegister = "OnCrossRegister"

local local_modulename = ...

local AgentServiceCross = {}

local package = package
local AgentServiceCrossIsLocalService = false
local AgentServiceCrossEventMudleName = nil
local AgentServiceCrossClassName = ""
local AgentServiceCrossHotfixModuleName = nil
local AgentServiceCrossLocalLogServiceName = ""

local AgentServiceCrossList = {}
local function GetAgentServiceCrossObj(serviceid)
	return AgentServiceCrossList[serviceid]
end

-----------------------------------------------------------------
function AgentServiceCross.OnCRecvCall(IClientObj, proto, data)
	package.loaded[AgentServiceCrossEventMudleName][IAgentServiceCrossNetFunc_OnCrossRecvCall](IClientObj:GetPrivateData(), proto, data)
end

function AgentServiceCross.OnCRecvCommon(IClientObj, proto, data)
	package.loaded[AgentServiceCrossEventMudleName][IAgentServiceCrossNetFunc_OnCrossRecvCommon](IClientObj:GetPrivateData(), proto, data)
end

function AgentServiceCross.OnCRegister(IClientObj)
	package.loaded[AgentServiceCrossEventMudleName][IAgentServiceCrossNetFunc_OnCrossRegister](IClientObj:GetPrivateData())
end

function AgentServiceCross.OnCRemoteConnect(IClientObj, socket, ip)
	
end

function AgentServiceCross.OnCRemoteDisConnect(IClientObj, socket)
	
end

function AgentServiceCross.OnCRemoteRecvData(IClientObj, socket, proto, data)
	
end

function AgentServiceCross.OnCConnect(IClientObj, ip)
	print("AgentService.OnCConnect", ip)
end

function AgentServiceCross.OnCDisConnect(IClientObj)
	print(debug.traceback(), "\n", "AgentService.OnCDisConnect")
end

function AgentServiceCross.OnCConnectFailed(IClientObj)
	print(debug.traceback(), "\n", "AgentService.OnCConnectFailed")
end

-----------------------------------------------------------------

function AgentServiceCross.SendRemote(serviceid, remote_socket, proto, data)
	return GetAgentServiceCrossObj(serviceid):SendRemoteData(remote_socket, proto, data or "")
end

function AgentServiceCross.SendData(serviceid, targetName, proto, data)
	return GetAgentServiceCrossObj(serviceid):SendData(targetName, proto, data or "")
end

function AgentServiceCross.CallData(serviceid, targetName, proto, data, timeout_millsec)
	return GetAgentServiceCrossObj(serviceid):CallData(targetName, proto, data or "", timeout_millsec)
end

function AgentServiceCross.RetCallData(serviceid, data, bExtended)
	return GetAgentServiceCrossObj(serviceid):RetCallData(data, bExtended)
end

function AgentServiceCross.GetName()
	return AgentServiceCrossClassName..""
end

function AgentServiceCross.GetLocalLogServiceName()
	return AgentServiceCrossLocalLogServiceName
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

function AgentServiceCross.AddService(serviceid, cRemoteIP, iRemotePort, cUnixSocketName, iTimeOut, iConnectTimeOut, bNoDelay)
	if not AgentServiceCrossEventMudleName then
		print(debug.traceback(), "\n", string.format("AgentServiceCross.AddService service=%s not run AgentServiceCross.Init", serviceid))
		return false
	end
	if AgentServiceCrossList[serviceid] then
		print(debug.traceback(), "\n", string.format("AgentServiceCross.AddService service=%s is reg", serviceid))
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

	obj:SetPrivateData(serviceid)
	AgentServiceCrossList[serviceid] = obj
	AgentServiceCrossLocalLogServiceName = serviceid

	if not obj:Connect() then
		print(debug.traceback(), "\n", string.format("AgentServiceCross.AddService %s Failed. cLocalIP=%s iLocalPort=%s", serviceid, cRemoteIP, iRemotePort))
		return false
	end

	return true
end

function AgentServiceCross.DelService(serviceid)
	if not AgentServiceCrossList[serviceid] then
		return
	end
	AgentServiceCrossList[serviceid]:SetNotReconnect()
	AgentServiceCrossList[serviceid]:DisConnect()
	AgentServiceCrossList[serviceid]:DelRegServiceTimer()
	AgentServiceCrossList[serviceid]:DelPingTimer()
	AgentServiceCrossList[serviceid]:DelReConnectTimer()
	AgentServiceCrossList[serviceid]:SetPrivateData()
	AgentServiceCrossList[serviceid] = nil
end

function AgentServiceCross.IsExistService(serviceid)
	return AgentServiceCrossList[serviceid] ~= nil
end

function AgentServiceCross.GetAllServiceID()
	local result = {}
	for serviceid,_ in pairs(AgentServiceCrossList) do
		table.insert(result, serviceid)
	end
	return result
end

-- serviceConfArray = {{serviceid, cRemoteIP, iRemotePort, cUnixSocketName}, ......}
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

	-- 赋值
	AgentServiceCrossIsLocalService = bLocalService
	AgentServiceCrossEventMudleName = tostring(modulename)
	AgentServiceCrossClassName = tostring(className)
	AgentServiceCrossHotfixModuleName = tostring(hotfixModuleName)

	for _, value in ipairs(serviceConfArray) do
		local serviceid, cRemoteIP, iRemotePort, cUnixSocketName = table.unpack(value)
		if not AgentServiceCross.AddService(serviceid, cRemoteIP, iRemotePort, cUnixSocketName, iTimeOut, iConnectTimeOut, bNoDelay) then
			return false
		end
	end

	print(string.format("AgentServiceCross.Init is localservice=%s", AgentServiceCrossIsLocalService and "true" or "false"))
	return true
end

return util.ReadOnlyTable(AgentServiceCross)