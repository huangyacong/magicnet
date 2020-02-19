local AgentGate = require "AgentGate"
local net_module = require "ccorenet"
local CoreTool = require "CoreTool"
local ccoroutine = require "ccoroutine"
local util = require "util"
local timer = require "timer"

local local_modulename = ...
timer.register(local_modulename)

local event = {}


local client_list = {}
local count = 0
function event.OnLocalRecvCall(IServerObj, socket, proto, data)
	
end

function event.OnLocalRecvCommon(IServerObj, socket, proto, data)
	
end

function event.OnSystem(IServerObj, socket, proto, data)
	print("OnSystem", socket, proto, data)
	IServerObj:SendSystemData(socket, proto, data)

	AgentGate.SendSystemData("svr_a", "111111111","2222222222222222")
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
	timer.addtimer(local_modulename, "framefunc", 1000)

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
