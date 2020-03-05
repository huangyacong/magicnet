local msgpack = require "msgpack53"
local ccorenet = require "ccorenet"

local CoreTool = require "CoreTool"
local ccoroutine = require "ccoroutine"
local CoreTool = require "CoreTool"
local util = require "util"
local timer = require "timer"

local local_modulename = ...
timer.register(local_modulename)

local client_event = {}

function client_event.OnCPlayerConnect(IClientClassObj, ip)
	print("c_connect", IClientClassObj, ip)
	--IClientClassObj:SendSystemData("xxxxxxxxxxxxxxxx", ip)
end

function client_event.OnCPlayerConnectFailed(IClientClassObj)
	print("OnConnectFailed", IClientClassObj)
end

function client_event.OnCPlayerDisConnect(IClientClassObj)
	print("c_disconnect", IClientClassObj)
end

function client_event.OnCPlayerSendPacketAttach(IClientClassObj)
	print("OnSendPacketAttach", IClientClassObj)
end

function client_event.OnCPlayerPing(IClientClassObj)
	print("OnPing", IClientClassObj)
end

function client_event.OnCPlayerRecv(IClientClassObj, proto, data)
	print("OnRecvCommon", IClientClassObj, proto, data)
end

function client_event.framefunc()
	local report = ccorenet.statReport((ccorenet.getOS() ~= "Linux") and (30 * 1000) or 0)
	if report then
		local sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, timerCount = table.unpack(report)
		print(os.date("%Y-%m-%d %H:%M:%S"), string.format("statreport sendNumSpeed=%s sendByteSpeed=%s recvNumSpeed=%s recvByteSpeed=%s timerCount=%s %s, %s pool=%s", 
				sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, timerCount, ccoroutine.count_session_coroutine_id(), ccoroutine.count_session_id_coroutine(), ccoroutine.count_coroutine_pool()))
	end
end

function client_event.session_id_coroutine_timeout()
	timer.addtimer(local_modulename, "session_id_coroutine_timeout", 500)
	for i = 0, 0 do
		local name = string.format("clientObj%s", i)
		ccorenet.getGlobalObj(name):SendData(1213, "127.0.0.1")
	end
end

return client_event