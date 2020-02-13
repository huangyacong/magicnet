local util = require "util"
local ccorenet = require "ccorenet"
local IClient = require "IClient"
local timer = require "timer"
require "logic/testcorenetclientevent/event"

ccorenet.init("./test_client.log", 65535, true)

local client_event = "logic/testcorenetclientevent/event"
timer.addtimer(client_event, "session_id_coroutine_timeout", 1000)

local domain = ccorenet.IpV4
local ip = (ccorenet.getOS() == "Linux" and domain == ccorenet.UnixLocal) and "dont.del.local.socket" or "127.0.0.1"
for i = 0, 0 do
	local name = string.format("clientObj%s", i)
local clientObj = IClient.IClientClass.new(name, client_event, ip, 8888, 1000*60, 5*1000, domain, true, false)
ccorenet.addGlobalObj(clientObj, clientObj:GetName())
end
--util.print_table(testcorenetclientevent)
for i = 0, 0 do
	local name = string.format("clientObj%s", i)
	ccorenet.getGlobalObj(name):Connect()
end
--util.print_table(ccorenet.IClientList)
ccorenet.start()

ccorenet.fin()