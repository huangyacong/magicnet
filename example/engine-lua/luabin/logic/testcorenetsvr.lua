local util = require "util"
local timer = require "timer"
local IServerPlayer = require "IServerPlayer"
local ccorenet = require "ccorenet"
require "logic/testcorenetsvrevent/event"

local server_event = "logic/testcorenetsvrevent/event"

ccorenet.init("./test_svr.log", 65535, true)

local bReusePort = true
local domain = ccorenet.IpV4
local ip = (ccorenet.getOS() == "Linux" and domain == ccorenet.UnixLocal) and "dont.del.local.socket" or "127.0.0.1"
local serverObj = IServerPlayer.new("serverObj", server_event, ip, 8888, 1000*60, bReusePort, false)
ccorenet.addGlobalObj(serverObj, serverObj:GetName())
ccorenet.getGlobalObj("serverObj"):Listen()

ccorenet.start()

ccorenet.fin()