local AgentGate = require "AgentGate"
local net_module = require "ccorenet"
local CoreTool = require "CoreTool"
local ccoroutine = require "ccoroutine"
local util = require "util"
local timer = require "timer"
require "logic/loggate_event"

net_module.init("./test_svr.log", 65535, true)

local loggate_event = "logic/loggate_event"
timer.addtimer(loggate_event, "framefunc", 1000)

AgentGate.Init("gate", loggate_event, nil, "127.0.0.1", 8888, 60000, "127.0.0.1", 6666, "sssssss", 60000, false)

net_module.start()

net_module.fin()
