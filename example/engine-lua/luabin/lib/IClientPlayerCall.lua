local IClientPlayer = require "IClientPlayer"
local coroutine_nodule = require "CCoroutine"
local timer_module = require "CCoreTimer"
local net_module = require "CCoreNet"
local CoreTool = require "CoreTool"
local CoreNet = require "CoreNet"
local util = require "util"
require "class"

local local_modulename = ...
timer_module.register(local_modulename)

local table = table

local IClientPlayerCall = {}

function IClientPlayerCall.OnCPlayerConnect(IClientObj, ip) end

function IClientPlayerCall.OnCPlayerDisConnect(IClientObj) end

function IClientPlayerCall.OnCPlayerPing(IClientObj) end

function IClientPlayerCall.OnCPlayerConnectFailed(IClientObj) 
	local privateProto, recvProto, session_id = table.unpack(IClientObj:GetPrivateData())

	if not session_id then
		print(debug.traceback(), "\n", "IClientPlayerCall OnCPlayerConnectFailed session_id nil")
		return
	end

	local co = coroutine_nodule.get_session_id_coroutine(session_id)
	if not co then 
		print(debug.traceback(), "\n", "IClientPlayerCall OnCPlayerConnectFailed not find co", session_id)
		return
	end

	coroutine_nodule.resume(co, false, "OnCPlayerConnectFailed")
end

function IClientPlayerCall.OnCPlayerSendPacketAttach(IClientObj)
	local privateProto, recvProto, session_id, data = table.unpack(IClientObj:GetPrivateData())
	IClientObj:SendData(privateProto, data or "")
end

function IClientPlayerCall.OnCPlayerRecv(IClientObj, proto, data)
	local privateDataTwo = IClientObj:GetPrivateDataTwo()
	local privateProto, recvProto, session_id = table.unpack(IClientObj:GetPrivateData())

	if recvProto == proto then
		if not session_id then
			print(debug.traceback(), "\n", "IClientPlayerCall OnCPlayerRecv session_id nil")
			return
		end

		local co = coroutine_nodule.get_session_id_coroutine(session_id)
		if not co then 
			print(debug.traceback(), "\n", "IClientPlayerCall not find co", session_id)
			return
		end

		coroutine_nodule.resume(co, true, data)
	elseif privateDataTwo[proto] then
		privateDataTwo[proto](proto, data)
	end
end

local IClientPlayerCallClass = class()

function IClientPlayerCallClass:ctor(cIP, iPort)
	self.IClientPlayerObj = IClientPlayer.IClientPlayerClass.new("IClientPlayerCall", local_modulename, cIP, iPort, 60000, 30000, false, true)
end

function IClientPlayerCallClass:SetRecvFunction(recvProto, callBackFuncName)
	local privateDataTwo = self.IClientPlayerObj:GetPrivateDataTwo()
	privateDataTwo[tonumber(recvProto)] = callBackFuncName
end

function IClientPlayerCallClass:CallData(proto, recvProto, data)
	
	if not self.IClientPlayerObj then
		print(debug.traceback(), "\n", "IClientPlayerCallClass Obj Not New")
		return false, "IClientPlayerCallClass Obj Not New"
	end
	if not self.IClientPlayerObj:Connect() then
		print(debug.traceback(), "\n", "IClientPlayerCallClass Connect Failed")
		return false, "IClientPlayerCallClass Connect Failed"
	end

	proto, recvProto = tonumber(proto), tonumber(recvProto)
	local session_id = CoreTool.SysSessionId()
	self.IClientPlayerObj:SetPrivateData(table.pack(proto, recvProto, session_id, data))
	local succ, msg = coroutine_nodule.yield_call(session_id, nil, proto)
	self.IClientPlayerObj:SetPrivateData()
	self.IClientPlayerObj:SetPrivateDataTwo()

	self.IClientPlayerObj:DisConnect()
	self.IClientPlayerObj = nil
	
	return succ, msg
end

IClientPlayerCall.IClientPlayerCallClass = IClientPlayerCallClass

return util.ReadOnlyTable(IClientPlayerCall)