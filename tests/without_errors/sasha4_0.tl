-- test case 4.
-- Description : no error, parsing definitions, declarations

-- Expected : '0'
require "ifj21" 
global foo : function()
function foo()         
end                    
global baz : function(string)
function baz(str : string)   
end                          
global bar : function(string, integer)     
function bar(str : string, int : integer)  
end                                        
global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string
  
function aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa (str : string) : string, string, string
    
end                                                                                                
 global arst : function(string,         integer,             number,       number,     integer, string)             
function arst(str : string, ddd : integer, nummm : number, aaa : number, ii: integer, suka :string) 
end                                                                                                                 
foo() bar("hello", 10) baz("hello") arst("", 10, 10.0, 10.10, 10, "") 
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa("arstioen")                    

