local AgentService = require "AgentService"
local net_module = require "ccorenet"
local ccoroutine = require "ccoroutine"
local util = require "util"
require "class"

local event = {}
function event.OnRecvCall(proto, data)
	print("OnRecvCall", proto, data)
	AgentService.RetCallData("back....")
end

function event.OnRecvCommon(proto, data)
	print("OnRecvCommon", proto, data)
end

function event.OnSystem(proto, data)
	
end

function event.framefunc()
	net_module.addtimer(event, "framefunc", 1000)
	--local ret, data = AgentService:CallData("svr_a", "proto", "call svr_a recv ", 1000)
	--util.print(table.pack(ret, data))
	--print("ddddddddddddddddddddsssssssssssssssssssssssssssss")
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

AgentService.Init("svr_b", event, nil, "127.0.0.1", 6666, "sssssss", 60000, 10000, false)

net_module.start()

net_module.fin()