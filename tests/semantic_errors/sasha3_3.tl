-- test case 3.
-- Description : Redeclaration error

-- Expected : '3'
require "ifj21" 
 global foo : function() global baz : function(string) global bar : function(string, integer) global arst : function(string, integer, number, number, integer, string) global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string
 global foo:function() global baz:function(string) global bar:function(string, integer) global arst:function(string, integer, number, number, integer, string) global foo : function() : string
 global baz : function(string) : integer
 global bar : function(string, integer) : number
 global arst : function(string, integer, number) : number
 global foo : function() : string
 global baz : function(number) : integer, integer, integer, integer
 global bar : function(string, integer, number) : number
 global foo : function():string
 global baz : function(number):integer, integer, integer, integer
 global bar : function(string, integer, number):number
 global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string
 global bar : function(string,integer,number):number, integer
 global bar : function(string,integer,number):number, number
 global bar : function(string,integer,number):number, string
 global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string
 global foo : function()
 global foo : function()
 global foo : function()

