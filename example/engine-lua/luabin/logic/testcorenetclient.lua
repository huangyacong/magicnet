local util = require "util"
local ccorenet = require "ccorenet"
local testcorenetclientevent = require "testcorenetclientevent/event"

ccorenet.init("./test_client.log", 65535, true)

--util.print_table(testcorenetclientevent)
for i = 0, 5000 do
	local name = string.format("clientObj%s", i)
	ccorenet.getGlobalObj(name):Connect()
end
--util.print_table(ccorenet.IClientList)
ccorenet.start()

ccorenet.fin()