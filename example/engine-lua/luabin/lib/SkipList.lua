
--[[
    根据redis里的skipList实现的跳表模块, 用于排行榜的数据结构。
    
    例子：

    local tbRankBoard = skiplist:new()  -- 如果需要自定义排序比较, 需要传入比较方法, 默认方法是本文件内的defaultComFunc。
                                           比较方法中value相同return 0, 排前面排前面return 1, 否则 -1。
                                           相同value会默认按照插入榜中的时间顺序排序。

    tbRankBoard:add("key", 1)        -- 写入数据, value可以是结构体
    tbRankBoard:getAll(true)         -- 获取所有排名数据, 参数字段的作用是是否返回value, 传true时返回一个{{key = key, value = value}}结构的数组, 不传或传false时仅返回由key组成的数组
    tbRankBoard:getByKey("key")      -- 通过key获取对应的value
    tbRankBoard:getByRank(1)         -- 获取对应名次的key和value
    tbRankBoard:rank("key")          -- 获取key对应的rank排名
    tbRankBoard:rem("key")           -- 删除元素
]]

-- math.randomseed(os.time())
local SKIPLIST_MAXLEVEL = 32                    -- 最大32层
local SKIPLIST_P        = 0.25                  -- redis源码里是0.25, 跟着写的
--  
local skipList_meta     = {}  
skipList_meta.__index   = skipList_meta  
--  
local zset_meta         = {}  
zset_meta.__index       = zset_meta  
local zset              = setmetatable({}, zset_meta)  
--------------------------------------------------------------  
  
local function randomLevel()  
    local level = 1  
    while(math.random(1, 0xffff) < SKIPLIST_P * 0xffff) do  
        level = level + 1  
    end  
    return level < SKIPLIST_MAXLEVEL and level or SKIPLIST_MAXLEVEL  
end
  
local function createSkipListNode(level, key, value)  
    local sln = { key = key, value = value, level = {}, backward = nil}  
    for lv = 1, level do  
        table.insert(sln.level, {forward = nil, span = 0})  
    end  
    return sln  
end  
  
local function createSkipList(cmpFn)  
    assert(type(cmpFn) == "function")  
    return setmetatable({  
        header     = createSkipListNode(SKIPLIST_MAXLEVEL),  
        tail       = nil,  
        length     = 0,  
        level      = 1,  
        compareFn  = cmpFn,  
    }, skipList_meta)  
end  
  
---------------------------skipList---------------------------  
  
--[[
    @desc:          数据插入
    --@key:
	--@value: 
    @return:
]]
function skipList_meta:insert(key, value) 
    assert(type(value) == "table")
    local update = {}  
    local rank   = {}  
    local x      = self.header  
    local level  
    for i = self.level, 1, -1 do   
        -- 找到所有level中的节点位置
        rank[i] = i == self.level and 0 or rank[i+1]  
        while x.level[i].forward and self.compareFn(x.level[i].forward.value, value) >= 0 do  -- 从大到小排
            rank[i] = rank[i] + x.level[i].span  
            x = x.level[i].forward  
        end  
        update[i] = x  -- 第一个大于value的node
    end

    -- 允许修改分数, 所以在insert前需要判断key值是否已经在objs中。这里就不判断了, 默认当作key值不存在做插入处理。
    level = randomLevel()
    if level > self.level then  
        -- level = self.level + 1 -- 如在插入一个新元素前有三条链，而在插入之后就有了10条链。这是，新插入的元素为9级，尽管前面并没有出现3到8级元素。
        for i = self.level + 1, level do  
            rank[i] = 0  
            update[i] = self.header  
            update[i].level[i].span = self.length  
        end  
        self.level = level  
    end  
    x = createSkipListNode(level, key, value)  
    for i = 1, level do  
        x.level[i].forward = update[i].level[i].forward  
        update[i].level[i].forward = x  
          
        x.level[i].span = update[i].level[i].span - (rank[1] - rank[i])  
        update[i].level[i].span = (rank[1] - rank[i]) + 1  
    end  
      
    for i = level + 1, self.level do  
        update[i].level[i].span = update[i].level[i].span + 1  
    end  
      
    x.backward = update[1] ~= self.header and update[1]  
    if x.level[1].forward then  
        x.level[1].forward.backward = x  
    else  
        self.tail = x  
    end  
    self.length = self.length + 1  
end  
  
function skipList_meta:getRank(key, value)  
    local rank = 0  
    local x  
    x = self.header  
    for i = self.level, 1, -1 do  
        while x.level[i].forward and (self.compareFn(x.level[i].forward.value, value) > 0 or (self.compareFn(x.level[i].forward.value, value) == 0 and x.level[i].forward.key ~= key)) do  
            rank = rank + x.level[i].span  
            x = x.level[i].forward  
        end   
        if x.level[i].forward and x.level[i].forward.value and x.level[i].forward.key == key and x.level[i].forward.value == value then  
            rank = rank + x.level[i].span  
            return rank  
        end  
    end
    return 0  
end  
  
--[[
    @desc:          获取对应排名的元素
    --@rank: 
    @return:
]]
function skipList_meta:getNodeByRank(rank)  
    if rank <= 0 or rank > self.length then 
        return 
    end  
    local traversed = 0  
    local x = self.header  
    for i = self.level, 1, -1 do  
        while x.level[i].forward and traversed + x.level[i].span <= rank do  
            traversed = traversed + x.level[i].span  
            x = x.level[i].forward  
        end  
        if traversed == rank then  
            return x  
        end  
    end  
end  
  
