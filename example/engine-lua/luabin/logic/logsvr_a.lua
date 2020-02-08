local AgentService = require "AgentService"
local net_module = require "ccorenet"
local util = require "util"
require "class"

local event = {}
function event.OnRecvCall(proto, data)
	print("OnRecvCall", proto, data)
	AgentService.RetCallData("back....")
	local socket = table.unpack(data)
	AgentService.SendRemote(socket, 456, "sssss")
end

function event.OnRecvCommon(proto, data)
	print("OnRecvCommon", proto, data)
	AgentService.SendData("gate", "ssssss", "OnRecvCommon")
end

function event.framefunc()
	net_module.addtimer(event, "framefunc", 1000)
	--local ret, data = AgentService:CallData("gate", "proto", "data", 1000)
	--util.print(table.pack(ret, data))
	--print("ddddddddddddddddddddsssssssssssssssssssssssssssss")
	-------------AgentService.SendData("svr_b", "ssssss", "svr_a send ")
end



net_module.init("./test_svr_a.log", 65535, true)
net_module.addtimer(event, "framefunc", 1000)

AgentService.Init("svr_a", event, "127.0.0.1", 6666, "sssssss", 60000, 10000, false)

net_module.start()

net_module.fin()