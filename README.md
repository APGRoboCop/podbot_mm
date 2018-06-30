# podbot_mm
Introduction
------------
[What is PODBot MM?]
PODBot MetaMod is an open source (GPL) metamod plugin that adds computer players (bots) to a popular game called Counter-Strike.

[Why is it called PODBot MM?]
POD is an abbreviation for "Ping of Death". It's called that way because it was derived from the "High Ping Bastard Bot" Botman was doing. And if you look at the Bots Ping they always have a Ping of 5 and the very first generation of Bots did Headshots all the time!  
The sufix mm to PODBot name has been added since it has been converted from standalone dll to metamod one and it started to use all benefities (and also all limitations) metamods gives.

[What are Bots able to do during the game?]
The Bots will automatically know the goals of the map. Counter-Terrorists will go for hostages or defuse the bomb, Terrorists will plant the bomb. VIP Bots will try to reach the Rescue Points. The goal selection is done dynamically and will be affected by several factors like Personality, Health, Team-mates near & Items the Bot is carrying. The Bots will also try to support team-mates and take cover when they think it's needed. They can use some buttons like for light-switches and for some doors triggered by the button. It's possible usage of lifts by bots (but it's not recommended - because of the possiblity getting stuck by them in some ways normally not used by human-players).

[What Mods are supported?]
PODBot MM supports Steam CounterStrike 1.6 and CZERO. Non-Steam CS 1.6 is NOT supported. Podbot mm should also work with CS 1.5, but some functions might be unavailable (like i.e. fluctuating ping). CS below 1.5 version wasn't tested with podbot mm.

[PODBot MM and CounterStrike: Internet vs LAN]
You can join a network game over the Internet or on a LAN without the necessity to uninstall the Bot. You cannot spawn bots when connected to an Internet server unless it is your server that's hosted, but you can spawn bots on a LAN if you are running the server (Listenserver).

[Why does PODBot MM need waypoints?]
Is there a Bot who is doing great without any kind of navigational help like waypoints, wayzones or pre-defined areas? It's almost sure you won't find any. The reason for this again is simple. It would just take too much CPU Power to realtime calculate all Geometry inside a map (and it will make too much lags). Imagine a Bot either shooting some dozens of TraceLines (like sonar by submarine) through the part of the map he's seeing or sorting all entity coordinates he encounters. Doesn't matter what he actually does, it was just way too slow for the state of CPU technology, when PODBot was created by CF. You would end up with a Bot which is either pretty good/humanlike at movement but stupid as hell, vice versa or both.
Don't think somebody tells this to discourage Bot authors attempting this. Just try!

[What types of map are supported?]
Escape(Es), Knife Arena(Ka), Death Match(Dm), Fantasy(Fy), Grenade War(He), Prepared Assault (Pa), Assasination\Vip (As), Sniper Wars(Awp), Hostage Rescue(Cs), Bomb Defuse\Plant(De), Hostage Rescue\BombPlant\Defuse (CsDe)

[What are requirements to play with PODBot mm?]
The Bot currently works with Counter-Strike 1.5 (with some small limitations described above), Steam 1.6 and CZERO (with both Retail and Mod versions)
If you're not using the CounterStrike Retail Version, Half-Life version 1.1.0.0 or higher is required to use this bot. If you haven't updated yet your Half-Life version to 1.1.0.0 (or higher) you will need to do so before using it. For Counter-Ctrike 1.6 or CZERO You need steam latest version installed correctly.

[Disclaimer]
Software under this agreement is under no kind of warranty. Software under this agreement is provided as is, and isn't guaranteed in any way by the Bot author/deveolopers. Use this software at your own risk!

[Copyrights]
This bot is copyright Markus "Count Floyd" Klinge and no one else. He stopped working on it and released his source to the public so don't annoy him with help requests. Post your requests in the Bots United forums instead or use one of the other POD-bot clones featured at Bots United ( IvPBot, E[POD]Bot, YaPB...).

Installation
------------
This Install procedure is primarily written for podbot, but many other metamod plugins work the same so can be used similarly.
plugin.dll should be of the form "plugin_mm.dll" and resides normally in directory as follows.

[Assuming]
We assume:
You already have Steam or CS1.5 installed;
You have bought and ran Counter-Strike at least once;
You are not installing PODBot mm from All-In-One package
You have basic computer knowledge (if not - go ask for help at the forums, but use "search function).
You know what is the listenserver (the PC when You create a game, You are playing from it and Your friends are able to join the game on Your PC)
and the dedicated server (the PC-box You start the game and You can see only the console of it, but You are not playing from this PC).

[Retrieving]
Download the latest Metamod.
Choose one of the Metamod Binaries according to the operating system You have.
Most likely :
a) for windows metamod-1.19-win.zip
b) for linux32 metamod-1.19-tar.gz
c) for linux amd64 metamod-1.19-amd64.tar.gz
You may save it anywhere.
Download the lastest fullpack of PODBot mm.
You may save it anywhere.

