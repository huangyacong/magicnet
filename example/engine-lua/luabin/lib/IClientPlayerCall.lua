local IClientPlayer = require "IClientPlayer"
local ccoroutine = require "ccoroutine"
local net_module = require "ccorenet"
local CoreTool = require "CoreTool"
local CoreNet = require "CoreNet"
local timer = require "timer"
local util = require "util"
require "class"

local local_modulename = ...
timer.register(local_modulename)

local table = table

local IClientPlayerCall = {}

function IClientPlayerCall.OnCPlayerConnect(IClientObj, ip) end

function IClientPlayerCall.OnCPlayerDisConnect(IClientObj) end

function IClientPlayerCall.OnCPlayerPing(IClientObj) end

function IClientPlayerCall.OnCPlayerConnectFailed(IClientObj) 
	local privateProto, session_id = table.unpack(IClientObj:GetPrivateData())

	if not session_id then
		print(debug.traceback(), "\n", "IClientPlayerCall OnCPlayerConnectFailed session_id nil")
		return
	end

	local co = ccoroutine.get_session_id_coroutine(session_id)
	if not co then 
		print(debug.traceback(), "\n", "IClientPlayerCall OnCPlayerConnectFailed not find co", session_id)
		return
	end

	ccoroutine.resume(co, false, "OnCPlayerConnectFailed")
end

function IClientPlayerCall.OnCPlayerSendPacketAttach(IClientObj)
	local privateProto, session_id, data = table.unpack(IClientObj:GetPrivateData())
	IClientObj:SendData(privateProto, data)
end

function IClientPlayerCall.OnCPlayerRecv(IClientObj, proto, data)
	local privateProto, session_id = table.unpack(IClientObj:GetPrivateData())

	if not session_id then
		print(debug.traceback(), "\n", "IClientPlayerCall OnCPlayerRecv session_id nil")
		return
	end
	if privateProto ~= proto then
		print(debug.traceback(), "\n", "IClientPlayerCall OnCPlayerRecv proto error", proto)
		return
	end

	local co = ccoroutine.get_session_id_coroutine(session_id)
	if not co then 
		print(debug.traceback(), "\n", "IClientPlayerCall not find co", session_id)
		return
	end

	ccoroutine.resume(co, true, data)
end

local IClientPlayerCallClass = class()

function IClientPlayerCallClass:ctor(cIP, iPort)
	self.IClientPlayerObj = IClientPlayer.IClientPlayerClass.new("IClientPlayerCall", local_modulename, cIP, iPort, 60000, 30000, false, true)
end

function IClientPlayerCallClass:CallData(proto, data)
	
	if not self.IClientPlayerObj then
		print(debug.traceback(), "\n", "IClientPlayerCallClass Obj Not New")
		return false, "IClientPlayerCallClass Obj Not New"
	end
	if not self.IClientPlayerObj:Connect() then
		print(debug.traceback(), "\n", "IClientPlayerCallClass Connect Failed")
		return false, "IClientPlayerCallClass Connect Failed"
	end

	proto = tonumber(proto)
	local session_id = CoreTool.SysSessionId()
	self.IClientPlayerObj:SetPrivateData(table.pack(proto, session_id, data))
	local succ, msg = ccoroutine.yield_call(session_id)
	self.IClientPlayerObj:SetPrivateData()

	self.IClientPlayerObj:DisConnect()
	self.IClientPlayerObj = nil
	
	return succ, msg
end

IClientPlayerCall.IClientPlayerCallClass = IClientPlayerCallClass

return util.ReadOnlyTable(IClientPlayerCall)