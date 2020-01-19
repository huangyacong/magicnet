local util = require "util"
local ccorenet = require "ccorenet"
local testcorenetclientevent = require "testcorenetclientevent/event"

ccorenet.init("./test_client.log", 65535, 1, true)
ccorenet.setframefunc(testcorenetclientevent, "framefunc")

--util.print_table(testcorenetclientevent)
ccorenet.getGlobalObj("clientObj"):Connect()
--util.print_table(ccorenet.IClientList)
ccorenet.start()

ccorenet.fin()