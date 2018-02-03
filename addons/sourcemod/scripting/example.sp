#include <sourcemod>
#include <customkeyvalues>

public void OnPluginStart()
{
    RegConsoleCmd( "sm_test", Command_Test );
}

/*
worldspawn in bsp entity lump:
		"world_maxs" "0 1536 576"
		"world_mins" "-2048 -512 64"
		"skyname" "sky_dust"
		"mod_tier" "3"
		"mod_creator" "SlidyBat"
		"mod_bestplayerinworld" "Fozz"
		"maxpropscreenwidth" "-1"
		"detailvbsp" "detail.vbsp"
		"detailmaterial" "detail/detailsprites"
		"classname" "worldspawn"
		"mapversion" "5"
		"hammerid" "1"
*/

public Action Command_Test( int client, int args )
{
	char tier[8];
	bool success = GetCustomKeyValue( 0, "mod_tier", tier, sizeof( tier ) );
	PrintToServer( "Map tier: %i (%s)", StringToInt( tier ), success ? "success" : "failure" );
	
	// output: "Map tier: 3 (success)"
	
	return Plugin_Handled;
}