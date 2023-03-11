local coroutine_nodule = require "CCoroutine"
local CoreNetAgent = require "CoreNetAgent"
local timer_module = require "CCoreTimer"
local dateutil = require "dateutil"
local CoreTool = require "CoreTool"
local CoreNet = require "CoreNet"
local util = require "util"
require "class"

local table = table

local CCoreNet = {}
local sys_run = true
local sys_print = nil
local sys_tips_cnt = 0
local sys_tips_time = CoreTool.GetTickCount()

local clientObj = {}
local svrObj = {}

local globalSingleObj = {}
local globalHotfixModuleName = {}

-- socket类型 
CCoreNet.IpV4 = CoreNet.DOMAIN_INET
CCoreNet.IpV6 = CoreNet.DOMAIN_INET6
CCoreNet.UnixLocal = CoreNet.DOMAIN_UNIX

-- iServer列表
CCoreNet.IServerList = svrObj

-- IClient列表
CCoreNet.IClientList = clientObj

-- 热更新的模块列表
CCoreNet.HotfixModuleName = globalHotfixModuleName

-- 添加需要热更新的模块
function CCoreNet.addHotfixModuleName(modulename)
	globalHotfixModuleName[tostring(modulename)] = true
end

-- 加入一个全局实例
function CCoreNet.addGlobalObj(object, name)
	if globalSingleObj[name] then return globalSingleObj[name] end
	globalSingleObj[name] = object
	return globalSingleObj[name]
end

-- 获取一个全局实例
function CCoreNet.getGlobalObj(name)
	return globalSingleObj[name]
end

-- 删除一个全局实例
function CCoreNet.delGlobalObj(name)
	globalSingleObj[name] = nil
end

-- 判断操作系统 Linux,Windows,Unknow
function CCoreNet.getOS()
	return CoreNet.GetOS()
end

-- 获取TIPS的值 
function CCoreNet.getTips()
	local timecount = CoreTool.GetTickCount()
	if timecount >= (sys_tips_time + 5000) then
		local ret = sys_tips_cnt / (timecount - sys_tips_time)
		sys_tips_cnt, sys_tips_time = 0, timecount
		return math.floor(ret * 1000)
	end
	return 0
end

-- 初始化
function CCoreNet.init(cLogName, iMaxClientNum, bPrintLog2Screen)
	sys_print = print
	return CoreNet.Init(cLogName, iMaxClientNum, 16, bPrintLog2Screen)
end

-- 结束
function CCoreNet.fin()
	clientObj, svrObj = {}, {}
	return CoreNet.Fin()
end

local net_event_fliter_svr = {
	[CoreNet.SOCKET_CONNECT] = function(tcpsocketobj, netevent, listenscoket, recvsocket, data)
										tcpsocketobj:OnConnect(recvsocket, data)
								end,
	[CoreNet.SOCKET_DISCONNECT] = function(tcpsocketobj, netevent, listenscoket, recvsocket, data)
										tcpsocketobj:OnDisConnect(recvsocket)
								end,
	[CoreNet.SOCKET_RECV_DATA] = function(tcpsocketobj, netevent, listenscoket, recvsocket, data)
										tcpsocketobj:OnRecv(recvsocket, data)
								end,
}

local net_event_fliter_client = {
	[CoreNet.SOCKET_CONNECT] = function(tcpsocketobj, netevent, listenscoket, recvsocket, data)
										tcpsocketobj:OnConnect(data)
								end,
	[CoreNet.SOCKET_CONNECT_FAILED] = function(tcpsocketobj, netevent, listenscoket, recvsocket, data)
										tcpsocketobj:OnConnectFailed()
								end,
	[CoreNet.SOCKET_DISCONNECT] = function(tcpsocketobj, netevent, listenscoket, recvsocket, data)
										tcpsocketobj:OnDisConnect()
								end,
	[CoreNet.SOCKET_RECV_DATA] = function(tcpsocketobj, netevent, listenscoket, recvsocket, data)
										tcpsocketobj:OnRecv(data)
								end,
}

local net_event_id = {
	[CoreNet.SOCKET_CONNECT] = CoreNet.SOCKET_CONNECT,
	[CoreNet.SOCKET_CONNECT_FAILED] = CoreNet.SOCKET_CONNECT_FAILED,
	[CoreNet.SOCKET_DISCONNECT] = CoreNet.SOCKET_DISCONNECT,
	[CoreNet.SOCKET_RECV_DATA] = CoreNet.SOCKET_RECV_DATA,
	[CoreNet.SOCKET_TIMER] = CoreNet.SOCKET_TIMER,
}

