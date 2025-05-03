class CLINTON_RoboPlayerManager
{
	protected FactionManager fm;
	protected SCR_LoadoutManager lm;
	
	protected int m_iAmount;
	protected int m_iFactionListSize;
	
	ref protected static set<string> 								   m_aFactionsList = new set<string>();
	ref protected static array<ref CLINTON_Virtual_Player> 			   m_aBots 		   = {};
	ref protected static map<string, ref array<SCR_BasePlayerLoadout>> loadouts 	   = new map<string, ref array<SCR_BasePlayerLoadout>>();
	
	ref protected SCR_BasePlayerLoadout 			  defaultUSLoadout = null;
	ref protected map<string, ref array<SCR_AIGroup>> m_aGroups        = new map<string, ref array<SCR_AIGroup>>();
	
	protected static CLINTON_RoboPlayerManager s_Instance;
	
	void CLINTON_RoboPlayerManager(notnull World world)
	{
		if (s_Instance)
		{
			Print("Only one instance of CLINTON_RoboPlayerManager is allowed in the world!", LogLevel.WARNING);
			delete this;
			return;
		}

		s_Instance = this;

		// rest of the init code
		
		m_iAmount = 0;
		m_aBots = {};
		
		edit_world(world);
		discover_loadouts(world);
	}
	
	void ~CLINTON_RoboPlayerManager(){
		// m_iAmount = null;
		// m_aBots = null;
		
		// avoid using delete on refs if this is not an entity that survives after being run
	}
	
	static CLINTON_RoboPlayerManager getInstance(){ return s_Instance;}
	
	void edit_world(World world)
	{
		fm = FactionManager.Cast(     world.FindEntityByName("FactionManager"));
		lm = SCR_LoadoutManager.Cast( world.FindEntityByName("LoadoutManager"));
		
		list_playable_factions();
		check_for_ai_world(world);
				
		return;
	}
	
	void list_playable_factions()
	{
		if( fm )
		{
			ref array<Faction> tempFactionsList = {};
			m_iFactionListSize = fm.GetFactionsList(tempFactionsList);
			foreach( Faction faction : tempFactionsList)
			{
				if( SCR_Faction.Cast(faction).IsPlayable() )
				{
					m_aFactionsList.Insert(faction.GetFactionKey());
				}
			}
		} 
		else
		{
			m_iFactionListSize = 1;
            Print("This scenario is missing a FactionManager named `FactionManager`!", LogLevel.ERROR);
            return;
		}
	}
	
	void check_for_ai_world(World world)
	{
		// check for AIWorld
		SCR_AIWorld aiw = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		
		// TODO: Also check for Eden and Alrand
		
		if( !aiw )
		{
			// place
            Print("Couldn't find AIWorld, Navmesh maybe missing.", LogLevel.WARNING);
			ref Resource AIWorld_Ent = Resource.Load("{E0A05C76552E7F58}Prefabs/AI/SCR_AIWorld.et");
			
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform[3] = "0 0 0";
			GetGame().SpawnEntityPrefab(
					AIWorld_Ent,
					world,
					spawnParams
			);
		}
		return;
	}
	
	void add_bots_on_each_team(int quantity, int group_setting = 0)
	{
		foreach(string faction : m_aFactionsList)
		{
			for(int i = 0; i < quantity; i++)
			{
				m_aBots.Insert(new CLINTON_Virtual_Player(faction, group_setting));
				
			}
		}
	}
	
	void add_bots_on_faction(string faction, int quantity, int group_setting = 0)
	{
		for(int i = 0; i < quantity; i++)
		{
			m_aBots.Insert(new CLINTON_Virtual_Player(faction, group_setting));
			//UpdatePlayerList(m_aBots.Count());
		}
	}
	
	// I can't figure out adding menu entries here, unlike in removing
	// Also I'm sending back the characters name
	string add_bot(string faction, int group_setting = 0)
	{
		CLINTON_Virtual_Player b = new CLINTON_Virtual_Player(faction, group_setting);
		m_aBots.Insert(b);
		return b.GetVirtualPlayerName();
	}
	
	array<int> remove_bots_on_each_team(int quantity, array<ref SCR_PlayerListEntry> widgets_representing_bot)
	{
		array<int> removed_bots = {};
		
		map<string, int> each_faction_count = new map<string,int>();  // Sorry Computational Complexity
		
		foreach(string faction : m_aFactionsList)
		{
			each_faction_count.Set(faction,0);
		}
		// count = num_faction * quantity
		// while count > zero
		int total_count = m_iFactionListSize * quantity;
		int bot_list_iterator = 0;
		int bot_list_limit = m_aBots.Count();
		string bot_faction;
		// Maybe re inmplement where you build a list of indexes then recreate the
		// list excluding those items.
		while( total_count > 0)
		{
			// terminate on end of bot list
			if( bot_list_iterator == bot_list_limit )
			{
				// We can let the user click the button with a high number
				break;
			}
			bot_faction = m_aBots[bot_list_iterator].GetFactionKey();
			// check the current bots faction
			if( each_faction_count.Get(bot_faction) < quantity )
			{
				each_faction_count.Set(bot_faction, each_faction_count.Get(bot_faction) +1 ); 
				m_aBots.RemoveOrdered(bot_list_iterator);  // hsould this be ordered?
				widgets_representing_bot[bot_list_iterator].m_wRow.RemoveFromHierarchy();
				widgets_representing_bot.RemoveOrdered(bot_list_iterator);
				// won't work when sorted
				
				
				// Reverse engineer the indexes
				removed_bots.Insert(bot_list_iterator + ((m_iFactionListSize * quantity)) - total_count);
				
				bot_list_limit = bot_list_limit -1; // Do not move iterator forward
				total_count = total_count -1;
				
			} else {
				bot_list_iterator = bot_list_iterator +1;
			}
		}
		return removed_bots;
	}
	
	
	array<int> remove_bots_on_faction(string faction, int quantity, array<ref SCR_PlayerListEntry> widgets_representing_bot)
	{
		array<int> removed_bots = {};
		
		int total_count = quantity;
		int bot_list_iterator = 0;
		int bot_list_limit = m_aBots.Count();
		string bot_faction;
		while( total_count > 0)
		{
			// terminate on end of bot list
			if( bot_list_iterator == bot_list_limit )
			{
				// We can let the user click the button with a high number
				break;
			}
			bot_faction = m_aBots[bot_list_iterator].GetFactionKey();
			// check the current bots faction
			if( bot_faction == faction )
			{
				 
				m_aBots.RemoveOrdered(bot_list_iterator);  // hsould this be ordered?
				widgets_representing_bot[bot_list_iterator].m_wRow.RemoveFromHierarchy();
				widgets_representing_bot.RemoveOrdered(bot_list_iterator);
				// won't work when sorted
				//m_aBots.Remove(bot_list_iterator);
				
				// Reverse engineer the indexes
				removed_bots.Insert(bot_list_iterator + (quantity - total_count) );
				
				bot_list_limit = bot_list_limit -1; // Do not move iterator forward
				total_count = total_count -1;
				
			} else {
				bot_list_iterator = bot_list_iterator +1;
			}
		}
		return removed_bots;
	}
	
	// Click burger on the right side of the layout
	void remove_specific_bot(int index)
	{
		m_aBots.Remove(index);
	}
	
	void check_respawns()
	{
		if(!m_aBots) return;
		
		bool missing 	  = false;
		bool player_state = false;
		
		foreach(CLINTON_Virtual_Player player : m_aBots)
		{
			if( player.GetCharacterAlive() == false )
			{
				Print("Spawning...",LogLevel.DEBUG);
				// spawn
				player_state = player.spawnIn(this.GetGroups());
				if ( !player_state ) missing = true;
				player.SetCharacterAlive(player_state);
				
				// check_deaths(player);
				
			}
		}
		if (missing) GetGame().GetCallqueue().CallLater(check_respawns,2048,false);
	}
	
	/*
	void check_deaths(CLINTON_Virtual_Player persun)
	{
		SCR_CharacterControllerComponent what_happended_to_my_good_controller = SCR_CharacterControllerComponent
		.Cast(ChimeraCharacter
			.Cast(persun.GetCurrentCharacter())
			.GetCharacterController()
		);
		what_happended_to_my_good_controller.m_OnPlayerDeath.Insert(persun.inoffencive_name);
	}
	
	bool Create_Groups()
	{
		// SCR_AIGroup GetFirstNotFullForFaction(notnull Faction faction, SCR_AIGroup ownGroup = null, bool respectPrivate = false)
		// SCR_GroupsManagerComponent.
		
		SCR_SpawnPoint sp =  SCR_SpawnPoint.GetRandomSpawnPointForFaction("US");
		if( !sp ) return false;
				
		EntitySpawnParams spawnParams  = new EntitySpawnParams();
		spawnParams.TransformMode      = ETransformMode.WORLD;
		spawnParams.Transform[3]       = IEntity.Cast(sp).GetOrigin();
		
		ref Resource emptGroupResource = Resource.Load("{7DBFFCF40E113B9F}Prefabs/CLINTON_Group_Base.et");
		ref IEntity emptGroupEnt       = GetGame().SpawnEntityPrefab(emptGroupResource, GetGame().GetWorld(), spawnParams);
		ref SCR_AIGroup emptGroup      = SCR_AIGroup.Cast(emptGroupEnt);
		
		// TODO: More than just hard coding
		emptGroup.SetFaction(fm.GetFactionByKey("US"));
		
		ref SCR_GroupsManagerComponent gm = SCR_GroupsManagerComponent.GetInstance();
		if (!gm) return false;
		
		ref SCR_ChimeraCharacter user = null;
		
		bool term = false;
		int i = 0;
		while(!term)
		{
			if(m_aBots[i].GetFactionKey() == "US")
			{
				term = true;  // need the ChimeraCharacter for later
				user = SCR_ChimeraCharacter.Cast(m_aBots[i].GetCurrentCharacter());
			}
			i = i + 1;
		}
		
		emptGroup.SetCanDeleteIfNoPlayer(false);
		
		emptGroup.AddAIEntityToGroup(SCR_ChimeraCharacter.Cast(m_aBots[i-1].GetCurrentCharacter()));  // This
		ref AIWaypoint waypoint = AIWaypoint.Cast(GetGame().GetWorld().FindEntityByName("MoveC"));
		if( !waypoint )
		{
			Print("MAMA MAMA NO WAYPOINTMAMA, WAAAAAAAH",  LogLevel.ERROR);
		}
		emptGroup.AddWaypointToGroup(waypoint);
		// This is what is working
		return true;
	}
	*/
		
	void discover_loadouts(notnull World world)
	{
		array<ref SCR_BasePlayerLoadout> bpl = lm.GetPlayerLoadouts();
		// TODO: find the default loadout in the array
		defaultUSLoadout = bpl[0];
		string current_key = "";
		array<SCR_BasePlayerLoadout> current_loadout_array = {};
		
		foreach(SCR_BasePlayerLoadout loadout : bpl)
		{
			current_key = SCR_FactionPlayerLoadout.Cast(loadout).GetFactionKey();
			if( m_aFactionsList.Contains(current_key) )
			{
				current_loadout_array = loadouts.Get(current_key);
				if(current_loadout_array == null) current_loadout_array = {};
				current_loadout_array.Insert(loadout);
				loadouts.Set(current_key, current_loadout_array);
			}
		}
	}
	
	ref static map<string, ref array<SCR_BasePlayerLoadout>> GetLoadouts()
	{
		return loadouts;
	}
	
	ref map<string, ref array<SCR_AIGroup>> GetGroups()
	{
		return this.m_aGroups;
	}
	
	void SetGroups( string key, array<SCR_AIGroup> val )
	{
		this.m_aGroups.Set(key, val);
	}
	
	string GetVirtualPlayerName(int id)
	{
		return "Bot_" + id.ToString();
	}
	
	string GetVirtualPlayerFactionKey(int id)
	{
		return m_aBots[id].GetFactionKey();
	}
	
	static ref CLINTON_Virtual_Player GetPlayer(int id)
	{
		if(id >= m_aBots.Count()) return null;
		return m_aBots[id];
	}
	
	static ref array<ref CLINTON_Virtual_Player> GetPlayers()
	{
		return m_aBots;
	}
	
	int GetFactionListSize()
	{
		if( !m_iFactionListSize )
		{
			Print("Faction list size not ready!", LogLevel.ERROR);
		}
		return m_iFactionListSize;
	}
}


