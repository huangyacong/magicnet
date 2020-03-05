local net_module = require "ccorenet"

local AgentGate = {}

function AgentGate.Init(cLogName, iMaxClientNum, bPrintLog2Screen)
	net_module.InitAgentGate(cLogName, iMaxClientNum, 16, bPrintLog2Screen)
end

function AgentGate.Listen(IPRemote, PortRemote, IPService, PortService, IPServiceUnix, iTimeOut)
	return net_module.AgentGateListen(IPRemote, PortRemote, IPService, PortService, IPServiceUnix, iTimeOut)
end

function AgentGate.Start()
	return net_module.StartAgentGate()
end

function AgentGate.Fin()
	net_module.FinAgentGate()
end

return util.ReadOnlyTable(AgentGate)