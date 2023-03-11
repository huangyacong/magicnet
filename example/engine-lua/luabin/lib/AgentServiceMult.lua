local reloadmodule = require "reloadmodule"
local net_module = require "CCoreNet"
local CoreTool = require "CoreTool"
local IClient = require "IClient"
local util = require "util"
require "class"

local IAgentServiceMultNetFunc_OnMultRecvCall = "OnMultRecvCall"
local IAgentServiceMultNetFunc_OnMultRecvCommon = "OnMultRecvCommon"
local IAgentServiceMultNetFunc_OnMultRegister = "OnMultRegister"

local local_modulename = ...

local AgentServiceMult = {}

local package = package
local AgentServiceMultEventMudleName = nil

local AgentServiceMultList = {}
local function GetAgentServiceMultObj(serviceid)
	return AgentServiceMultList[serviceid]
end

-----------------------------------------------------------------
function AgentServiceMult.OnCRecvCall(IClientObj, proto, data)
	package.loaded[AgentServiceMultEventMudleName][IAgentServiceMultNetFunc_OnMultRecvCall](IClientObj:GetPrivateData(), proto, data)
end

function AgentServiceMult.OnCRecvCommon(IClientObj, proto, data)
	package.loaded[AgentServiceMultEventMudleName][IAgentServiceMultNetFunc_OnMultRecvCommon](IClientObj:GetPrivateData(), proto, data)
end

function AgentServiceMult.OnCRegister(IClientObj)
	package.loaded[AgentServiceMultEventMudleName][IAgentServiceMultNetFunc_OnMultRegister](IClientObj:GetPrivateData())
end

function AgentServiceMult.OnCRemoteConnect(IClientObj, socket, ip)
	
end

function AgentServiceMult.OnCRemoteDisConnect(IClientObj, socket)
	
end

function AgentServiceMult.OnCRemoteRecvData(IClientObj, socket, proto, data)
	
end

function AgentServiceMult.OnCConnect(IClientObj, ip)
	print("AgentService.OnCConnect", ip)
end

function AgentServiceMult.OnCDisConnect(IClientObj)
	print(debug.traceback(), "\n", "AgentService.OnCDisConnect")
end

function AgentServiceMult.OnCConnectFailed(IClientObj)
	print(debug.traceback(), "\n", "AgentService.OnCConnectFailed")
end

-----------------------------------------------------------------

function AgentServiceMult.SendRemote(serviceid, remote_socket, proto, data)
	return GetAgentServiceMultObj(serviceid):SendRemoteData(remote_socket, proto, data or "")
end

function AgentServiceMult.SendData(serviceid, targetName, proto, data)
	return GetAgentServiceMultObj(serviceid):SendData(targetName, proto, data or "")
end

function AgentServiceMult.CallData(serviceid, targetName, proto, data, timeout_millsec)
	return GetAgentServiceMultObj(serviceid):CallData(targetName, proto, data or "", timeout_millsec)
end

function AgentServiceMult.RetCallData(serviceid, data, bExtended)
	return GetAgentServiceMultObj(serviceid):RetCallData(data, bExtended)
end

function AgentServiceMult.AddService(serviceid, className, cRemoteIP, iRemotePort, iTimeOut, iConnectTimeOut, bNoDelay)

	if not AgentServiceMultEventMudleName then
		print(debug.traceback(), "\n", string.format("AgentServiceMult.AddService service=%s not run AgentServiceMult.Init", serviceid))
		return false
	end
	if AgentServiceMultList[serviceid] then
		print(debug.traceback(), "\n", string.format("AgentServiceMult.AddService service=%s is reg", serviceid))
		return false
	end

	local obj = IClient.IClientClass.new(className, local_modulename, cRemoteIP, iRemotePort, iTimeOut, iConnectTimeOut, net_module.IpV4, true, bNoDelay)

	obj:SetPrivateData(serviceid)
	AgentServiceMultList[serviceid] = obj

	if not obj:Connect() then
		print(debug.traceback(), "\n", string.format("AgentServiceMult.AddService %s Failed. cLocalIP=%s iLocalPort=%s", serviceid, cRemoteIP, iRemotePort))
		return false
	end

	return true
end

function AgentServiceMult.DelService(serviceid)
	if not AgentServiceMultList[serviceid] then
		return
	end
	AgentServiceMultList[serviceid]:SetNotReconnect()
	AgentServiceMultList[serviceid]:DisConnect()
	AgentServiceMultList[serviceid]:DelRegServiceTimer()
	AgentServiceMultList[serviceid]:DelPingTimer()
	AgentServiceMultList[serviceid]:DelReConnectTimer()
	AgentServiceMultList[serviceid]:SetPrivateData()
	AgentServiceMultList[serviceid] = nil
end

function AgentServiceMult.IsExistService(serviceid)
	return AgentServiceMultList[serviceid] ~= nil
end

function AgentServiceMult.GetService(serviceid)
	return AgentServiceMultList[serviceid]
end

function AgentServiceMult.GetAllServiceID()
	local result = {}
	for serviceid,_ in pairs(AgentServiceMultList) do
		table.insert(result, serviceid)
	end
	return result
end

-- serviceConfArray = {{serviceid, className, cRemoteIP, iRemotePort}, ......}
function AgentServiceMult.Init(modulename, serviceConfArray, iTimeOut, iConnectTimeOut, bNoDelay)
	-- 模块modulename中必须是table，同时必须有下面的key
	local packageName = package.loaded[modulename]

	if type(packageName) ~= type({}) then
		print(debug.traceback(), "\n", "AgentServiceMult.Init modulename not a table")
		return false
	end

	local bEmpty = true
	local funtList = {IAgentServiceMultNetFunc_OnMultRecvCall, IAgentServiceMultNetFunc_OnMultRecvCommon, IAgentServiceMultNetFunc_OnMultRegister}
	for _, funtname in pairs(funtList) do
		if not packageName[funtname] then
			print(debug.traceback(), "\n", string.format("AgentServiceMult.Init modulename not has key=%s", funtname))
			return false
		end
		bEmpty = false
	end

	if bEmpty then
		print(debug.traceback(), "\n", string.format("AgentServiceMult.Init modulename is empty"))
		return false
	end

	-- 赋值
	AgentServiceMultEventMudleName = tostring(modulename)

	for _, value in ipairs(serviceConfArray) do
		local serviceid, className, cRemoteIP, iRemotePort = table.unpack(value)
		if not AgentServiceMult.AddService(serviceid, className, cRemoteIP, iRemotePort, iTimeOut, iConnectTimeOut, bNoDelay) then
			return false
		end
	end

	return true
end

return util.ReadOnlyTable(AgentServiceMult)