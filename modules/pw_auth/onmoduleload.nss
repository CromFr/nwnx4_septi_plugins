#include "nwnx_sql"

void main()
{
	SQLExecDirect(
		"CREATE TABLE IF NOT EXISTS `pw_auth_accounts` ("
		+"  `account_name` varchar(32) COLLATE utf8mb4_bin NOT NULL,"
		+"  `password_hash` varchar(512) COLLATE utf8mb4_bin DEFAULT NULL,"
		+"  `password_salt` varchar(32) COLLATE utf8mb4_bin DEFAULT NULL,"
		+"  PRIMARY KEY (`account_name`)"
		+") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;"
	);
	SQLExecDirect(
		"CREATE TABLE IF NOT EXISTS `pw_auth_remembered` ("
		+"  `account_name` varchar(32) COLLATE utf8mb4_bin NOT NULL,"
		+"  `ip` varchar(15) COLLATE utf8mb4_bin NOT NULL,"
		+"  `cdkey` varchar(16) COLLATE utf8mb4_bin NOT NULL,"
		+"  PRIMARY KEY (`account_name`,`ip`,`cdkey`),"
		+"  KEY `fk_pw_auth` (`account_name`),"
		+"  CONSTRAINT `fk_pw_auth` FOREIGN KEY (`account_name`) REFERENCES `pw_auth_accounts` (`account_name`) ON DELETE CASCADE ON UPDATE CASCADE"
		+") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;"
	);

	ExecuteScript("x2_mod_def_load", OBJECT_SELF);
}