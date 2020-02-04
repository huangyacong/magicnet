﻿local msgpack = require "msgpack53"
local util = require "util"

local ccoroutine = {}

local running_thread = nil
local session_coroutine_id = {}
local session_id_coroutine = {} -- 需要做超时处理，协程回调就靠这个触发了
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
		coroutine_resume(co, f)
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
	if co then coroutine_resume(co, false, "time out") end
end

function ccoroutine.yield_call(net_modulename, sessionId)
	local retpcall, ret, data = pcall(function() 
		local timerId = net_modulename.addtimer(ccoroutine, "session_id_coroutine_timeout", 1000 * 20, sessionId)
		session_id_coroutine[sessionId] = running_thread
		local succ, msg = coroutine.yield("YIELD_CALL")
		session_id_coroutine[sessionId] = nil
		net_modulename.deltimer(timerId)
		return succ, (succ == true) and msgpack.unpack(msg) or msg
		end)
	session_id_coroutine[sessionId] = nil
	if not retpcall then 
		print(debug.traceback(), "\n", ret)
		return false, "traceback"
	end
	return ret, data
end

return util.ReadOnlyTable(ccoroutine)