--[[
    @desc:          delete使用的内部函数
    --@node:
	--@update: 
    @return:
]]
function skipList_meta:deleteNode(node, update)  
    for i = 1, self.level do  
        if update[i].level[i].forward == node then  
            update[i].level[i].span = update[i].level[i].span + node.level[i].span - 1  
            update[i].level[i].forward = node.level[i].forward  
        else  
            update[i].level[i].span = update[i].level[i].span - 1  
        end  
    end  
    if node.level[1].forward then  
        node.level[1].forward.backward = node.backward  
    else  
        self.tail = node.backward  
    end  
    while self.level > 2 and not self.header.level[self.level -1].forward do  
        self.level = self.level - 1  
    end  
    self.length = self.length - 1  
end  
  
--[[
    @desc:          删除
    --@key:
	--@value: 
    @return:
]]  
function skipList_meta:delete(key, value)  
    local update = {}  
    local x = self.header
    for i = self.level, 1, -1 do 
        while x.level[i].forward and (self.compareFn(x.level[i].forward.value, value) > 0 or (self.compareFn(x.level[i].forward.value, value) == 0 and x.level[i].forward.key ~= key)) do  
            x = x.level[i].forward  
        end 
        update[i] = x  
    end  
    -- 确保对象的正确
    x = x.level[1].forward  
    if x and x.key == key and x.value == value then   
        self:deleteNode(x, update)  
        return true  
    end  
    return false  
end  
  
function skipList_meta:Range(start, stop)  
    local node = self:getNodeByRank(start)  
    local result = {}  
    local len = stop - start + 1  
    local n = 0  
    while node and n < len do  
        n = n + 1  
        result[#result+1] = node.value
        node = node.level[1].forward
    end  
    return result  
end  
  
function skipList_meta:get_count()  
    return self.length  
end  
  

function skipList_meta:print()  
    local x = self.header
    local i = 0  
    while x.level[1].forward do  
        x = x.level[1].forward
        i = i + 1
        print("rank ".. i .."- key ".. x.key  .. "- value " .. x.value.RankValue) 
    end  
end  

----------------------------zset-----------------------------  
  
--[[
    @desc:          默认的比较函数。
    --@a:
	--@b: 
    @return:
]]
local function defaultComFunc(a, b)
    if a.RankValue == b.RankValue then
        return 0
    end

    return a.RankValue > b.RankValue and 1 or -1
end
--[[
    @desc:          构造
    --@comFunc:     比较函数
    @return:
]]
function zset_meta:new(comFunc)
    if not comFunc or type(comFunc) ~= "function" then
        comFunc = defaultComFunc
    end
    return setmetatable({  
        sl   = createSkipList(comFunc),  
        objs = {},  
    }, zset_meta) 
end

--[[
    @desc:          value可以是自定义结构, 自己定义好比较结构就行
    --@key:
	--@value: 
    @return:
]]
function zset_meta:add(key, value)  
    if not key or not value then
        return 
    end
    local old = self.objs[key]  
    if old then
        if old.RankValue == value.RankValue then
            return
        end  
        self.sl:delete(key, old)  
    end  
    self.sl:insert(key, value)  
    self.objs[key] = value  
end  
  
--[[
    @desc:          通过key值获取对应value
    --@key: 
    @return:
]]
function zset_meta:getByKey(key)  
    return self.objs[key]  
end  
  

--[[
    @desc:          通过排名获取对应值
    --@rank: 
    @return:
]]
function zset_meta:getByRank(rank)  
    local node = self.sl:getNodeByRank(rank)
    if not node then
        return 
    end
    return node.key, node.value
end  
  
--[[
    @desc:          通过key值获取对应排名
    --@key: 
    @return:
]]
function zset_meta:rank(key)  
    local value = self.objs[key]  
    if not value then 
        return 0, {}
    end
    local rank = self.sl:getRank(key, value)
    if rank > 0 then
        return rank, value
    else
        return 0, {}
    end
end  
  
--[[
    @desc:          获取排名数量
    @return:
]] 
function zset_meta:count()  
    return self.sl:get_count()  
end  
  
--[[
    @desc:          获取排行区间
    --@start:       开始排名
	--@stop:        结束排名
	--@withValue:   返回值是否包括value
    @return:
        res = {
            {
                key = key1,
                value = value1,
            },
            {
                key = key2,
                value = value2
            }
            ...
        }
]]
function zset_meta:range(start, stop)  
    local count = self.sl:get_count()  
    --  
    if start < 0 then 
        start = count + start + 1 
    end  
    if start < 1 then 
        start = 1 
    end  
    --  
    if stop == nil then 
        stop = count 
    end  
    if stop < 0 then 
        stop = count + stop + 1 
    end  
    if stop < 1 then stop = 1 end  
    --  
    if start > stop then  
        return {}  
    end  

    return self.sl:Range(start, stop)
end  
  
--[[
    @desc:          根据key删除元素
    --@key: 
    @return:
]]
function zset_meta:rem(key)  
    local old = self.objs[key]  
    if old then  
        self.sl:delete(key, old)  
        self.objs[key] = nil  
    end  
end  


--[[
    @desc:          删除所有元素
    @return:
]]
function zset_meta:remAll()
    for _, v in pairs(self:getAll()) do
        self:rem(v)
    end
end

--[[
    @desc:          获取全部数据
    @return:
]]
function zset_meta:getAll()  
    return self:range(1, self:count()) 
end 
  
--[[
    @desc:          从头开始打印所有数据
    @return:
]]
function zset_meta:print()  
    self.sl:print()  
end 
  
return zset  
