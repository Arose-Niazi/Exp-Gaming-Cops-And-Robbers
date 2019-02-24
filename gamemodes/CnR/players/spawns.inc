/*

	Spawns Script
	Scripted by Arose Niazi.
	Scripting started on 20th October 2017
	Last Updated: -----
	
	Credits:
		Fawkes for LS Civilian Spawns
		
	Functions:
		
		
	Functions Being Called:
		
*/
enum SpawnsEnum 
{
	Float:SpawnLocX,
	Float:SpawnLocY,
	Float:SpawnLocZ,
	Float:SpawnLocA
};
#if CITY_ == LS
new Spawns[][SpawnsEnum] = {
	{656.8817,-499.1801,16.3359,359.4021 },// Dillimore
	{-81.9810,-1562.9924,2.6107,220.6373 },// Flint trailerpark
	{-568.2385,-1033.9773,24.1512,236.9507 },// Flint huts
	{314.7237,-1776.0431,4.6824,180.3472 },// Santa Maria Beach carpark
	{1445.4264,-2286.8711,13.5469,88.8942 },// LS Airport carpark
	{2749.1538,-2390.8435,13.6495,179.0722 },// Ocean Docks
	{2870.6887,-1942.9655,11.1084,90.1584 },// Playa Beach
	{2821.6411,-1540.9597,10.9219,141.8733 },// East Beach carpark
	{2269.4304,-77.4366,26.5859,180.3458 },// PC carpark
	{1287.0060,190.2584,20.1562,157.7911 },// Montgomery trailerpark
	//{320.7392,-47.9860,1.5662,179.8072 },// Blueberry construction
	{333.1419,-1335.0247,17.9531,204.3630 },// Richman 24/7 carpark
	{764.9440,-1379.7729,13.6468,0.3950 },// Interglobal television
	{1361.7854,-1750.7207,13.5289,89.7371 },// commerce 24/7 carpark
	{1240.7367,-2029.7841,59.9809,269.3017 },// Observatory carpark
	{1683.9675,-1102.6963,24.0703,86.8959 },// Mulholland intersection carpark
	{2142.8740,-1171.5458,23.9922,271.3521 },// jefferson 24/7 carpark
	{2512.3982,-1672.9097,13.4953,75.3031 },// Grove street
	{2733.2434,-1841.1914,10.1307,174.3477 },// Los Santos forum carpark
	{2747.6477,-1084.8018,69.5781,105.1226 },// Saints Blvd hill
	{1042.3070,-1086.1741,23.8281,89.6534 },// Temple carpark
	{1128.8982,-1426.6661,15.7981,0.6043 },// Market mall
	{1875.7863,-1405.9149,13.5703,358.5343 },// Skatepark
	{290.7871,-1149.2280,80.9099,223.7691 },// Richman hills
	{1207.1481,-871.7902,43.2092,189.5071 },// Mullholand burgershot
	{2433.2661,115.4294,26.4689,269.7472 },// PC NorthEast
	{2357.5542,-653.2653,128.0547,267.9680 },// North Rock
	{886.7549,-35.3036,63.1953,68.2281 },// Fern Ridge hut
	//{91.8604,-164.9383,2.5938,269.8046 },// Blueberry Acres
	{824.1684,-2052.3345,12.8672,270.7321 }// Verona Beach pier
};
new SpawnsFBI[][SpawnsEnum] = {
	{1513.7909,-1449.0516,13.5469,358.3983}
};
new SpawnsCOP[][SpawnsEnum] = {
	{1580.2386,-1634.4849,13.5620,177.3429}
};
new SpawnsSHERIFF[][SpawnsEnum] = {
	{610.8305,-585.3387,17.2266,184.6325}
};
#elseif CITY_ == SF
new Spawns[][SpawnsEnum] = {
};
new SpawnsFBI[][SpawnsEnum] = {
	
};
new SpawnsCOP[][SpawnsEnum] = {
	
};
new SpawnsSHERIFF[][SpawnsEnum] = {
	
};
#else 
new Spawns[][SpawnsEnum] = {
};
new SpawnsFBI[][SpawnsEnum] = {
	
};
new SpawnsCOP[][SpawnsEnum] = {
	
};
new SpawnsSHERIFF[][SpawnsEnum] = {
	
};
#endif	