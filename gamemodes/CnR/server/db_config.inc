/*  CnR / server / db_config.inc  —  MySQL connection credentials
 *
 *  Persistence backend: MySQL (maddinat0r R41 plugin).
 *  Toggle LOCAL_DB in CnR.pwn to switch between the local and remote credential
 *  sets below. DO NOT commit real production credentials — replace the
 *  placeholders locally or (preferably) inject them via your deployment.
 */
#if defined _cnr_db_config_included
	#endinput
#endif
#define _cnr_db_config_included

#if defined LOCAL_DB
	#define SQL_HOST	"127.0.0.1"
	#define SQL_USER	"admin"
	#define SQL_PASS	""
	#define SQL_DB		"cnr"
#else
	#define SQL_HOST	"CHANGE_ME_REMOTE_HOST"
	#define SQL_USER	"cnr"
	#define SQL_PASS	"CHANGE_ME_REMOTE_PASS"
	#define SQL_DB		"cnr"
#endif
