local CoreTool = require "CoreTool"
local timer = require "timer"
local util = require "util"

local local_modulename = ...
timer.register(local_modulename)

local ccoroutine = {}

local running_thread = nil
local session_coroutine_id = {}
local session_id_coroutine = {} -- 需要做超时处理，协程回调就靠这个触发了
local wait_coroutine_doing_thread = {}
local wait_coroutine_event = {} -- 需要做超时处理，协程回调就靠这个触发了
local wait_coroutine_queue = {} -- 等待队列
local wait_coroutine_time_sleep = {}
local coroutine_pool = setmetatable({}, { __mode = "kv" })

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

function ccoroutine.add_session_coroutine_id(sessionId, srcName, proto)
	session_coroutine_id[running_thread] = {sessionId, tostring(srcName), tostring(proto)}
end

function ccoroutine.get_session_coroutine_id()
	local sessionData = session_coroutine_id[running_thread]
	assert(sessionData, "get_session_coroutine_id failed")
	local sessionId, srcName, proto = table.unpack(sessionData)
	return sessionId, srcName, proto
end

function ccoroutine.del_session_coroutine_id()
	session_coroutine_id[running_thread] = nil
end

function ccoroutine.count_session_coroutine_id()
	local count = 0
	for k, v in pairs(session_coroutine_id) do count = count + 1 end
	return count
end

function ccoroutine.get_session_id_coroutine(sessionId)
	return session_id_coroutine[sessionId]
end

function ccoroutine.count_session_id_coroutine()
	local count = 0
	for k, v in pairs(session_id_coroutine) do count = count + 1 end
	return count
end

function ccoroutine.count_coroutine_pool()
	return #coroutine_pool
end

function ccoroutine.co_create(f)
	return co_create(f)
end

function ccoroutine.resume(co, ...)
	local running = running_thread
	local ret, err = coroutine_resume(co, ...)
	running_thread = running
	if not ret then pcall(function() print(debug.traceback(), "\n", string.format("ccoroutine.resume %s", err)) end) end
	return ret, err
end

function ccoroutine.session_id_coroutine_timeout(sessionId)
	local co = session_id_coroutine[sessionId]
	print(debug.traceback(), "\n", "CallData time out", sessionId)
	if co then 
		ccoroutine.resume(co, false, "time out") 
	end
end

function ccoroutine.yield_call(sessionId, timeout_millsec)
	timeout_millsec = timeout_millsec or (1000 * 20)
	local retpcall, ret, data = pcall(function() 
		assert(session_id_coroutine[sessionId] == nil)
		local timerId = timer.addtimer(local_modulename, "session_id_coroutine_timeout", timeout_millsec, sessionId)
		session_id_coroutine[sessionId] = running_thread
		local succ, msg = coroutine.yield("YIELD_CALL")
		session_id_coroutine[sessionId] = nil
		timer.deltimer(timerId)
		return succ, msg
		end)
	session_id_coroutine[sessionId] = nil
	if not retpcall then 
		print(debug.traceback(), "\n", ret)
		return false, "traceback"
	end
	return ret, data
end

function ccoroutine.wait_time_sleep_timeout(sessionId)
	local co = wait_coroutine_time_sleep[sessionId]
	ccoroutine.resume(co, sessionId) 
end

function ccoroutine.wait_time_sleep(timeout_millsec)
	timeout_millsec = timeout_millsec or (1000 * 20)
	local retpcall, err = pcall(function() 
		local sessionId = CoreTool.SysSessionId()
		assert(wait_coroutine_time_sleep[sessionId] == nil)
		local timerId = timer.addtimer(local_modulename, "wait_time_sleep_timeout", timeout_millsec, sessionId)
		wait_coroutine_time_sleep[sessionId] = running_thread
		local yield_sessionId = coroutine.yield("YIELD_CALL_TIME_SLEEP")
		assert(yield_sessionId == sessionId)
		wait_coroutine_time_sleep[sessionId] = nil
		end)
	if not retpcall then 
		print(debug.traceback(), "\n", err)
	end
end

function ccoroutine.wait_event_other_resume(event_id, sessionId, result)
	while next(wait_coroutine_event[event_id][sessionId]) do
		local co = wait_coroutine_event[event_id][sessionId][1]
		ccoroutine.resume(co, sessionId, result)
	end
	wait_coroutine_event[event_id][sessionId] = nil
end

function ccoroutine.wait_event(event_id, f, ...)
	local param = table.pack(...)
	assert(type(event_id) == type(""))
	local retpcall, data = pcall(function() 
		if not wait_coroutine_event[event_id] then
			wait_coroutine_event[event_id] = {}
		end
		if not wait_coroutine_doing_thread[event_id] then
			local sessionId = CoreTool.SysSessionId()
			assert(not wait_coroutine_event[event_id][sessionId])
			wait_coroutine_doing_thread[event_id] = {sessionId, running_thread}
			wait_coroutine_event[event_id][sessionId] = {}
			return table.pack(f(table.unpack(param)))
		end
		local doing_sessionId, doing_thread = table.unpack(wait_coroutine_doing_thread[event_id])
		assert(running_thread ~= doing_thread, string.format("wait_event=%s is dead loop!", event_id))
		table.insert(wait_coroutine_event[event_id][doing_sessionId], running_thread)
		local yield_sessionId, result = coroutine.yield("YIELD_CALL_WAIT_EVENT")
		assert(yield_sessionId == doing_sessionId)
		assert(wait_coroutine_event[event_id][yield_sessionId][1] == running_thread)
		table.remove(wait_coroutine_event[event_id][yield_sessionId], 1)
		return result
		end)

	if wait_coroutine_doing_thread[event_id] then
		-- 谁带头做，谁唤醒其它等待的协程
		local doing_sessionId, doing_thread = table.unpack(wait_coroutine_doing_thread[event_id])
		if doing_thread == running_thread then
			local result = nil
			if retpcall then result = data end
			local timerId = timer.addtimer(local_modulename, "wait_event_other_resume", 1, event_id, doing_sessionId, result)
			if timerId == 0 then
				print(debug.traceback(), "\n", string.format("ccoroutine.wait_event addtimer failed. event_id=%s", event_id))
			end
			wait_coroutine_doing_thread[event_id] = nil
		end
	end

	if not retpcall then 
		print(debug.traceback(), "\n", string.format("ccoroutine.wait_event event_id=%s", event_id), "\n", data)
		return nil
	end
	return table.unpack(data)
end

function ccoroutine.wakeup_queue(queue_id, bPopHead, queueNum)
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
		ccoroutine.resume(co, bPopHead and true or false) 
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
	end
end

function ccoroutine.wait_queue(queue_id)
	if not wait_coroutine_queue[queue_id] then
		wait_coroutine_queue[queue_id] = {}
	end
	table.insert(wait_coroutine_queue[queue_id], running_thread)
	local bPopHead = coroutine.yield("YIELD_CALL_WAIT_QUEUE")
	assert(wait_coroutine_queue[queue_id][bPopHead and 1 or #wait_coroutine_queue[queue_id]] == running_thread)
	table.remove(wait_coroutine_queue[queue_id], bPopHead and 1 or nil)
end

return util.ReadOnlyTable(ccoroutine)
