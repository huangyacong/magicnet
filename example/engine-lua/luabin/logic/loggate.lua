local AgentGate = require "AgentGate"
local CoreTool = require "CoreTool"
local util = require "util"
local timer = require "timer"

AgentGate.Init("./test_svr_gate.log", 65535, true)

AgentGate.Listen("127.0.0.1", 8888, "127.0.0.1", 6666, "sssssss", 60000)

AgentGate.Start()

AgentGate.Fin()
