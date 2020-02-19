local AgentService = require "AgentService"
local net_module = require "ccorenet"
local util = require "util"
local ccoroutine = require "ccoroutine"
local timer = require "timer"
require "class"

local local_modulename = ...
timer.register(local_modulename)

local event = {}
function event.OnRecvCall(proto, data)
	--print("OnRecvCall", proto, data)
	--AgentService.RetCallData("back....")
	--local socket = table.unpack(data)
	--AgentService.SendRemote(socket, 456, "sssss")
end

function event.OnRecvCommon(proto, data)
	local socket, xx = table.unpack(data)
	AgentService.SendRemote(socket, 456, xx)

	--print("OnRecvCommon", proto, data)
	--AgentService.SendData("gate", "ssssss", "OnRecvCommon")
end

function event.OnSystem(proto, data)
	print(proto, data)
end

function event.framefunc()
	timer.addtimer(local_modulename, "framefunc", 1000)
	AgentService.SendSystemData("ddddddddddd", "rrrrrrrrrrrrrrrrrrr")
	--local ret, data = AgentService:CallData("gate", "proto", "data", 1000)
	--util.print(table.pack(ret, data))
	--print("ddddddddddddddddddddsssssssssssssssssssssssssssss")
	-------------AgentService.SendData("svr_b", "ssssss", "svr_a send ")
	local report = net_module.statreport()
	--util.print_table(ccorenet.IClientList)
	if report then
		local sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, printNumSpeed, printByteSpeed, timerCount = table.unpack(report)
		print(os.date("%Y-%m-%d %H:%M:%S"), string.format("statreport sendNumSpeed=%s sendByteSpeed=%s recvNumSpeed=%s recvByteSpeed=%s pingNumSpeed=%s pingByteSpeed=%s timerCount=%s %s, %s pool=%s", 
				sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, printNumSpeed, printByteSpeed, timerCount, ccoroutine.count_session_coroutine_id(), ccoroutine.count_session_id_coroutine(), ccoroutine.count_coroutine_pool()))
		collectgarbage()
	end
end

return event