[Arranging]
Extract the files from the archives.
Use something like 7-Zip.
Find the cstrike or czero folder (according to the game You have (CS1.5, CS1.6-Steam, CZERO-Steam supported).
On CS1.5 most likely C:\Program Files\Sierra\Counter-Strike\cstrike
On CS1.6 listensever most likely C:\Program Files\Steam\steamapps\your@email\Counter-Strike\cstrike
On CS1.6 dedicated server most likely C:\Program Files\Steam\steamapps\your@email\dedicated server\cstrike
On CZERO listensever most likely C:\Program Files\Steam\steamapps\your@email\Counter-Strike\czero
On CZERO dedicated server most likely C:\Program Files\Steam\steamapps\your@email\dedicated server\czero
Once You know where is Your the most important folder (cstrike or czero) Yoy can go ahead with installing.
Create a folder called addons in the above folder.
Create a folder called metamod in the addons folder.
Create a folder called dlls in the metamod folder.
Move the file called - if You have:
a) windows - metamod.dll
b) linux - metamod_i386.so
c) linux amd64 - metamod_amd64.so
into the dlls folder.
Move the folder called podbot extracted from podbot_full_V3_BXY.zip to the addons folder.

If You had already installed some older than podbot 2.6 mm version (some podbot_mm.dll created after 26.04.2004), we advise you to delete any previous .pxp files (bots experience files). You can find them inside addons\podbot\wptdefaut folder.
Since the bot now supports the new CS 1.6 weapons, you MUST use the included botweapons.cfg file which takes them into account. If you attempt to use an older weapon configuration file from POD-bot 2.5 or 2.6 (but not 2.6mm) or some old ".pxp" files the bot will crash!!!
Podbot mm requires all new config files (included in the fullpack) to the correct work.

[Setting]
Set Metamod to load PODBot mm
Run Notepad.
Type:
a) windows: win32 addons/podbot/podbot_mm.dll
b) linux: linux addons/podbot/podbot_mm_i386.so
c) linux amd64: linux addons/podbot/podbot_mm_amd64.so
into the textarea exactly as-is.
Go to Your addons/metamod folder. Check if there is a file called plugins.ini.
If you already have metamod installed (and the above file is there), You only need to add one of these above lines to metamod's plugins.ini file.
If no - You need to save Your file You created by Notepad as plugins.ini in the correct folder. To do this - Push Ctrl and S at the same time.
Change Save as type: to All files
Browse to your addons/metamod folder.
Type plugins.ini into the textbox labeled File name: exactly as-is.
Left click Save. At this moment You have prepared metamod to load podbot mm.
Set Counter-Strike to launch Metamod
Inside the cstrike (or czero) folder You need to find the file called liblist.gam . Make a copy of it and rename the copy to liblist.old
Open by Notepad the file liblist.gam, find the line gamedll "dlls\mp.dll" and change it to:
a) windows users: gamedll "addons\metamod\dlls\metamod.dll"
b) linux users: gamedll_linux "addons/metamod/dlls/metamod_i386.so"
c) linux amd64 bit users: gamedll_linux "addons/metamod/dlls/metamod_amd64.so"
then save it.
If You have CS1.6 or CZERO - Run Steam.
Login to your account.
Right click on Counter-Strike.
Left click on Properties.
Left click on Set launch options...
Make sure that textbox is empty or at least it calls metamod with on such command lines:
a) for windows - -dll addons/metamod/dlls/metamod.dll
b) for linux 32 - -dll addons/metamod/dlls/metamod_i386.so
b) for linux amd64 - -dll addons/metamod/dlls/metamod_amd64.so
Left click OK.
Left click Close.
That command line here and the liblist.gam calls for metamod they are equivalent, but the higher pririty to call the game dll is the command line from launch priporeties.
At this moment You have prepared Counter-Strike to load metamod.

[Uninstalling]
If You want to uninstall podbot mm only, but Yo still want to use metamod (because of using other mm plugins like AMX MOD X, AMX etc.) all You need is just remove the line from addons\metamod\plugins\ini file:
a) windows: win32 addons/podbot/podbot_mm.dll
b) linux: linux addons/podbot/podbot_mm_i386.so
c) linux amd64: win32 addons/podbot/podbot_mm_amd64.so
