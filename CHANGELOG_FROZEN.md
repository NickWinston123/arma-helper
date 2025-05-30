
#### Changes since 0.2.9.0:

##### Fixed Bugs

 * Can't open the .appimage file. Missing libwebp.so.5 ([#54](https://gitlab.com/armagetronad/armagetronad/-/issues/54))

##### Contributors

Manuel Moos


#### Changes since 0.2.8.3.5:

##### Fixed Bugs

 * Debian builds: Documentation html files have empty last change ([#9](https://gitlab.com/armagetronad/armagetronad/-/issues/9))
 * Socket and unclean exit trouble due to fork(), execve() and exit() interaction ([#13](https://gitlab.com/armagetronad/armagetronad/-/issues/13))
 * Ubuntu PPA release candidate builds would overwrite previous stable releases ([#15](https://gitlab.com/armagetronad/armagetronad/-/issues/15))
 * Language string identifier silence_player_text used twice ([#22](https://gitlab.com/armagetronad/armagetronad/-/issues/22))
 * Bad video mode error recovery ([#23](https://gitlab.com/armagetronad/armagetronad/-/issues/23))
 * AppRun script does not work if call path contains spaces ([#24](https://gitlab.com/armagetronad/armagetronad/-/issues/24))
 * Included forum links outdated ([#25](https://gitlab.com/armagetronad/armagetronad/-/issues/25))
 * Client segfaults on OpenBSD ([#26](https://gitlab.com/armagetronad/armagetronad/-/issues/26))
 * Client compiled with clang 10.0 (optimized) sefgaults on logout ([#28](https://gitlab.com/armagetronad/armagetronad/-/issues/28))
 * Valgrind reports uses of unitialized, freed or invalid memory ([#29](https://gitlab.com/armagetronad/armagetronad/-/issues/29))
 * Windows: DPI scaling behavior wrong ([#33](https://gitlab.com/armagetronad/armagetronad/-/issues/33))

##### New Features

 * Update German translation ([#2](https://gitlab.com/armagetronad/armagetronad/-/issues/2))
 * Adapt ChangeLog and fingerprint generation to git ([#11](https://gitlab.com/armagetronad/armagetronad/-/issues/11))
 * Integrated build system ([#14](https://gitlab.com/armagetronad/armagetronad/-/issues/14))
 * Make playback time display optional ([#16](https://gitlab.com/armagetronad/armagetronad/-/issues/16))
 * Deploy to itch.io ([#19](https://gitlab.com/armagetronad/armagetronad/-/issues/19))
 * Credit contributors in patch notes ([#21](https://gitlab.com/armagetronad/armagetronad/-/issues/21))

##### Other Changes

 * Make custom camera the default for new players ([#35](https://gitlab.com/armagetronad/armagetronad/-/issues/35))

##### Contributors

Daniel Harple, Hugh McMaster, Luke-Jr, Manuel Moos, Uzix, fman23, zolk3ri

##### Other changes on the 0.2.9 branch

 * Added detection and reaction code for timing assist bots
 * /shuffle now works before you actually join a team
 * Implemented /shout command and associated settings
 * Fixed suspension and silenced status not being re-applied after a player
   disconnects and rejoins a server.
 * Tweaks to enemy influence system.
 * Ingame menu and console now are drawn on top of a semi-transparent,
   darkened area for increased readability.
 * New first start menu with clearer language selection and initial setup.
 * Tutorial match against one AI at slower speed.
 * Tutorial tooltips for the most important keybindings.
 * ADD_MASTER_SERVER command to announce a server to a new master server.
   Simplifies the process to list your server on a subculture.
 * Team launch positions logged to ladderlog with POSITIONS event. Disabled by
   default.
 * New command-line option "--input" added to poll for input from a file
   instead of stdin.
 * Added WHITELIST_ENEMIES_[IP/USERNAME] to allow players to be enemies, even
   if they come from the same IP address and ALLOW_ENEMIES_SAME_IP is
   disabled (which is its default setting).
 * GAME_END, NEW_MATCH, and NEW_ROUND ladderlog events include date and time.
 * Added ENCODING ladderlog event, which specifies the encoding for data in
   ladderlog.txt.
 * "--input" now can be used more than once to read from multiple files or pipes.
 * new team management ladderlog messages:

        TEAM_CREATED <team name>
        TEAM_DESTROYED <team name>
        TEAM_RENAMED <old team name> <new team name>
        TEAM_PLAYER_ADDED <team name> <player>
        TEAM_PLAYER_REMOVED <team name> <player>
        
 * Manage external scripts on Unix dedicated servers. New commands: 
   SPAWN_SCRIPT, RESPAWN_SCRIPT, FORCE_RESPAWN_SCRIPT, KILL_SCRIPT,
   LIST_SCRIPTS, SCRIPT_ENV.
 * New setting ACCESS_LEVEL_ANNOUNCE_LOGIN that determines if a player's
   login/logout message can be announced.
 * Authentication is now enabled by default.

#### Changes since 0.2.8.3.4:

 * Security fix: Check that the remote is allowed to create an object
   before creating it, not decide whether it gets to keep it after
   it already has been created and potential damage has been done.
   No arbitrary code could be executed, but a client could effectively 
   shut down a server by sending crucial objects, such as the main game
   timer or the game itself.
 * Compilation fixes for current systems.

#### Changes since 0.2.8.3.3:

 * Eliminated undefined behavior that was fine in gcc 5, but now
   (rightfully!) turned into crashes in gcc 6.
 * Eliminated compilation disambiguity that was just causing a mild
   performance degradation in gcc 5 and is now a compilation failure in
   gcc 6.
   
#### Changes since 0.2.8.3.2:

 * security fix: do not read ahead of the beginning of network buffer.
 * security fix: don't attribute network errors from processing random
   packets to the connection to the server 
 * security fix: while at it, don't process random packets unless they
   may be important
 * fix for potential crash with friend list filtering
 * intel driver compatibility
 * fix for rare crash with sound lock
 * fix for camera turning for bizarre axis configurations

#### Changes since 0.2.8.3.1:

 * security fix: old style action commands from clients no loger cause hangs and crashes
 * security fix: oversized packets are ignored properly
 * security fix: never read one byte outside of the received buffer
 * security fix: only include .cfg files from the var subfolder
 * compiler compatibility: adapted to gcc 4.60
 * smaller spelling fixes
 * FOV calculations now correct for widescreen and horizontal splitscreen
 * Additional checks against illegal values passed to renderer

#### Changes since 0.2.8.3:

 * fixed temporary sound disappearance when alt-tabbing away and back
 * default subcultures are now distributed
 * enable 32 bit color depth at desktop resolution
 * no more display lists for SiS cards
 * spelling mistakes and missing German translations

#### Changes since 0.2.8.3_rc4:

 * New setting: KEEP_PLAYER_SLOT allows the server to kick (preferably) spectators
   if it gets full so there is always one slot open for players
 * New setting: ACCESS_LEVEL_AUTOKICK_IMMUNITY sets the access level required 
   to be immune from such kicks (and idle autokicks, too)
 * Intercepted chat commands are now written to ladderlog.txt. Format:
   COMMAND /command-intercepted player [command arguments]
 * Increased default speed of server pinging
 * Reordered game menus a bit
 * fast finish works again

#### Changes since 0.2.8.3_rc3:

 * Pinging previously popular servers earlier for quicker server
   browsing
 * Improved master server random polling, the second polled master now
   also is random, and all masters are polled if need be
 * Custom camera snaps to cycle direction once when activated if the turn speed
   is zero.
 * RESERVE_SCREEN_NAME now works regardless of the ALLOW_IMPOSTOR setting.
 * USER_LEVEL now escapes the given authenticated name so you don't have to
   guess the escaping rules. Ex: USER_LEVEL "name with spaces@forums" 2
   changes user level for name\_with\_spaces@forums

#### Changes since 0.2.8.3_rc2:

 * Fixed crash with server polling
 * ROUND_WINNER and MATCH_WINNER ladderlog events now include the players of
   the team.
 * Camera switches to user preferred camera when you die in incam mode
 * Better support for custom language files
 * Players can no longer spam chat with /shuffle messages. Added new setting
   SHUFFLE_SPAM_MESSAGES_PER_ROUND, a per round, per player limit on the number
   of shuffle messages displayed.

#### Changes since 0.2.8.3_rc1:

 * Fixed dark color filters, such as FILTER_DARK_COLOR_STRINGS, to not filter
   the reset color.
 * ASE moviepack models no longer have a vertex count limit
 * Fixed endless recursion crash between rounds

#### Changes since 0.2.8.3_beta3:

 * Chat prefix spam protection. Guards against prefix spam, such as when a
   user always changes their chat message color, or prefixes all messages
   with a clan tag. Toggle with the PREFIX_SPAM_ENABLE setting.
 * Fixed VOTING_MATURITY to use the time when the player entered as its 
   reference time.
 * Fixed single player highscore list not containing everyone.
 * Fixed generation of duplicate names in highscore lists.
 * Fixed camera crash when the grid is empty.

#### Changes since 0.2.8.3_beta2:
 * Fixed 'invincibility' bug. It really was cycles freezing in time.
 * Fixed sporadic sound related crash at round end.
 * setting rename: SILENCE_ALL -> SILENCE_DEFAULT.
 * Sound no longer gets muted after you switch the sound settings.
 * Fixed object collision detection that would sometimes pick a wrong 
   wall to collide with, most likely the reason behind wrongly accredited
   kills.
 * Fixed inability to switch viewport assignment in four player mode.
 * Better handling of lag clusterbombs.
 * Server disconnects now also take immediate effect even when you're
   in a menu or chatting.

#### Changes since 0.2.8.3_beta1:
 * Made ready for Armatrators
 * Authentication routines now use utf8 as forward- compatibility with
   trunk. Previously, non-ASCII usernames simply didn't work at all.
 * Better lag slide protection: clients no longer get confused when
   the server sends their cycle back in time, and the server no longer
   does that for old clients.
 * Fixed rare wallpassing bug; it required what whould usually be an
   instakill and infinte wall length.

#### Changes since 0.2.8.2:

New settings and commands:

 * MIN_PLAY_TIME_*: minimal play time required before you are allowed to
   play on this server.
 * SPAM_AUTOKICK_COUNT: minimal number of spam warning messages you get in a
   row before you get kicked.
 * FORTRESS_COLLAPSE_SPEED: tune the collapse speed of fortress zones.
 * VOTE_KICK_REASON: default reason given on kick votes.
 * "/vote" chat interface to votes; "/vote kick <player>" issues a kick vote.
 * new vote type "/vote suspend <player>" suspends a player for 
   VOTING_SUSPEND_ROUNDS rounds.
 * new vote type "/vote include <file>", includes configuration file
   vote/<file>.
 * new vote type "/vote command <command>" executes console command
   <command>. (Both that and /vote include are by default only available
   to moderators).
 * VOTING_HARM_TIME: time between all harmful votes against a player
 * VOTING_KICK_MINHARM: auto-transform the first few menu issued kick 
   votes to suspensions
 * (UN)SUSPEND kicks a player to spectator mode with a timeout. Intended to
   let new players watch before they play.
 * VOTES_CANCEL cancels all running polls.
 * VOTING_SUSPEND [<minutes>] disallows voting during a specified amount of time.
 * ZONE_ALPHA_TOGGLE to override the systemwide alpha blendings setting and
   have zones rendered in wireframe mode even when alpha blending is on.
 * ZONE_ALPHA(_SERVER) to control intensity of zone rendering.
 * SCORE_HOLE for awarding sacrifice.
 * SCORE_SURVIVE for player survival and FORTRESS_HELD_SCORE for not losing
   each fortress zone.
 * KICK_TO and MOVE_TO: redirect a client to a different server
 * DEFAULT_KICK(_TO)_MESSAGE: default reason given to players for a kick
 * DEFAULT_KICK_TO_SERVER/PORT: default redirection target for KICK/MOVE_TO.
 * RENAME renames a player.
 * If all clients support it, PING_CHARITY_MIN/MAX can be used to constrain ping charity.
 * VOTE_KICK_TO_SERVER/PORT: redirection target for vote kicks.
 * SILENCE and VOICE allow to control chat of annoying players.
 * SILENCE_ALL can be used on the server to inhibit all cross-team communication unless specifically voiced.
 * ENABLE_CHAT can be set to 0 to disable all chat. If it is set on the server it only disables public chat and logged in players can still speak.
 * LAG_*: controls game level lag compensation.
 * FILTER_NAME_MIDDLE: collapses multiple whitespaces to one in player names.
 * FORTRESS_CONQEST_TIMEOUT: if an enemy contact with a zone happens longer ago than
   this value, it is discarded from zone memory.
 * TOPOLOGY_POLICE_PARALLEL, extra topology police flag to control the checks
   for walls getting copied into the grid in exactly the same place.
 * CAMERA_GLANCE_*: separate settings for the glancing camera, work like CAMERA_CUSTOM_*.
   Likewise, CAMERA_SERVER_GLANCE_* can replace CAMERA_SERVER_CUSTOM_* when glancing.
   CAMERA_*CUSTOM_GLANCE* are server side settings that can control whether server or
   clientside settings are used in various situations, see settings.cfg for details.
 * CYCLE_BOOST_? and CYCLE_BOOSTFACTOR_? for boosts when breaking away from walls,
   see settings.cfg or config.html for details
 * CYCLE_ACCEL_TUNNEL, works like CYCLE_ACCEL_SLINGSHOT, but is active when
   none of the two walls you're in between is your own.
 * CYCLE_WIDTH for making it impossible to squeeze through tight tunnels,
   see settings.cfg for details
 * VOTING_MATURITY controls how long you have to wait after login before
   you can issue kick votes.
 * CYCLE_DELAY_DOUBLEBIND_BONUS is an extra factor applied to CYCLE_DELAY
   for turns in the same direction. Can be used to limit the advantage
   doublebinders get while not punishing players who press both left and
   right at the same time.
 * LAG_O_METER_(TRESHOLD|BLEND|USE_OLD): Configure the minimal size of the
   Lag-O-Meter, its color and whether to scrap it all and stick with the old
   one.
 * ALLOW_TEAM_CHANGE: if set to 0 players won't be able to join a team, create
   a new team or change to another team.
 * ALLOW_TEAM_CHANGE_PLAYER/DISALLOW_TEAM_CHANGE_PLAYER: allow/disallow specific
   players to switch teams
 * INTERCEPT_COMMANDS is a space-delimited list of chat commands that shall be
   intercepted by an external script. Example value: /login /admin /teamleave
   /teamshuffle /shuffle /team
 * INTERCEPT_UNKNOWN_COMMANDS is a boolean specifying if an external script
   wishes to handle unrecognized chat commands. This would allow hooking
   arbitrary things like /foobarz or /mode ctf
 * CONSOLE_LADDER_LOG boolean places the ladderlog on stdout prefixed with
   "[L] "
 * PLAYER_MESSAGE <user ID or name> <Message>: Like /msg, but from the
   console
 * All ladderlog outputs can be enabled or disabled separately by using
   LADDERLOG_WRITE_*. LADDERLOG_WRITE_ALL enables or disables all ladderlog
   output alltogether.
 * CHAT_LOG controls whether to write a machine-readable chatlog to
   var/chatlog.txt
 * CONSOLE_DECORATE_TIMESTAMP write a timestamp for every console message?
 * LADDERLOG_DECORATE_TIMESTAMP prefix every ladderlog message with a timestamp?
 * SLAP <player> [points] sends a message to everyone and subtracts a number
   of points (default: 1) from <player>'s score. Negative points are
   supported.
 * AUTO_TEAM_SPEC_SPAM to disable the spectator join/leave messages you get
   when AUTO_TEAM is enabled

If --enable-armathentication was activated:

 * /lock, /unlock, /invite and /uninvite team management chat for players of
   access level ACCESS_LEVEL_TEAM or better.
 * /teams chat command that rougly prints team layout.
 * ACCESS_LEVEL_IPS to let admins see IPs of players in the list.
 * ACCESS_LEVEL_SPY_MSG/TEAM to let admins spy on team/private messages.
 * CASACL to temporarily raise the access level
 * ACCESS_LEVEL to modify the required access level to change settings
 * AUTHORITY_WHITELIST and AUTHORITY_BLACKLIST to filter authorities you want on your server.
 * GLOBAL_ID_ENABLED to toggle remote accounts
 * LOCAL_USER, LOCAL_TEAM for local login accounts
 * MD5_PREFIX/MD5_SUFFIX for additional password scrambling
 * USER_LEVEL to grant users various access levels
 * RESERVE_SCREEN_NAME to reserve a screen name to a certain player
 * USER_ALIAS to bend authentication names around 
 * (UN)BAN_USER to ban really stupid users based on their global user ID, 
   BAN_USER_LIST to show a list.
 * ACCESS_LEVEL_OP/ADMIN/CHAT/PLAY/PLAY_SLIDING to control who can do which things
 * /login chat command then uses the more secure hashed base logins
 * new chat commands /op and /deop to change other players' access rights

Featurelets:

 * Added Polish language file
 * clientside play time tracking and server controlled minimal required experience
   to be allowed to play.
 * color codes in chat increase the spam level logarithmically.
 * /team messages get a reduced spam level; reduction factor is given by 
   (team size)/(number of total players).
 * /team messages can be used by spectators to chat with other spectators only.
 * Performance optimizations. Using OpenGL display lists to cache geometry.
 * Removed ztrick, antialiasing and perspective correction settings. They all had only one right
   setting and caused us some interesting bug reports.
 * Added PLAYER_RANDOM_COLOR for lazy people who just want to have a
   color that differs from everyone else on the server.
 * Configuration files get reloaded on SIGHUP
 * all admin commands now also accept partial name matches, like /msg, and check for case sensitive
   matches in the screen name first, then the user name, then case insensitive matches in the two.
 * --enable-krawallserver has been actually implemented now, and it enables secure logins
   to accounts local to the server and not-so-secure logins managed by authentication servers.
 * A subculture list for server groups that are not managed by our main master servers
 * A friends list and filter for the server browser that shows you only servers with
   your friends on them.
 * The client now can handle network traffic while it is waiting for the graphics
   card to swap buffers.
 * The dedicated server no longer uses a fixed delay, but a select() call (that
   returns when network data arrives) to idle. When data arrives, it does 
   the minimum amount of work before it sends out the responses, resulting in
   lower latency. It is now safe to decrease DEDICATED_FPS to 20 or 10, latency
   is no longer influenced by that setting.
 * Game level lag compensation. When a command arrives from a client that is delayed,
   it is backdated to the time it was issued, provided enough "lag credit" is left.
   The client is informed of the delay so it can compensate in the future, which it does
   by forging the game timer.
 * Nonnormalized axes and axes with odd turning directions are now properly supported
 * The camera has a memory which player you like to watch. Every time you manually switch
   the camera and your favorite player is not dead, the favorite is set to the currently 
   watched player. The next time the camera doesn't know what to watch, your favorite
   will be selected.
 * Players leaving to spectator mode with <0.2.8.2 clients are handled like
   spectators of 0.2.8.2 clients: they stay visible to the others.
 * Kick statistics are now persistent across server runs.
 * MESSAGE_OF_DAY is now displayed fullscreen if both server and client support it
 * Manual fullscreen messages can be triggered with
   FULLSCREEN_MESSAGE <timeout> <message content>
 * Axes Indicators by meriton, can be enabled using the AXES_INDICATORS
   setting.
 * New "About" menu item in the main menu displays the version of
   Armagetron Advanced, the paths it uses, links to the main site, wiki
   and forums and the names of the project admins.
 * New entries in ladderlog.txt:
   - GAME_TIME <time> (only if enabled via the LADDERLOG_GAME_TIME_INTERVAL
     setting): The time that has passed since the last start of a round
   - BASEZONE_CONQUERED <team> <x> <y>: A fortress zone belonging to <team>
     at the coordinates (<x>, <y>) has been conquered
   - BASEZONE_CONQUERER <player>: at the time of the last BASEZONE_CONQUERED
     message, <player> has been in the conquered zone
   - ONLINE_PLAYER <player> [<ping [<team>]]: At the beginning of the round,
     <player> had <ping> and was on <team>. This line is not printed for bots
   - NUM_HUMANS <n>: There are <n> players that are not AIs and are part of a
     team.
   - ROUND_SCORE has the player's team name appended.
   - ROUND_SCORE_TEAM: score a team accumulated during the last round.
   - WAIT_FOR_EXTENAL_SCRIPT: printed if the setting of the same name is
     enabled. In this case the server will delay the start of the next round
     until WAIT_FOR_EXTENAL_SCRIPT_TIMEOUT is reached or the setting is
     disabled.
 * Wrapped console lines are indented by an amount of spaces that can be
   specified using the setting CONSOLE_INDENT.
 * /help command that supports multiple help topics. Use
   HELP_INTRODUCTORY_BLURB and ADD_HELP_TOPIC to control its contents
 * /rtfm command that allows moderators to send help messages to newbies
 * /players now shows the players' color and allows searching
 * TEAM_ELIMINATION_MODE sets the way ArmagetronAd removes teams when there's too much teams.

Bugfixes:

 * Added compensation for drifting timers (observed often with
   overclocked mainboards)
 * Text input fields now let the text wrap correctly and make use of the space allocated
   for them in a not-completely-dumb way. And *sigh* depending on the text field, color
   codes are either ignored (for the usernames) or both displayed in text and rendered.
 * "KICK 2pack" no longer kicks user 2 instead of player 2pack.
 * Fixed various trail end related extrapolation/simulation inaccuracies that looked
   like lag.
 * When extrapolating, the game's sensors never detected own or teammates' walls,
   it mistook them for enemy walls.
 * The acceleration and boost system always used settings for enemy walls instead of
   settings for team players' walls. That means that old clients connecting to a
   0.2.8.3+ server that has CYCLE_ACCEL_ENEMY and CYCLE_ACCEL_TEAM set to different
   values experience syncing problems. The default behavior therefore is to lock 0.2.8.2
   and earlier out of your server if you set one of them away from the default. 
   If you want to set both accelerations away from the default, but to the same value,
   there won't be a compatibility problem; in that case, set CYCLE_ACCEL_ENEMY_OVERRIDE
   and CYCLE_ACCEL_TEAM_OVERRIDE to 0 and old clients will be allowed in. However, you
   should manually lock out players with 0.2.7 clients, for example by modifying
   CYCLE_ACCEL_RIM a tiny bit.
 * "Team Red" bug. If the server allows maximally two teams, no team ever
   gets to be Team Red.
 * Phasing made even more difficult; nice side effect of the fix is
   reduced network bandwidth usage.
 * Speed gauge resets between rounds.
 * More accurate Lag-O-Meter by meriton.
 * Zones were sometimes invisible.

Internals:

 * A complete netsync cycle is now sn_Receive(); nNetObject::SyncAll(); sn_SendPlanned();.
 * Ping charity is now completely server controlled.

#### Changes since 0.2.8.1:
 * Timestamps and teamscores added to scorelog.txt
 * Dedicated server now works on FreeBSD and OpenBSD
 * User running the dedicated server is called "armagetronad" again, the
   longer "armagetronad-dedicated" caused problems with BSD
 * Test versions refuse to connect to servers more than one version ahead
 * Prepared client for respawns
 * Team spawn formation is now configurable
 * Added reasons to bans
 * Added spectator autokicking
 * Added history to chat and console (wrtlprnft). Press "Cursor up" to
   edit and repeat previous chats and console commands.
 * You only enter a game once your client is synced
 * /msg now prefers exact matches over partial matches
 * Cycles now have better memory for pending turns (wrtlprnft)
 * Added player join/leave/rename messages to ladderlog.txt with IPs
 * Ping variance influence on packet loss tolerance code is now clamped
   by the regular, configurable, packet loss tolerance: effect of variance
   can be no bigger than the effect of ping.
 * Spectators are now regular players and can chat
 * The /msg command now converts the entered pattern into the internal
   name so if you write "/msg öäü" and there is a user named "öäü" with
   an internal nick of "oau" it will still get matched
 * Team changes are now only executed when allowed, the need for the 
   temporary/permanent team imbalance setting distinction has gone away
 * Added brake toggle button
   Bugfixes:
 * Client sometimes crashed when leaving a server with AIs on over the menu
 * The server starting scripts were chowning /var/run and /var/log to armagetronad
 * Short names could crash the server
 * Direct cycle collisions sometimes lead to both passing trough each other
 * Init scripts now installed correctly with --enable-initscripts
 * Hopeless pending team change wishes are deleted
 * Large timesteps are now broken down into smaller ones on the game level 
 * Pretend keys are released before entering chat
 * Two cycle steering events were sent if you doublebound the brake
 * Large speed boosts out of game rule bounds were possible during network
   glitches
 * Debug recording reliability increased, function call with undefined return
   value eliminated
 * Recording and scorekeeping no longer conflict
 * Input during a recording now has an end marker, making quick keypresses
   after leaving a menu not break it
 * CYCLE_RUBBER_DELAY was overridden sometimes by packet loss tolerance
 * ALLOW_CONTROL_DURING_CHAT now ignores modifier keys like shift and
   has been reactivated
 * The camera sometimes got stuck in free mode for the first round you watch
 * Crashfix in hole blowing code when no cycle was set
 * After a "perfect" 180, you're more often on the right side of your own wall
   That code caused some havoc at first, so it has been reworked
 * The smart cam did not like fluctuating framerates
 * Improved debug recording reliability: multiple master server visits
   and too quick keypresses were causing trouble.
 * Canonical DESTDIR support and added ROOTDIR support for testing
 * Login floods are detected and ignored
 * Player name updates sanitized
 * Disabled complicated feasibility tests for team menu entries, 
   they did not have the full information and were often wrong
 * Team score only added to player score if no teamplay is possible

#### Changes since 0.2.8.0:
 * INCLUDE console command now gives a file not found error
 * ALL user given paths are validated for security problems in a special function
 * Fallback to default map on map load failure works again (broken sine 0.2.8.0_rc4)

#### Changes since 0.2.8.0_rc4:
Bugfixes:

 * SECURITY FIX: via a MAP_FILE path with included .., it was possible
   to write files anywhere, provided they did not exist previously.
   Clients with this vulnerability are locked out of the server now.
 * ALLOW_CONTROL_DURING_CHAT has been disabled for now.
 * Another connection termination bug.

#### Changes since 0.2.8.0_rc3: 
 * Added spectator mode toggle key.
 * Thresholds when a white background bar is rendered behind texts is
   now configurable in the expert section of settings.cfg.
 * Ingame menu trigger key is configurable now, only ESC stays hardcoded.
 * The map parser checks whether the map is stored at the right location.
 * Map validation errors are now visible on the client.
 * Made magic numbers used by the smart camera configurable for user tweaking.
   This is undocumented and unsupported.
 * The server browser now shows a B beside the score to mark bookmarked servers.
 Bugfixes:
 * -Os compile option is filtered out if GCC 3.3 is used, it makes the build go bad.
 * Extremely low values of CYCLE_DELAY were not honored, an arbitrary delay of .005s
   was added to it.
 * On entering a server, the camera would sometimes be stuck in free mode.

#### Changes since 0.2.8.0_rc2:
 * Almost final artwork
 * Moviepacks can now also contain texture replacements for the default cycle model
 * Added number of teammates alive to HUD
   Bugfixes:
 * Dual textured floor rendering is now disabled without alpha blending as it
   did not work anyway.
 * Lag-O-Meter now has the correct size (the old, buggy one can be restored by
   setting LAG_O_METER_SCALE to .5).
 * The network system made some IDs randomly unusable, causing zombie
   cycles, invisible walls and disconnections.
 * The AI Team from a previous local game session was sometimes still visible
   on the score table in the following network game on a remote server.
 * The server did not start a new match when temporarily, only spectators were online.
 * Player walls were rendered twice.
 * The server wrote console center messages to stderr.
 * UNBAN_IP did not work at all.
 * The code that should kill a cycle in performance/DOS protection
   circumstances just crashed.

#### Changes since 0.2.8.0_rc1: 
 * Bugfixes:
   - Dedicated server for Windows works again.
   - Cycle turns are now executed more accurately on the server.
   - Timer stuttering at end of round (was non-critical, but ugly).
   - Custom camera did not turn when the internal camera was
     configured not to turn.
   - Idle player kick code crashed when it kicked a client with
     many players.
   - The code that removes inactive players after a while removed
     the AI players.
 * Reverted smart camera movement to pre-beta4 behavior,
   the new code made some players seasick :)
 * Ingame admin interface gives more useful information to the invoker 
 * Small font rendering improvements
 * Zone conquest message, if it is not the one that ends the round,
   now states the name of the zone owning team.

#### Changes since 0.2.8_beta4.2:
 * Fixed bugs:
   - Random client disconnection
   - Another vote-not-appearing-on-client bug
   - Invisible wall segments right after turn
   - Arena initialization before all settings are transmitted from the server
   - The server was using more rubber than the client sometimes
   - Crash with moviepack and some maps
   - Object sync messages not accepted by client in second network session
 * Improved kill/suicide/team kill distinction (ENEMY_* settings)
 * Chatbots and idle players can be removed from the game and kicked
   (CHATTER/IDLE_REMOVE_TIME,IDLE_KICK_TIME)
 * Players/team leaders can pause the game before a rond starts (PLAYER_CHAT_WAIT*)
 * Color codes can be filtered from player names (FILTER_COLOR_NAMES)
 * New machine readable log file: ladderlog.txt
 * Configurable and better core dump/team kill/suicide distinction algorithm
 * Unknown settings are recognized now in configuration files
 * All global key bindings (Grab mouse, pause and texture reload were left)
   are now configurable
 * Player listing length limit adjusted so two teams/16 players fits on screen
 * Less verbose upgrade warning messages
 * Imposters can get a non-conflicting name (ALLOW_IMPOST[EO]RS)

#### Changes since 0.2.8_beta4:
 * Fixed bugs:
   - Windows version had a memory overwrite error causing crashes and weird behavior.
   - The game quit when the chat key and a key bound to an instant chat were pressed to quickly.
   - Votes expired too early on the client.
   - Performance: a lot of unused temporary walls were hanging around each round.
   - Cycles sometimes warped to odd locations.
 * Tweaked smart camera to not stay in front of the cycle so much.
   - Tweaked smart camera custom glancing.
     Report back whether you like or hate it!
 * Made custom camera even more configurable, changed default settings.
   (old ones are still in settings.cfg)
 * Forced glancing end on cycle turn now looks better.
 * FOV settings now deal better with wide screens/splitscreen mode.
 * Increased ping measurements again by adding the fluctuations. The raw 
   average ping caused the game logic to be too picky.

#### Changes since 0.2.8_beta3:
 * Famous fixed bugs: 
   - Random huge felt lag or blank screen (it was actually a problem with the timer) is gone.
   - Bogus "possible phase bug" message.
   - Seemingly random walls popping out of nothing, only to disappear a second later.
   - Random camera focus changes while you were still alive.
   - Every 100th console line got deleted.
 * Reduced pings! Actually, just the measurement is improved, the actual latency
   stays the same. I hope nobody notices that. Good thing nobody reads NEWS files :)
 * New text editing convenience: Home and end keys work, and CTRL+left/right/del/backspace
   work on whole words. Thanks, Nemo!
 * New instant chat functions: There are now 25 instant chat strings. If an instant
   chat string ends in a \, it will not be sent immediately, it's opened to be edited by you.
   Pressing an instant chat key while already chatting will insert the string, provided the
   key does not produce a regular character.
 * If enabled by the server administrator: control your cycle while you chat.
 * Turning speed of custom and internal camera can be adjusted with CAMERA_CUSTOM_TURN_SPEED
   and CAMERA_IN_TURN_SPEED.
 * The custom camera zooms out from a closeup at round start. Useful to see your position
   in your team. Configurable with CAMERA_CUSTOM_ZOOM.
 * Master server redundancy enabled. Thanks to Lucifer for hosting the backup master!

#### Changes since 0.2.8_beta2:
 * More bugfixes: most annoying was the crash/infinite loop a server got in when
   the master server was unreachable
 * Added zones to map specification
 * Reworked top level map format (sorry to the map makers for the inconvenience)
 * Windows: added start menu entries that open the system and user data directories
 * Bandwidth limitation now works. That means if you lowered the bandwidth
   limits in the network setup, they were not respected. You probably should
   readjust them.

#### Changes since 0.2.8_beta1:
 * bugfixes, of course
 * added recording and playback startmenu entries for Windows
 * abuse prevention: added IP banning, player number per IP and client limit, 
   doublebinding can be crippled on server command

#### Changes since 0.2.7.1:
 * Server side cycle synchronization issues fixed:
   - destination removal fixed
   - lag sliding caused by different interpretation of grinding turn commands on client and server fixed
   - niceness for older clients: sync interval decreased, no sync in front of wall to prevent local tunneling
 * Client side cycle synchronization issues fixed:
   - smooth corrections now only affect the appearance, the correct values are used internally
   - sync messages are no longer ignored,
   - new extrapolating sync method implemented,
   - ghosting fixed,
   - wall ends sticking out fixed ( for enemy cycles ),
   - the sophisticated distance based sync code implemented years ago is now actually used, oh dear.
 * Performance improvements:
   - netobject synchronization does no longer traverse all netobjects
   - disabled gameobject-gameobject interaction
 * added support for nonorthogonal driving directions
 * added support for debug recording and playback
 * switched to using automake

#### Changes since 0.2.7.0:
 * Fixed security issues: fake message ID crash, fake peer ID crash ( both
   maybe could be used to execute arbitrary code ), fake logout exploit, fake
   connections DOS attack
 * The feared "left grid" error message has been replaced by recovery code
 * Fixed transfer of infinite floats over the network
 * Fixed network timer inaccuracies on long running servers
 * Fixed rip bug ( for those who like it: search and you will find )
 * Fixed wall bending ( there is still some wall bending left, but it will be
   too small to notice )
 * Fixed tunneling bugs ( you were able to move from one side of a wall to the other )
 * Fixed bounce bug ( a small zigzag towards a wall sometimes put you further
   away from it, leaving a hole )
 * Fixed massive reference counting overflow crash caused by repeated close grinding/180ing
 * Made rubber code customizable and frame rate independent ( new CYCLE_RUBBER_
   variables in settings.cfg )
 * For debugging and scripting: dedicated server console messages can be
   decorated with client ID and IP
 * Topology police: facility to catch tunneling issues after they happened. off
   by default
 * Cycle network code is more robust
 * Update frequency of cycle position now configurable and more frequent for
   client's own cycle that the rest by default
 * Corrected rc.d scripts to start the server as a system service
 * Voting more configurable: voters can be made public, spectators can't vote
   any more by default
 * Moviepack floor color fixed
 * Hostname lookup failure fixed
 * "PLAYER_1 nick" console command problems fixed
 * Windows libraries moved out of the main source tree
 * Source code now compatible with 64 bit and gcc 3.4
 * Colored explosions!
 * HUD enhancements: customization, works in splitscreen mode
 * Directory scanning code added ( not yet visible, will be used for
    moviepack/cycle selection )
 * Moviepack sky texture support
 * Master server can now run on windows, too
 * Explosion sound now actually works

#### Changes since 0.2.6.0:
 * added HUD
 * Winzone may now be a deathzone

#### Changes since 0.2.5.2:
 * Split custom camera into server defined and client defined custom camera
 * Fixed monster chat message exploit
 * Added local player silencing
 * Added Kick voting
 * Added player names on cycles ( implemented by Christian Reitwiessner )
 * Harsher ( and configurable ) spam protection
 * Fixed strangely bent walls

#### Changes since 0.2.5.1:
 * Fixed fullscreen toggle while chatting
 * Disabled trilinear filtering for ATI cards by default ( to avoid lockups )
 * Worked around flawy S3 texture storage
 * Fixed crash when trying to change teams in spectator mode

#### Changes since 0.2.5:
 * Another go at the "11th Round Bug"
 * Player list is now sorted after displayed score ( before: some invisible combination of team score and own score )
 * Fixed brakes when connecting to a 0.2.4 or earlier server
 * Updated FAQ with server setting hints
 * Unified Windows and Unix network code
 * Fixed texture hardware mode
 * Less connection losses
 * Fixed network ID related crash
 * Added player names to information you get from a server
 * Added window-fullscreen toggle key and separate setting of screen resolution/window size

#### Changes since 0.2.4:
 * Fixed online observer mode switching
 * Fixed command line options
 * Fixed player quit message
 * Fixed AI join message
 * Fixed double registration of some network objects ( had no known visible symptom )
 * Added security checks to see if Armagetron is run correctly
 * Fixed big brother ( reporting of OS, graphic card and version to the master server )
 * Disabled memory manager by default
 * Fixed single player exit game crash
 * Fixed zero length sting receive crash
 * Completed network version control
 * Added timestamps to logfiles ( code fragments by k_at_work )
 * Added reason to login deny/kick messages
 * Fixed 11th round bug ( additional round started after match end )
 * Added --doc command line switch to print offline version of online help
 * Fixed sign of rise/drop highscore messages
 * New console commands: quit/exit ( dedicated server only ), kick <player name>, center_message <message> and console_message <message>
 * New setting item: MAX_CLIENTS
 * Improved server browser, less skipping around
 * Added instant win zone
 * Cycle brake is now of limited duration

#### Changes since 0.2.3:
 * Fixed score display ( really! )
 * Fixed freestyle mode round counting
 * Fixed server freezer
 * Fixed explosion sync
 * Fixed AI crashes when player leaves in mid-battle
 * Fixed configure script: now really uses sdl-config
 * Speeded up deletion of unreachable servers
 * Fixed endless recursion bug ( "11th player bug" )
 * Moved team start positions further apart
 * Player name is now taken from $USER on first start and player color is randomized

#### Changes since 0.2.2:
 * Fixed highscores and ladder
 * Delayed score deletion after match end
 * Fixed autoexec.cfg loading
 * More automatic rebuilding
 * Fixed dedicated server idle quitting
 * Fixed auto AI
 * Fixed instant chat
 * Fixed score display for many players or long player names
 * Fixed very rare client crash
 * Fixed not so rare release only crash
 * Fixed balance-related typos
 * Tweaked smart camera so that it works well when braking
 * Fixed finish mode game setting

#### Changes since 0.2.1:
 * Fixed another connection-loss bug
 * Fixed observer freeze
 * Observers now don't just get kicked when nobody else is online
 * Fixed software renderer detection
 * Updated FAQ
 * Fixed config file loading on some architectures
 * Improved team coloring
 * Fixed stuck brake bug

#### Changes since 0.2.0:
 * Fixed crash when watching a quitting player

#### Changes since 0.2.pre4:
 * Fixed crashes
 * Fixed windows network connection problems
 * Fixed clour related problems
 * Added marker for own cycle to avoid confusion in team mode
 * Changed directory placement: configuration resides in /etc/armagetron,
   user configuration in ~/.armagetron/var
 * Data files added to ~/.armagetron/textures ( or /models or /sounds )
   are now loaded in favor of original data, allowing user specific mods

#### Changes since 0.2.pre3:
 * Fixed some more rare crashes and lockups
 * Fixed team handling: all team change operations now get only active
   on next round address
 * Fixed moviesounds
 * Fixed server-master server communications
 * new installation method: make update ( keeps user configuration )

#### Changes since 0.2.pre2:
 * Fixed some rare crashes and lockups
 * Improved cycle handling over network
 * It is now allowed to fight with an arbitrary large team against the AI
   regardless of the balancing settings
 * improved rendering precision: z-fighting artefact are now uncommon,
   and at the same time early clipping on the near clipping plane

#### Changes since 0.2.pre:
 * got rid of "zombie" players
 * corrected team messages
 * fixed chatting
 * fixed 32 bit rendering

#### Changes since 0.1.4.9:
 * it is now possible for players to form teams
 * explosion now can blow holes in player walls
 * delay added to wall disappearance after death
 * finite wall length supported for snake-style gameplay
 * arena size and game speed configurable in the menu
 * the AI has been greatly improved
 * there is an Internet master server active
 * support for localization has been added, currently featuring 
   English and German
 * AI players now have names
 * the moviepack title screen is displayed
 * Linux version: easier system-wide installation with RPM packages 
   or "make install"

