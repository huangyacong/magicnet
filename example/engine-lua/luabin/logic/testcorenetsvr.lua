local util = require "util"
local ccorenet = require "ccorenet"
local testcorenetsvrevent = require "testcorenetsvrevent/event"

ccorenet.init("./test_svr.log", 65535, 1000, true)
ccorenet.setframefunc(testcorenetsvrevent, "framefunc")

--util.print_table(testcorenetsvrevent)
ccorenet.getGlobalObj("serverObj"):Listen()
ccorenet.start()

ccorenet.fin()