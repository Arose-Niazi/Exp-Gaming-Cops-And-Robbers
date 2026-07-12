/*
	RouteConnector plugin include (minimal)
	Plugin by Gammer_Z — GPS pathfinding over scriptfiles/GPS.dat.

	The original include never shipped with this repo; this file declares
	exactly the natives and callback the CnR gamemode uses (CnR/server/gps.inc),
	matching the legacy plugin binaries under plugins/.
*/
#if defined _routeconnector_included
	#endinput
#endif
#define _routeconnector_included

native NearestPlayerNode(playerid);
native NearestNodeFromPoint(Float:x, Float:y, Float:z);
native GetNodePos(nodeid, &Float:x, &Float:y, &Float:z);
native CalculatePath(startnode, endnode, routeid, bool:calculate_polygon = false, Float:polygon_size = 12.0, bool:node_positions = false);
native gps_AddPlayer(playerid);
native gps_RemovePlayer(playerid);

forward GPS_WhenRouteIsCalculated(routeid, node_id_array[], amount_of_nodes, Float:distance, Float:Polygon[], Polygon_Size, Float:NodePosX[], Float:NodePosY[], Float:NodePosZ[]);
