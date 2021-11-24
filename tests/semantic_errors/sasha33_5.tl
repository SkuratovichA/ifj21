-- test case 33.
-- Description : Function semantics: nil in declaration, but not in definition

-- Expected : '5'
require "ifj21" 
global foo : function( string, string ) : nil
function foo () : string                     
end                                          

