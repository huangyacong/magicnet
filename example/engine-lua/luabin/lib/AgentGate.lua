local CoreNetAgent = require "CoreNetAgent"
local util = require "util"

local AgentGate = {}

function AgentGate.Init(cLogName, iMaxClientNum, bPrintLog2Screen)
	CoreNetAgent.InitAgentGate(cLogName, iMaxClientNum, 16, bPrintLog2Screen)
end

function AgentGate.AgentGateListenRemote(IPRemote, PortRemote, iTimeOut)
	return CoreNetAgent.AgentGateListenRemote(IPRemote, PortRemote, iTimeOut)
end

function AgentGate.AgentGateListenAgent(IPService, PortService, IPServiceUnix, iTimeOut)
	return CoreNetAgent.AgentGateListenAgent(IPService, PortService, IPServiceUnix, iTimeOut)
end

function AgentGate.Start()
	CoreNetAgent.StartAgentGate()
end

function AgentGate.Fin()
	CoreNetAgent.FinAgentGate()
end

return util.ReadOnlyTable(AgentGate)