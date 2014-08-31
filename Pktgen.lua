-- Create some short cuts to the real functions.
gsub		    = string.gsub
gmatch          = string.gmatch
strrep		    = string.rep
strsub          = string.sub
strfmt          = string.format
strmatch        = string.match
strrep          = string.rep
strfind         = string.find
strlen          = string.len
tolower         = string.lower
unlink          = os.remove
system          = os.execute
tinsert         = table.insert
tgetn           = table.getn
tconcat         = table.concat

-- ===========================================================================
-- getPwd - Get the current working directory path.
-- Works for both dos and cygwin shell, which may emit an error if one failes.
--
function getPwd( func )
	local s = syscmd("pwd", func);
	if ( s == nil ) then
		s = syscmd("sh pwd", func);
	end
	return s;
end

-- ===========================================================================
-- d2u - convert dos backslashes to unix forward slashes.
--
function d2u( str )
    return gsub(str or "", "\\", "/");
end

-- ===========================================================================
-- u2d - convert unix forward slashes to dos backslashes.
--
function u2d( str )
    return gsub(str or "", "/", "\\");
end

-- ===========================================================================
-- str2tbl - convert a string to a table.
--
function str2tbl( str )
    local t = {};
    local s;

    for s in gmatch(str or "", "(.-)\n") do
        tinsert(t, s);
    end
    return t
end

-- ===========================================================================
-- trims the string of beginning and trailing spaces and tabs.
--
function trim(txt) return gsub(txt, "%s*(.-)%s*$", "%1", 1); end

-- ===========================================================================
-- A formatted output routine just like the real printf.
--
function printf(...) io.write(strfmt(...)); io.flush(); end

-- ===========================================================================
-- returns the table size or number of items in table.
--
function getn( t )

    local   i = 0;

    if ( (t ~= nil) and (type(t) == "table") ) then
        i = tgetn(t);
        if ( i > 0 ) then
            return i;
        end
        for k in pairs(t) do
            i = i + 1;
        end
    end
    return i;
end

-- ===========================================================================
-- returns the 'basename' and the 'basedir' 
--
function basename(filename) 
    local   fn, dn;

    -- Convert the '\' to '/' in the path name.
    filename = d2u(filename);

    fn = gsub(filename, "(.*/)(.*)", "%2") or filename; 
    dn = gsub(filename, "(.*/)(.*)", "%1") 

    dn = strsub(dn, 1, -2);

    return fn, dn;
end

-- ===========================================================================
-- Default routine to read data from syscmd function.
--
local function __doRead(fn)
    local   data, f;

    -- Open and read all of the data.
    f = assert(io.open(fn));
    data = f:read("*all");
    f:close();

    unlink(fn);

    return data;
end

-- ===========================================================================
-- Execute the system command return the command output if needed.
--
function syscmd( cmd, funcPtr )
    local tmpFile = "syscmd_tmp";

    system( cmd .. " > " .. tmpFile );
 
    funcPtr = funcPtr or __doRead;

    return funcPtr(tmpFile);    -- tmpFile is removed by the function.
end

-- ===========================================================================
-- Execute the string and return true/false.
--
function isTrue(f)
    local   s;

    if ( f == nil ) then
        return 0;
    end

    s = "if ( "..f.." ) then return 1 else return 0 end";
    return assert(loadstring(s))();
end

-- ===========================================================================
-- Output a message and return.
--
function msg(m, ...)

    if ( m ~= nil ) then io.write("++ "..strfmt(m, ...)); io.flush(); end
end
-- ===========================================================================
-- Display an error message and exit.
--
function errmsg(m, ...)

    if ( m ~= nil ) then printf("** %s", strfmt(m, ...)); end

    os.exit(1);
end

-- ===========================================================================
-- Output a 'C' like block comment.
--
function cPrintf(m, ...)

    printf("/* ");
    io.write(strfmt(m, ...));
    printf(" */\n");
end
-- ===========================================================================
-- Output a 'C' like comment.
--
function comment(msg)

    printf("/* %s */\n", msg or "ooops");
end

-- Standard set of functions for normal operation.
--

-----------------------------------------------------------------------------
-- serializeIt - Convert a variable to text or its type of variable.
--
local function serializeIt(v)
    local   s;
    local   t = type(v);

    if (t == "number") then
        s = tostring(v);
    elseif (t == "table") then
        s = tostring(v);
    elseif (t == "string") then
        s = strfmt("%q", v);
    elseif (t == "boolean") then
        s = tostring(v);
    elseif (t == "function") then
        s = strfmt("()");
    elseif (t == "userdata") then
        s = tostring(v);
    elseif (t == "nil") then
        s = "nil";
    else
        s = strfmt("<%s>", tostring(v));
    end

    return s;
end

-----------------------------------------------------------------------------
-- Serialize a value
--    k - is the variable name string.
--    o - is the orignal variable name for tables.
--    v - the value of the variable.
--    saved - is the saved table to detect loops.
--    tab - is the current tab depth.
--
local function doSerialize(k, o, v, saved, tab)
	local s, t;
    local space = function(t) return strrep(" ", t); end;

    tab     = tab or 0;
    t       = type(v);
    saved   = saved or {};

	if (t == "table") then
        if ( saved[v] ~= nil ) then
            return strfmt("%s[%s] = %s,\n", space(tab), serializeIt(o), saved[v]); 
        else
            local   kk, vv, mt;

            saved[v] = k;

            if ( tab == 0 ) then
                s = strfmt("%s%s = {\n", space(tab), tostring(k)); 
            else
                s = strfmt("%s[%s] = {\n", space(tab), serializeIt(o)); 
            end
            for kk,vv in pairs(v) do
                local fn = strfmt("%s[%s]", tostring(k), serializeIt(kk));

                s = s .. doSerialize(fn, kk, vv, saved, tab+2);
            end

			if ( tab == 0 ) then
            	return s .. strfmt("%s}\n", space(tab));
			else
            	return s .. strfmt("%s},\n", space(tab));
			end
        end
	else
        return strfmt("%s[%s] = %s,\n", space(tab), serializeIt(o), serializeIt(v));
	end
end

-----------------------------------------------------------------------------
-- serialize - For a given key serialize the global variable.
--    k is a string for display and t is the table to display.
--    e.g. printf(serialize("foo", { ["bar"] = "foobar" } ));
--              
function serialize(k, t)

    if ( k == nil ) then
	    k = "Globals";
        t = _G;			-- Dump out globals
    end
	if ( t == nil ) then
		t = _G;
	end

   	return doSerialize(k, k, t, {}, 0);
end

function prints(k, t) io.write(serialize(k, t)); io.flush(); end
function sleep(t) pktgen.delay(t * 1000); end
