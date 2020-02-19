local msgpack = require "msgpack53"
local ccorenet = require "ccorenet"
local reloadmodule = require "reloadmodule"
local ccoroutine = require "ccoroutine"
local util = require "util"
local CoreNet = require "CoreNet"
local CoreTool = require "CoreTool"
local timer = require "timer"

local local_modulename = ...
timer.register(local_modulename)

local svr_event = {}

local count = 0
local socketObj = nil
function svr_event.OnSConnect(IServerClassObj, socket, ip)
	--print("connect----", socket, ip)
	--IServerClassObj:DisConnect(socket)
	socketObj = socket
	count = count + 1
end

function svr_event.OnSDisConnect(IServerClassObj, socket)
	--print("disconnect", socket)
	count = count - 1
end

function svr_event.OnSRegister(IServerClassObj, socket, regname)
	--print("OnRegister", socket, regname)
end

function svr_event.OnSSystem(IServerClassObj, socket, proto, ret)
	print("OnSystem", socket, proto, ret)
	IServerClassObj:SendSystemData(socket, "fffffffffffffff", "ssssssssssssss127.0.0.1")
end

function svr_event.OnSRecvCall(IServerClassObj, socket, targetName, proto, ret)
	--print("OnRecvCall-----------------", socket, targetName, proto, ret)
	if "testCallData" == proto then
		--util.print(msgpack.unpack(ret))
		IServerClassObj:RetCallData(socket, ret)
		--local oo, data = IServerClassObj:CallData(socket, "testCallData", {"testCallData", ret})
		--print(oo)
		--if type(data) == type({}) then 
			--util.print(data) 
	--	else 
		--	print(data) 
		--end
		return
	end
end

function svr_event.OnSRecvCommon(IServerClassObj, socket, targetName, proto, ret)
	--print("OnRecvCommon-----------------", socket, targetName, proto, ret)
	IServerClassObj:SendData(socket, targetName, proto, ret)
end

function svr_event.OnSRecvRemote(IServerClassObj, socket, remote_socket, proto, ret)
	--print("OnRecvRemote-----------------", socket, remote_socket, type(remote_socket), proto, type(proto), ret)
	
end

function svr_event.test()
	--reloadmodule.reloadlist({"testcorenetsvrevent/event"})
	--print("7777777777777777777")
end

function svr_event.test1()
	--print("88888888888888888888888")
end

local timero = CoreTool.GetTickCount()
function svr_event.framefunc()

	
	--if 0 == 0 then return end
	--print("framefunctttttttttt")
	--if CoreTool.GetTickCount() - timero < 1000 *17 then
		--util.print_table(ccorenet.IClientList)
		--print("-------------------------------------------------------------------")
		--util.print_table(ccorenet.IServerList)
		if socketObj then
			--local oo, data = ccorenet.getGlobalObj("serverObj"):CallData(socketObj, "testCallData", {"12345"})
		end
	--end

	local report = ccorenet.statreport()
	if report then
		svr_event.test()
		svr_event.test1()
		local sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, printNumSpeed, printByteSpeed, timerCount = table.unpack(report)
		print(os.date("%Y-%m-%d %H:%M:%S"), string.format("statreport sendNumSpeed=%s sendByteSpeed=%s recvNumSpeed=%s recvByteSpeed=%s pingNumSpeed=%s pingByteSpeed=%s timerCount=%s %s, %s pool=%s", 
				sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, printNumSpeed, printByteSpeed, timerCount, ccoroutine.count_session_coroutine_id(), ccoroutine.count_session_id_coroutine(), ccoroutine.count_coroutine_pool()))
		collectgarbage()
	end
	svr_event.test1()
end

function svr_event.timerfunc(...)
	--print("timerfunc", ...)
	timer.addtimer(local_modulename, "timerfunc", 1000, 1, 2, 3)
	svr_event.framefunc()
	
end




return svr_event