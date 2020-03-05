local CoreNetAgent = require "CoreNetAgent"
local util = require "util"

local AgentGate = {}

function AgentGate.Init(cLogName, iMaxClientNum, bPrintLog2Screen)
	CoreNetAgent.InitAgentGate(cLogName, iMaxClientNum, 16, bPrintLog2Screen)
end

function AgentGate.Listen(IPRemote, PortRemote, IPService, PortService, IPServiceUnix, iTimeOut)
	return CoreNetAgent.AgentGateListen(IPRemote, PortRemote, IPService, PortService, IPServiceUnix, iTimeOut)
end

function AgentGate.Start()
	return CoreNetAgent.StartAgentGate()
end

function AgentGate.Fin()
	CoreNetAgent.FinAgentGate()
end

return util.ReadOnlyTable(AgentGate)