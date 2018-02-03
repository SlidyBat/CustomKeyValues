#if !defined _CUSTOMKEYVALUES_INC_
#define _CUSTOMKEYVALUES_INC_

native bool GetCustomKeyValue( int entity, const char[] key, char[] value, int maxlen );
native void SetCustomKeyValue( int entity, const char[] key, const char[] value );

#if !defined REQUIRE_EXTENSIONS
public __ext_customkeyvalues_SetNTVOptional()
{
    MarkNativeAsOptional("GetCustomKeyValue");
}
#endif  

public Extension __ext_customkeyvalues = 
{
	name = "Custom KeyValues",
	file = "customkeyvalues.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};

#endif