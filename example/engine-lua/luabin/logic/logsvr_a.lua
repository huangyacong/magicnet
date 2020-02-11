local AgentService = require "AgentService"
local net_module = require "ccorenet"
local util = require "util"
local ccoroutine = require "ccoroutine"
require "class"

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

function event.framefunc()
	net_module.addtimer(event, "framefunc", 1000)
	--local ret, data = AgentService:CallData("gate", "proto", "data", 1000)
	--util.print(table.pack(ret, data))
	--print("ddddddddddddddddddddsssssssssssssssssssssssssssss")
	-------------AgentService.SendData("svr_b", "ssssss", "svr_a send ")
	local report = net_module.statreport()
	--util.print_table(ccorenet.IClientList)
	if report then
		local sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, timerCount = table.unpack(report)
		print(os.date("%Y-%m-%d %H:%M:%S"), string.format("statreport sendNumSpeed=%s sendByteSpeed=%s recvNumSpeed=%s recvByteSpeed=%s timerCount=%s %s, %s pool=%s", 
				sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, timerCount, ccoroutine.count_session_coroutine_id(), ccoroutine.count_session_id_coroutine(), ccoroutine.count_coroutine_pool()))
		collectgarbage()
	end
end



net_module.init("./test_svr_a.log", 65535, true)
net_module.addtimer(event, "framefunc", 1000)

AgentService.Init("svr_a", event, "127.0.0.1", 6666, "sssssss", 60000, 10000, false)

net_module.start()

net_module.fin()