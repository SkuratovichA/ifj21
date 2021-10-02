require "ifj21"
--[[--------------- function declarations -----------------------]]
-- functions with no returns
global foo : function() 
global baz : function(string)
global bar : function(string, integer)
global arst : function(string, integer, number, number, integer, string)
global foo:function() 
global baz:function(string)
global bar:function(string, integer)
global arst:function(string, integer, number, number, integer, string)
---
global foo
:
function()    
---
global baz
:
function(string)
---
global bar
:
function(string, integer)
---
global arst
:
function(string,
         integer,
         number, 
         number, 
         integer,
         string
        )
------------------------------------------


-- functions with one return:
global foo : function() : string
global baz : function(string) : integer
global bar : function(string, integer) : number
global arst : function(string, integer, number) : number

--- functions with more returns:
global foo : function() : string
global baz : function(number) : integer, integer, integer, integer
global bar : function(string, integer, number) : number
global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string

global foo : function():string
global baz : function(number):integer, integer, integer, integer
global bar : function(string, integer, number):number
global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string

global bar : function(string,integer,number):number, integer
global bar : function(string,integer,number):number, number
global bar : function(string,integer,number):number, string
global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string

---
global foo : function() 
global foo : function()
global foo : function()
global foo : function() 

