local AgentService = require "AgentService"
local ccoroutine = require "ccoroutine"
local net_module = require "ccorenet"
local util = require "util"
local timer = require "timer"
require "class"

local local_modulename = ...
timer.register(local_modulename)

local event = {}
function event.OnRecvCall(proto, data)
	print("OnRecvCall", proto, data)
	--AgentService.RetCallData("back....")
	--local socket = table.unpack(data)
	--AgentService.SendRemote(socket, 456, "sssss")
	AgentService.RetCallData(data)
end

function event.OnRecvCommon(proto, data)
	print("OnRecvCommon", proto, data)
end

function event.OnSystem(proto, data)
	print(proto, data)
end

function event.OnRegister()
end

function event.framefunc()
	timer.addtimer(local_modulename, "framefunc", 10000)

	local a, b = AgentService.CallData(".watchdog", "eeeeeeeeeee", {1,2,3})
	util.print(table.pack(a, b))

	AgentService.SendData(".watchdog", "eeeeeeeeeeeee", {1,2,3})

	local a, b = AgentService.CallData("svr_a", "me call", {1,2,3})
	util.print(table.pack(a, b))

	AgentService.SendData("svr_a", "me copmmon", {1,2,3})
	
	local report = net_module.statReport((net_module.getOS() ~= "Linux") and (30 * 1000) or 0)
	if report then
		local sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, timerCount = table.unpack(report)
		print(os.date("%Y-%m-%d %H:%M:%S"), string.format("statreport sendNumSpeed=%s sendByteSpeed=%s recvNumSpeed=%s recvByteSpeed=%s timerCount=%s %s, %s pool=%s", 
				sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, timerCount, ccoroutine.count_session_coroutine_id(), ccoroutine.count_session_id_coroutine(), ccoroutine.count_coroutine_pool()))
		collectgarbage()
	end
end

return event
