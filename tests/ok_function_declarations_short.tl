require "ifj21"
--[[--------------- function declarations -----------------------]]
functions with no returns
global foo : function()
global baz : function(string)
global bar : function(string, integer)

global foo:function()
---
global baz
:
function(string)
------------------------------------------

-- functions with one return:
global foo : function() : string
global bar : function(string, integer) : number

--- functions with more returns:
global foo : function() : string
global bar : function(string, integer, number) : number
global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string
-- 
global foo : function():string
global bar : function(string, integer, number):number
--
global bar : function(string,integer,number):number, string
 global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string,string,string
-- 
-- ---
global foo : function()

