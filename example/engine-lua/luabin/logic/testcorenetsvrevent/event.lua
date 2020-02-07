local msgpack = require "msgpack53"
local ccorenet = require "ccorenet"
local IServer = require "IServer"
local reloadmodule = require "reloadmodule"
local ccoroutine = require "ccoroutine"
local util = require "util"
local CoreNet = require "CoreNet"
local CoreTool = require "CoreTool"

local svr_event = {}

local socketObj = nil
function svr_event.OnConnect(IServerClassObj, socket, ip)
	--print("connect----", socket, ip)
	--IServerClassObj:DisConnect(socket)
	socketObj = socket
end

function svr_event.OnDisConnect(IServerClassObj, socket)
	--print("disconnect", socket)
end

function svr_event.OnRegister(IServerClassObj, socket, regname)
	print("OnRegister", socket, regname)
end

function svr_event.OnRecvCall(IServerClassObj, socket, targetName, proto, ret)
	print("OnRecvCall-----------------", socket, targetName, proto, ret)
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

function svr_event.OnRecvCommon(IServerClassObj, socket, targetName, proto, ret)
	print("OnRecvCommon-----------------", socket, targetName, proto, ret)
	IServerClassObj:SendData(socket, targetName, proto, ret)
end

function svr_event.OnRecvRemote(IServerClassObj, socket, remote_socket, proto, ret)
	print("OnRecvRemote-----------------", socket, remote_socket, proto, ret)
	
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
		local sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed = table.unpack(report)
		CoreNet.HookPrint(string.format("statreport sendNumSpeed=%s sendByteSpeed=%s recvNumSpeed=%s recvByteSpeed=%s %s %s", sendNumSpeed, sendByteSpeed, recvNumSpeed, recvByteSpeed, ccoroutine.count_session_coroutine_id(), ccoroutine.count_session_id_coroutine()))
		collectgarbage()
	end
	svr_event.test1()
end

function svr_event.timerfunc(...)
	--print("timerfunc", ...)
	ccorenet.addtimer(svr_event, "timerfunc", 1000, 1, 2, 3)
	if socketObj then
		local oo, data = ccorenet.getGlobalObj("serverObj"):CallData(socketObj, "dddddddddddddddd", "testCallData", {"12345"})
		
		print(oo)
		if type(data) == type({}) then 
			util.print(data) 
		else 
			print(data) 
		end

	end
end


ccorenet.addtimer(svr_event, "timerfunc", 1000, 1, 2, 3)

local bReusePort = true
local domain = ccorenet.IpV4
local ip = (ccorenet.getOS() == "Linux" and domain == ccorenet.UnixLocal) and "dont.del.local.socket" or "127.0.0.1"
local serverObj = IServer.new("serverObj", svr_event, ip, 8888, 1000*60, domain, bReusePort, false)
ccorenet.addGlobalObj(serverObj, serverObj:GetName())

return svr_event