class CLINTON_Virtual_Player
{
	protected ref ScriptInvokerVoid m_OnCharacterDeath;
	
	protected bool m_bCharacterAlive;
	protected IEntity m_cCurrentCharacter;
	protected string  m_sFactionKey;
	protected int m_iGroupSetting;
	
	float m_fSpawnDelay;
	
	CLINTON_Virtual_Player CLINTON_Virtual_Player(string factionKey = "US", int group_setting = 0)
	{
		/* singleton code
		
		if (s_Instance)
		{
			Print("Only one instance of CLINTON_Virtual_Player is allowed in the world!", LogLevel.WARNING);
			// delete this;
			// return null;
			return this;
		}
		*/


		// rest of the init code
		
		m_bCharacterAlive = false;
		m_fSpawnDelay = 0.0;
		m_cCurrentCharacter = null;
		m_sFactionKey = factionKey;
		m_iGroupSetting = group_setting;  // TODO: Implement group creating settings
		// a bots id is just their position in an array storing them
		
		return this;
	}
	
	void ~CLINTON_Virtual_Player()
	{
		if(m_cCurrentCharacter)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(m_cCurrentCharacter);
		}
		
		delete m_OnCharacterDeath;
	}
	
	// Now avoiding using the first characters identity to avoid UI complications
	string GetVirtualPlayerName()
	{
			ChimeraCharacter char = ChimeraCharacter.Cast(GetCurrentCharacter());
			if(!char)
			{
				 return "[Character not dressed yet!]";
			}
			SCR_CharacterIdentityComponent idComp = SCR_CharacterIdentityComponent.Cast(
				char.FindComponent(SCR_CharacterIdentityComponent));
			string format = "";
			string name = "";
			string alias = "";
			string surname = "";
			idComp.GetFormattedFullName(format,name,alias,surname);
			return name + surname;
	}
		
	ScriptInvokerVoid inoffencive_name()
	{
		// https://community.bistudio.com/wiki/Arma_Reforger:ScriptInvoker_Usage
		if (!m_OnCharacterDeath)
		{
			m_OnCharacterDeath = new ScriptInvokerVoid();
		}
		SetCharacterAlive(false);
		// oh they humanity
		
		return m_OnCharacterDeath;
	}
	
	void check_deaths()
	{
		ref ChimeraCharacter chara = ChimeraCharacter.Cast(GetCurrentCharacter());
		ref CharacterControllerComponent contrl = chara.GetCharacterController();
		ref SCR_CharacterControllerComponent what_happended_to_my_good_controller = SCR_CharacterControllerComponent
		.Cast(contrl
		);  // might be the controller being the group and not the individual
		what_happended_to_my_good_controller.m_OnPlayerDeath.Insert(inoffencive_name);
	}
	
	bool GetCharacterAlive()
	{
		return m_bCharacterAlive;
	}
	
	void SetCharacterAlive(bool state)
	{
		m_bCharacterAlive = state;
	}
	
	void SetCurrentCharacter(IEntity character)
	{
		m_cCurrentCharacter = character;
	}
	
	IEntity GetCurrentCharacter()
	{
		return m_cCurrentCharacter;
	}
	
	string GetFactionKey()
	{
		return m_sFactionKey;
	}
	
	bool spawnIn(map<string, ref array<SCR_AIGroup>> groups)
	{
		// TODO: Decisions for which spawnpoint to pick
		SCR_SpawnPoint sp =  SCR_SpawnPoint.GetRandomSpawnPointForFaction(this.GetFactionKey());
		World world = GetGame().GetWorld();
		// TODO: Logic for when there are no spawns atm
		if( !sp ) return false;
		
		// TODO: Decisions for which loadout to use
		Resource reso = Resource.Load(CLINTON_RoboPlayerManager.GetLoadouts()
			.Get(this.GetFactionKey())[0]
			.GetLoadoutResource());
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = IEntity.Cast(sp).GetOrigin();
		this.SetCurrentCharacter(GetGame().SpawnEntityPrefab(
			reso,
			world,
			spawnParams
		));
		check_deaths();
		
		
		
		SCR_BaseGameMode 			   bgm = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		ref SCR_GroupsManagerComponent gmc = SCR_GroupsManagerComponent.GetInstance();
		FactionManager 				   fm  = FactionManager.Cast(world.FindEntityByName("FactionManager"));
		if(!gmc)
		{
			Print("Missing Group Manager Component",  LogLevel.WARNING);
			return false;
		}
		
		// map<string, ref array<SCR_AIGroup>> gropus = CLINTON_RoboPlayerManager.GetGroups();
		// array<SCR_AIGroup> faction_groups = groups.Get(this.GetFactionKey());
		
		// faction_groups is not being mutated
		
		ref array<SCR_AIGroup> faction_groups = groups.Get(this.GetFactionKey());
		ref SCR_AIGroup        current_group;
		bool terminate = false;
		
		if(faction_groups)
		{
			// check for fullness
			int i = 0;
			while( !terminate 	&& i < faction_groups.GetSizeOf() )
			{
				current_group = faction_groups[i];
				if( current_group && !current_group.IsFull() )
				{  
					current_group.AddAIEntityToGroup(SCR_ChimeraCharacter.Cast(this.GetCurrentCharacter()));
					faction_groups.Set(i, current_group);
					groups.Set(this.GetFactionKey(), faction_groups);
					terminate = true;
				}
				// Ending the entire group's characters somehow sets the group to null
				// current_group.AddAgent(AIAgent.Cast(vPlayer.GetCurrentCharacter()));
			}
		} else
		{
			// no groups yet in faction
			faction_groups = new array<SCR_AIGroup>();
		}
		
		// All groups were full
		if( !terminate )
		{
			// spawnParams  = new EntitySpawnParams();
			// spawnParams.TransformMode      = ETransformMode.WORLD;
			// spawnParams.Transform[3]       = IEntity.Cast(sp).GetOrigin();
			
			ref Resource emptGroupResource = Resource.Load("{89CE895CA0AE72DF}Prefabs/CLINTON_Group_Base_Again.et");
			ref IEntity emptGroupEnt       = GetGame().SpawnEntityPrefab(emptGroupResource, world, spawnParams);
			ref SCR_AIGroup emptGroup      = SCR_AIGroup.Cast(emptGroupEnt);
			
			emptGroup.SetFaction(fm.GetFactionByKey(this.GetFactionKey()));
			// emptGroup.SetCanDeleteIfNoPlayer(false);
			// emptGroup.SetDeleteWhenEmpty(true);
			
			// TODO: Waypoints made from tasks
			//ref AIWaypoint waypoint = AIWaypoint.Cast(world.FindEntityByName("MoveC"));
			
			// ref Resource soos = Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et");
			
			// EntitySpawnParams spawnParams2  = new EntitySpawnParams();
			// spawnParams2.TransformMode      = ETransformMode.WORLD;
			// spawnParams2.Transform[3]       = IEntity.Cast(waypoint).GetOrigin();
			
			// AIWaypoint new_move = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(soos, null, spawnParams2));
			// emptGroup.AddWaypointToGroup(new_move);
			//emptGroup.AddWaypoint(waypoint);
			
			//if( !waypoint )
			//{
			//	Print("No waypoints.",  LogLevel.ERROR);
			//}
			
			emptGroup.AddAIEntityToGroup(SCR_ChimeraCharacter.Cast(this.GetCurrentCharacter()));
			faction_groups.Insert(emptGroup);  // null reference?
			groups.Set(this.GetFactionKey(), faction_groups);
			
			CLINTON_BotWaypointManagerEntity.getInstance().update_group(emptGroup, this.GetFactionKey());
		} else {
			CLINTON_BotWaypointManagerEntity.getInstance().update_group(current_group, this.GetFactionKey());
		}
		return true;
	}
	
	void update_groups_waypoints(SCR_AIGroup emptGroup)
	{
		array<AIWaypoint> outWaypoints = new array<AIWaypoint>();
		int waypoint_count = emptGroup.GetWaypoints(outWaypoints);
		
		
	}
}