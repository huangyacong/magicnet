local timer_module = require "CCoreTimer"
local CoreTool = require "CoreTool"
local util = require "util"
require "class"

local local_modulename = ...
timer_module.register(local_modulename)

local table = table
local coroutine = coroutine
local running_thread = nil
local session_coroutine_id = {}
local session_id_coroutine = {} -- 需要做超时处理，协程回调就靠这个触发了
local wait_coroutine_event = {} -- 需要做超时处理，协程回调就靠这个触发了
local wait_coroutine_queue, wait_coroutine_queue_timerid = {}, {} -- 等待队列
local wait_coroutine_time_sleep = {}
local coroutine_pool = {}--setmetatable({}, { __mode = "kv" })

local function coroutine_resume(co, ...)
	running_thread = co
	return coroutine.resume(co, ...)
end

local function co_create(f)
	local co = table.remove(coroutine_pool)
	if co == nil then
		co = coroutine.create(function(...)
			f(...)
			while true do
				-- recycle co into pool
				f = nil
				coroutine_pool[#coroutine_pool+1] = co
				-- recv new main function f
				f = coroutine.yield "SUSPEND"
				f(coroutine.yield())
			end
		end)
	else
		local ret, err = coroutine.resume(co, f)
		if not ret then print(debug.traceback(), "\n", string.format("co_create %s", err)) end
	end
	return co
end

local CCoroutine = {}

function CCoroutine.add_session_coroutine_id(sessionId, srcName, proto)
	session_coroutine_id[running_thread] = {sessionId, tostring(srcName), tostring(proto)}
end

function CCoroutine.get_session_coroutine_id()
	local sessionData = session_coroutine_id[running_thread]
	--assert(sessionData, "get_session_coroutine_id failed")
	local sessionId, srcName, proto = table.unpack(sessionData)
	return sessionId, srcName, proto
end

function CCoroutine.del_session_coroutine_id()
	session_coroutine_id[running_thread] = nil
end

function CCoroutine.count_session_coroutine_id()
	local count = 0
	for k, v in pairs(session_coroutine_id) do count = count + 1 end
	return count
end

function CCoroutine.get_session_id_coroutine(sessionId)
	local co, msgExtended = table.unpack(session_id_coroutine[sessionId])
	return co, msgExtended
end

function CCoroutine.count_session_id_coroutine()
	local count = 0
	for k, v in pairs(session_id_coroutine) do count = count + 1 end
	return count
end

function CCoroutine.count_coroutine_pool()
	return #coroutine_pool
end

function CCoroutine.co_create(f)
	return co_create(f)
end

function CCoroutine.resume(co, ...)
	local running = running_thread
	local ret, err = coroutine_resume(co, ...)
	running_thread = running
	if not ret then xpcall(function() print("traceback error", "\n", string.format("CCoroutine.resume %s", err)) end, debug.traceback) end
	return ret, err
end

function CCoroutine.session_id_coroutine_timeout(sessionId, proto)
	local co, msgExtended = table.unpack(session_id_coroutine[sessionId])
	print(debug.traceback(), "\n", "CallData time out", sessionId, proto)
	if co then 
		CCoroutine.resume(co, false, "time out") 
	end
end

function CCoroutine.yield_call(sessionId, timeout_millsec, proto)
	timeout_millsec = timeout_millsec or (1000 * 20)
	local retpcall, ret, data = xpcall(function() 
		assert(session_id_coroutine[sessionId] == nil)
		local timerId = timer_module.addtimer(local_modulename, "session_id_coroutine_timeout", timeout_millsec, sessionId, proto)
		session_id_coroutine[sessionId] = {running_thread, {}}
		local succ, msg = coroutine.yield("YIELD_CALL")
		session_id_coroutine[sessionId] = nil
		timer_module.deltimer(timerId)
		return succ, msg
		end, debug.traceback)
	session_id_coroutine[sessionId] = nil
	if not retpcall then 
		print("traceback error", "\n", ret)
		return false, "traceback"
	end
	return ret, data
end

function CCoroutine.wait_time_sleep_timeout(sessionId)
	local co = wait_coroutine_time_sleep[sessionId]
	CCoroutine.resume(co, sessionId) 
end

function CCoroutine.wait_time_sleep(timeout_millsec)
	timeout_millsec = timeout_millsec or (1000 * 20)
	local retpcall, err = xpcall(function() 
		local sessionId = CoreTool.SysSessionId()
		assert(wait_coroutine_time_sleep[sessionId] == nil)
		local timerId = timer_module.addtimer(local_modulename, "wait_time_sleep_timeout", timeout_millsec, sessionId)
		wait_coroutine_time_sleep[sessionId] = running_thread
		local yield_sessionId = coroutine.yield("YIELD_CALL_TIME_SLEEP")
		assert(yield_sessionId == sessionId)
		wait_coroutine_time_sleep[sessionId] = nil
		end, debug.traceback)
	if not retpcall then 
		print("traceback error", "\n", err)
	end
end

local IWaitCoroutineEvent = class()
function IWaitCoroutineEvent:ctor(sessionId)
	self.sessionId = sessionId
	self.coroutineQueue = {}
end

function CCoroutine.wait_event_other_resume(event_id, sessionId, result)
	local IWaitCoroutineEventObj = wait_coroutine_event[event_id]
	assert(sessionId == IWaitCoroutineEventObj.sessionId)
	while next(IWaitCoroutineEventObj.coroutineQueue) do
		local co = IWaitCoroutineEventObj.coroutineQueue[1]
		CCoroutine.resume(co, sessionId, result)
	end
	if not next(IWaitCoroutineEventObj.coroutineQueue) then 
		wait_coroutine_event[event_id] = nil 
	end
end

function CCoroutine.wait_event(event_id, f, ...)
	local param = table.pack(...)
	event_id = tostring(event_id)
	local sessionId = CoreTool.SysSessionId()
	
	local retpcall, data = xpcall(function() 
		if not wait_coroutine_event[event_id] then
			wait_coroutine_event[event_id] = IWaitCoroutineEvent.new(sessionId)
			local resultFunction = f(table.unpack(param))
			return table.pack(resultFunction)
		end
		local IWaitCoroutineEventObj = wait_coroutine_event[event_id]
		table.insert(IWaitCoroutineEventObj.coroutineQueue, running_thread)
		local yield_sessionId, result = coroutine.yield("YIELD_CALL_WAIT_EVENT")
		assert(yield_sessionId == IWaitCoroutineEventObj.sessionId)
		assert(IWaitCoroutineEventObj.coroutineQueue[1] == running_thread)
		table.remove(IWaitCoroutineEventObj.coroutineQueue, 1)
		return result
		end, debug.traceback)

	-- 谁带头做，谁唤醒其它等待的协程
	local IWaitCoroutineEventObj = wait_coroutine_event[event_id]
	if IWaitCoroutineEventObj.sessionId == sessionId then
		if not next(IWaitCoroutineEventObj.coroutineQueue) then 
			wait_coroutine_event[event_id] = nil 
		else
			local result = nil
			if retpcall then result = data end
			local timerId = timer_module.addtimer(local_modulename, "wait_event_other_resume", 1, event_id, sessionId, result)
			if timerId == 0 then
				print("traceback error", "\n", string.format("CCoroutine.wait_event addtimer failed. event_id=%s", event_id))
			end
		end
	end

	if not retpcall then 
		print("traceback error", "\n", string.format("CCoroutine.wait_event event_id=%s", event_id), "\n", data)
		return nil
	end

	return table.unpack(data)
end

function CCoroutine.get_wait_event(event_id)
	local queueTable = wait_coroutine_event[queue_id]
	if not queueTable then return 0 end
	return #queueTable
end

function CCoroutine.has_wait_event(event_id)
	local queueTable = wait_coroutine_event[queue_id]
	if not queueTable then return false end
	return true
end

function CCoroutine.get_queue_count(queue_id)
	local queueTable = wait_coroutine_queue[queue_id]
	if not queueTable then return 0 end
	return #queueTable
end

function CCoroutine.wakeup_queue(queue_id, bPopHead, queueNum)
	if not wait_coroutine_queue[queue_id] then
		return
	end

	local doNum = 1
	if not queueNum or queueNum <= 0 then
		doNum = nil
	else
		doNum = queueNum
	end

	local count = 0
	while next(wait_coroutine_queue[queue_id]) do
		local co = wait_coroutine_queue[queue_id][bPopHead and 1 or (#wait_coroutine_queue[queue_id])]
		CCoroutine.resume(co, bPopHead and true or false) 
		count = count + 1

		if doNum ~= nil then
			doNum = doNum - 1
			if doNum <= 0 then
				break
			end
		end
	end

	if not next(wait_coroutine_queue[queue_id]) then
		wait_coroutine_queue[queue_id] = nil
		timer_module.deltimer(wait_coroutine_queue_timerid[queue_id] or 0)
		wait_coroutine_queue_timerid[queue_id] = nil
	end
end

function CCoroutine.wait_queue(queue_id, bUseTimer, timeout_millsec)
	if not wait_coroutine_queue[queue_id] then
		wait_coroutine_queue[queue_id] = {}
		if bUseTimer then
			wait_coroutine_queue_timerid[queue_id] = timer_module.addtimer(local_modulename, "wakeup_queue", timeout_millsec or 15 * 1000, queue_id, true)
		end
	end
	table.insert(wait_coroutine_queue[queue_id], running_thread)
	local bPopHead = coroutine.yield("YIELD_CALL_WAIT_QUEUE")
	assert(wait_coroutine_queue[queue_id][bPopHead and 1 or #wait_coroutine_queue[queue_id]] == running_thread)
	table.remove(wait_coroutine_queue[queue_id], bPopHead and 1 or nil)
end

return util.ReadOnlyTable(CCoroutine)
