local util = require "util"
local timer = require "timer"
local IServer = require "IServer"
local ccorenet = require "ccorenet"
require "logic/testcorenetsvrevent/event"

ccorenet.init("./test_svr.log", 65535, true)

local server_event = "logic/testcorenetsvrevent/event"
timer.addtimer(server_event, "timerfunc", 1000, 1, 2, 3)

local bReusePort = true
local domain = ccorenet.IpV4
local ip = (ccorenet.getOS() == "Linux" and domain == ccorenet.UnixLocal) and "dont.del.local.socket" or "127.0.0.1"
local serverObj = IServer.new("serverObj", server_event, ip, 8888, 1000*60, domain, bReusePort, false)
ccorenet.addGlobalObj(serverObj, serverObj:GetName())
ccorenet.getGlobalObj("serverObj"):Listen()

ccorenet.start()

ccorenet.fin()