
# PW Auth

Early authentication system for Neverwinter Nights 2 servers

# Run the demo module

You need to have a MariaDB (or MySQL) server running and nwnx4 configured to use it.

1. Copy this folder (`modules\pw_auth`) to `Documents\Neverwinter Nights 2\modules`
2. Install xp_msgServer from this repository
3. Edit nwnx4.ini:
	- Load the pw_auth module: `parameters = -moduledir pw_auth`
	- Add xp_msgServer to the plugin list (after xp_bugfix): `plugin_list = ..., xp_msgServer`
4. Edit xp_msgServer.ini:
	- Set `EnableConnectionSystem = 1`
	- Set `OnConnectionScript = pw_auth_onconnection`
5. Start nwnx4 + nwn2server, launch the game and connect to your server

# Install

## 1. Copy files

Copy the following files to your module folder:
- `pw_auth_*.nss`
- `gui_pw_auth_*.nss`

Copy the following files inside one of your HAK files:
- `pw_auth_*.xml`

## 2. Configure

`pw_auth_conf.nss` defines constants and functions thet needs to be modified to suit your PW needs

# Update

Follow the same steps as in the [Install](#install) section, except do not overwrite `pw_auth_conf.nss`. You need to compare the two versions of `pw_auth_conf.nss` to update your copy accordingly.