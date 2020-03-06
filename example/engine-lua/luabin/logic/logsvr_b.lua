local AgentService = require "AgentService"
local net_module = require "ccorenet"
local timer = require "timer"
local util = require "util"

require "logic/loggate_event"

local loggate_event = "logic/loggate_event"

net_module.init("./test_svr_b.log", 65535, true)
timer.addtimer(loggate_event, "framefunc", 1000)

AgentService.Init(".watchdog", loggate_event, nil, "127.0.0.1", 6666, "sssssss", 60000, 10000, false)

net_module.start()

net_module.fin()