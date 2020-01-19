local ccoremysql = require "ccoremysql"
local util = require "util"


util.print_table(ccoremysql)

ccoremysql.loadmysqldll()

ccoremysql.init("127.0.0.1", 3306, "test", "root", "123456", "utf8")
ccoremysql.tryconect()



--ccoremysql.modifysql("insert into aa(aa) values(1)")


print("")
print("")
print("")
print("")

util.print_table(ccoremysql.selectsql("select * from aa"))

print("")
print("")
print("")
print("")

print(ccoremysql.escapesql("sdasd 幅度是否收到 \n"))

ccoremysql.disconnect()
ccoremysql.downmysqldll()




	


	


	
