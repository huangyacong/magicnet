local CoreNet = require "CoreNet"
local util = require "util"

local AgentGate = {}

function AgentGate.Init(cLogName, iMaxClientNum, bPrintLog2Screen)
	CoreNet.InitAgentGate(cLogName, iMaxClientNum, 16, bPrintLog2Screen)
end

function AgentGate.Listen(IPRemote, PortRemote, IPService, PortService, IPServiceUnix, iTimeOut)
	return CoreNet.AgentGateListen(IPRemote, PortRemote, IPService, PortService, IPServiceUnix, iTimeOut)
end

function AgentGate.Start()
	return CoreNet.StartAgentGate()
end

function AgentGate.Fin()
	CoreNet.FinAgentGate()
end

return util.ReadOnlyTable(AgentGate)