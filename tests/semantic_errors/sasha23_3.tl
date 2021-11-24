-- test case 23.
-- Description : semantic: more than one declaration (error)

-- Expected : '3'
require "ifj21" 
function me() : stringend function me() : integerend function me() : booleanend 