local function worker()
	local netevent, listenscoket, recvsocket, data = table.unpack(CoreNet.Read())
	
	if net_event_id[netevent] then
		if svrObj[listenscoket] then
			sys_tips_cnt = sys_tips_cnt + 1
			local tcpsocketobj = svrObj[listenscoket]
			local fliter = net_event_fliter_svr[netevent]
			if fliter then
				fliter(tcpsocketobj, netevent, listenscoket, recvsocket, data)
			else
				print(debug.traceback(), "\n", string.format("CCoreNet.read svrObj not netevent=%s listenscoket=%s recvsocket=%s", netevent, listenscoket, recvsocket))
			end
		elseif clientObj[recvsocket] then
			sys_tips_cnt = sys_tips_cnt + 1
			local tcpsocketobj = clientObj[recvsocket]
			local fliter = net_event_fliter_client[netevent]
			if fliter then
				fliter(tcpsocketobj, netevent, listenscoket, recvsocket, data)
			else
				print(debug.traceback(), "\n", string.format("CCoreNet.read clientObj not netevent=%s listenscoket=%s recvsocket=%s", netevent, listenscoket, recvsocket))
			end
		elseif CoreNet.SOCKET_TIMER == netevent then
			local doNum = timer_module.timeout()
			sys_tips_cnt = sys_tips_cnt + doNum
		else
			sys_tips_cnt = sys_tips_cnt + 1
			print(debug.traceback(), "\n", string.format("CCoreNet.read event=%s not find listenscoket=%s recvsocket=%s", netevent, listenscoket, recvsocket))
		end
	else
		sys_tips_cnt = sys_tips_cnt + 1
		print(debug.traceback(), "\n", string.format("CCoreNet.read event=%s not do listenscoket=%s recvsocket=%s", netevent, listenscoket, recvsocket))
	end
end

-- 运行
function CCoreNet.start()
	while sys_run == true do
		local ret, co = xpcall(function() return coroutine_nodule.co_create(worker) end, debug.traceback)
		if not ret then pcall(function() sys_print(string.format("coroutine_nodule.co_create %s", co)) end) end
		if co then coroutine_nodule.resume(co) end
	end
	sys_print(string.format("exit.... %s", sys_run))
end

-- 停止
function CCoreNet.exit()
	sys_run = false
end

-- 生成token
function CCoreNet.genToken(key, name)
	return CoreNetAgent.GenRegToken(key, name)
end

-- 打包
function CCoreNet.netPack(srcName, targetName, proto, PTYPE, session_id)
	session_id = session_id or 0
	local header = CoreNetAgent.NetPack(srcName, targetName, PTYPE, session_id, proto)
	return header
end

-- 解包
function CCoreNet.netUnPack(data)
	local srcName, targetName, PTYPE, session_id, proto, iDataIndex = CoreNetAgent.NetUnPack(data)
	return srcName, targetName, PTYPE, session_id, proto, string.sub(data, iDataIndex, -1)
end

-- 等待一个函数执行完毕
function CCoreNet.waitEvent(event_id, f, ...)
	return coroutine_nodule.wait_event(event_id, f, ...)
end

-- 事件数量
function CCoreNet.countWaitEvent(event_id)
	return coroutine_nodule.get_wait_event(event_id)
end

-- 存在事件
function CCoreNet.hasWaitEvent(event_id)
	return coroutine_nodule.has_wait_event(event_id)
end

-- 唤醒队列
function CCoreNet.wakeUpQueue(queue_id, bPopHead, queueNum)
	return coroutine_nodule.wakeup_queue(queue_id, bPopHead, queueNum)
end

-- 加入队列
function CCoreNet.waitQueue(queue_id, bUseTimer, timeout_millsec)
	return coroutine_nodule.wait_queue(queue_id, bUseTimer, timeout_millsec)
end

-- 队列数量
function CCoreNet.countQueue(queue_id)
	return coroutine_nodule.get_queue_count(queue_id)
end

-- 挂起
function CCoreNet.suspendTimeOut(timeout_millsec)
	return coroutine_nodule.wait_time_sleep(timeout_millsec)
end

-- 系统print函数钩子
local logLock = false
local serviceName = ""
local LogFunctionCallBack = nil

local function hook_print(...)
	
	local cacheArray = {}
	table.insert(cacheArray, "[" .. serviceName .. "]")
	for k,v in ipairs(table.pack(...)) do 
		table.insert(cacheArray, " ") 
		if type(v) == type("") then
			table.insert(cacheArray, v) 
		else
			table.insert(cacheArray, tostring(v)) 
		end
	end
	local cache = table.concat(cacheArray)

	if logLock then
		sys_print(cache)
		return
	end

	logLock = true
	local result, errMsg = xpcall(function() LogFunctionCallBack(cache) end, debug.traceback)
	if not result then sys_print(string.format("GlobalLogCallBack %s", errMsg)) end
	logLock = false
end

function CCoreNet.hookprint(nameStr, OnLogFunctionCallBack)

	if not sys_print then
		return
	end

	serviceName = nameStr
	LogFunctionCallBack = OnLogFunctionCallBack
	
	print = hook_print
end

-- 恢复系统print函数
function CCoreNet.undolprint()
	if not sys_print then
		return
	end
	print = sys_print
end

-- 取消系统print函数
function CCoreNet.cancelprint()
	if not sys_print then
		return
	end
	local hook_print = function(...) end
	print = hook_print
end

-- 获取统计数据
function CCoreNet.statReport(timeDelay)
	return CoreNet.Report(timeDelay)
end

-- 系统打印
function CCoreNet.sys_print(...)
	sys_print(...)
end

return util.ReadOnlyTable(CCoreNet)
