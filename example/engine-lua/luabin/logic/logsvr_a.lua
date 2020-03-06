local AgentService = require "AgentService"
local net_module = require "ccorenet"
local util = require "util"
local timer = require "timer"
require "class"
require "logic/logsvr_event"

local logsvr_event = "logic/logsvr_event"

net_module.init("./test_svr_a.log", 65535, true)
timer.addtimer(logsvr_event, "framefunc", 1000)

AgentService.Init("svr_a", logsvr_event, nil, "127.0.0.1", 6666, "sssssss", 60000, 10000, false)

net_module.start()

net_module.fin()