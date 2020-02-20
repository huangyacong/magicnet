local timer = require "timer"
local util = require "util"

local local_modulename = ...
timer.register(local_modulename)

local ccoroutine = {}

local running_thread = nil
local session_coroutine_id = {}
local session_id_coroutine = {} -- 需要做超时处理，协程回调就靠这个触发了
local wait_coroutine_first_event = nil
local wait_coroutine_event = {} -- 需要做超时处理，协程回调就靠这个触发了
local suspend_coroutine_event = {} -- 挂起队列
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
				-- 执行完毕，删除会话
				session_coroutine_id[co] = nil
				-- recycle co into pool
				f = nil
				coroutine_pool[#coroutine_pool+1] = co
				-- recv new main function f
				f = coroutine.yield "SUSPEND"
				f(coroutine.yield())
			end
		end)
	else
		local running = running_thread
		local ret, err = coroutine_resume(co, f)
		if not ret then print(debug.traceback(), "\n", string.format("co_create %s", err)) end
		running_thread = running
	end
	return co
end

function ccoroutine.add_session_coroutine_id(sessionId)
	session_coroutine_id[running_thread] = sessionId
	return running_thread, sessionId
end

function ccoroutine.get_session_coroutine_id()
	return session_coroutine_id[running_thread]
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
	return coroutine_resume(co, ...)
end

function ccoroutine.session_id_coroutine_timeout(sessionId)
	local co = session_id_coroutine[sessionId]
	print(debug.traceback(), "\n", "CallData time out")
	if co then 
		local ret, err = coroutine_resume(co, false, "time out") 
		if not ret then print(debug.traceback(), "\n", string.format("session_id_coroutine_timeout %s", err)) end
	end
end

function ccoroutine.yield_call(sessionId, timeout_millsec)
	timeout_millsec = timeout_millsec or 1000 * 20
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

function ccoroutine.wait_event_other_resume(event_id, result)
	if not wait_coroutine_event[event_id] then
		return
	end
	if not next(wait_coroutine_event[event_id]) then
		return
	end
	
	local sortData = {}
	util.copytable(wait_coroutine_event[event_id])
	for sessionId, _ in pairs(wait_coroutine_event[event_id]) do
		sortData[#sortData + 1] = sessionId
	end
	table.sort(sortData)-- 从小到大

	for _, sessionId in ipairs(sortData) do
		local co = wait_coroutine_event[event_id][sessionId]
		if co then 
			local ret, err = coroutine_resume(co, result) 
			if not ret then print(debug.traceback(), "\n", string.format("wait_event_other_resume %s", err)) end
		end
	end
end

function ccoroutine.wait_event(event_id, sessionId, f, ...)
	local param = table.pack(...)
	assert(type(event_id) == type(""))
	local retpcall, data = pcall(function() 
		if not wait_coroutine_first_event then
			wait_coroutine_first_event = running_thread
			wait_coroutine_event[event_id] = {}
			return table.pack(f(table.unpack(param)))
		end
		assert(wait_coroutine_event[event_id][sessionId] == nil)
		wait_coroutine_event[event_id][sessionId] = running_thread
		local result = coroutine.yield("YIELD_CALL_WAIT_EVENT")
		wait_coroutine_event[event_id][sessionId] = nil
		return result
		end)

	-- 第一个进入者唤醒其它等待的协程
	if wait_coroutine_first_event == running_thread then
		wait_coroutine_first_event = nil
		if next(wait_coroutine_event[event_id]) then 
			local result = nil
			if retpcall then result = data end
			local timerId = timer.addtimer(local_modulename, "wait_event_other_resume", 1, event_id, result)
			if timerId == 0 then
				print(debug.traceback(), "\n", string.format("ccoroutine.wait_event addtimer failed. event_id=%s", event_id))
			end
		end
	end

	if not retpcall then 
		print(debug.traceback(), "\n", string.format("ccoroutine.wait_event event_id=%s", event_id), "\n", data)
		return nil
	end
	return table.unpack(data)
end

function ccoroutine.suspend_event_time_out(event_id)
	assert(type(event_id) == type(""))
	local co = suspend_coroutine_event[event_id]
	if co then 
		local ret, err = coroutine_resume(co) 
		if not ret then print(debug.traceback(), "\n", err) end
	end
end

function ccoroutine.wakeup_event(event_id)
	return ccoroutine.suspend_event_time_out(event_id)
end

function ccoroutine.suspend_event(event_id, timeout_millsec)
	assert(type(event_id) == type(""))
	timeout_millsec = timeout_millsec or 1000 * 20
	assert(suspend_coroutine_event[event_id] == nil)
	local timerId = timer.addtimer(local_modulename, "suspend_event_time_out", timeout_millsec, event_id)
	suspend_coroutine_event[event_id] = running_thread
	local succ, msg = coroutine.yield("YIELD_CALL_SUSPEND")
	suspend_coroutine_event[event_id] = nil
	timer.deltimer(timerId)
end

return util.ReadOnlyTable(ccoroutine)
