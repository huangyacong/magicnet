local AgentGate = require "AgentGate"
local net_module = require "ccorenet"
local CoreTool = require "CoreTool"
local ccoroutine = require "ccoroutine"
local util = require "util"

local event = {}


local client_list = {}
local count = 0
function event.OnLocalRecvCall(IServerObj, socket, proto, data)
	
end

function event.OnLocalRecvCommon(IServerObj, socket, proto, data)
	
end

function event.OnRemoteRecv(socket, proto, data)
	AgentGate.SendData("svr_a", "sssss", table.pack(socket, "sssssssssssssssssssssssssssss"))
end

function event.OnRemoteConnect(socket)
client_list[socket] =socket
count = count + 1
end

function event.OnRemoteDisConnect(socket)
	client_list[socket] =nil
	count = count - 1
end

function event.framefunc()
	net_module.addtimer(event, "framefunc", 1000)

	local report = net_module.statreport()
	--util.print_table(ccorenet.IClientList)
	if report then
		local sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, timerCount = table.unpack(report)
		print(os.date("%Y-%m-%d %H:%M:%S"), string.format("statreport count=%s sendNumSpeed=%s sendByteSpeed=%s recvNumSpeed=%s recvByteSpeed=%s timerCount=%s %s, %s pool=%s", 
				count, sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, timerCount, ccoroutine.count_session_coroutine_id(), ccoroutine.count_session_id_coroutine(), ccoroutine.count_coroutine_pool()))
		collectgarbage()
	end
end

net_module.init("./test_svr.log", 65535, true)
net_module.addtimer(event, "framefunc", 1000)

AgentGate.Init("gate", event, "127.0.0.1", 8888, 60000, "127.0.0.1", 6666, "sssssss", 60000, false)

net_module.start()

net_module.fin()
