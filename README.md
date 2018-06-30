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
