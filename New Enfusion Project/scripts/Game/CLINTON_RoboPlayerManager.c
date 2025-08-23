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
	// a ref to a null object signifies a released/free compartment
	protected ref map<AIAgent,ref BaseCompartmentSlot> m_mAgentsReservedCompartment;
	
	protected static CLINTON_RoboPlayerManager s_Instance;
	protected static float standard_respawn_time;
	
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
		
		standard_respawn_time = 10;
		SCR_RespawnTimerComponent debug_me = SCR_RespawnTimerComponent.Cast(
		SCR_BaseGameMode.Cast(GetGame().GetGameMode()).FindComponent(SCR_RespawnTimerComponent));
		if( debug_me )
		{
			standard_respawn_time = debug_me.GetRespawnTime();
			if( standard_respawn_time == float.INFINITY )
			{
				standard_respawn_time = 10.0;
			}
		}
		
		m_mAgentsReservedCompartment = new map<AIAgent,ref BaseCompartmentSlot>();
	}
	
	void ~CLINTON_RoboPlayerManager(){
		// m_iAmount = null;
		// m_aBots = null;
		
		// avoid using delete on refs if this is not an entity that survives after being run
	}
	
	static CLINTON_RoboPlayerManager getInstance(){ return s_Instance;}
	
	void edit_world(World world)
	{
		// Incase someone places it by the workdesk
		if( world.IsEditMode()) return;
		
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
			if( player.GetCharacterAlive() == false && player.GetCharacterWaitingToRespawn() == false )
			{
				player.SetCharacterWaitingToRespawn(true);
				GetGame().GetCallqueue().CallLater(queue_respawn, standard_respawn_time*10.24, false, player);
			}
		}
	}
	
	void queue_respawn(CLINTON_Virtual_Player player)
	{
		Print("Spawning...",LogLevel.DEBUG);
		// spawn
		bool player_state = player.spawnIn(this.GetGroups());
		player.SetCharacterAlive(player_state);
		
		// check_deaths(player);
		if (!player_state)
		{
			GetGame().GetCallqueue().CallLater(queue_respawn,2048,false, player);
		} else {
			player.SetCharacterWaitingToRespawn(false);
		}
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
	
	static int GetPlayersCount()
	{
		return m_aBots.Count();
	}
	
	int GetFactionListSize()
	{
		if( !m_iFactionListSize )
		{
			Print("Faction list size not ready!", LogLevel.ERROR);
		}
		return m_iFactionListSize;
	}
	
	void SetPlayerSeat(SCR_ChimeraAIAgent c, BaseCompartmentSlot b)
	{
		m_mAgentsReservedCompartment.Set(c, b);
	}
	
	// When using the Carpool behavior, a player/agent will reserve a seat in a nearby vehicle
	bool QueryPlayerSeatingArangement(SCR_ChimeraAIAgent c)
	{
		BaseCompartmentSlot b = m_mAgentsReservedCompartment.Get(c);
		if (!b) return false;  // No reservation
		return true;
	}
}


class CLINTON_Virtual_Player
{
	protected ref ScriptInvokerVoid m_OnCharacterDeath;
	
	protected bool m_bCharacterAlive;
	protected IEntity m_cCurrentCharacter;
	protected string  m_sFactionKey;
	protected int m_iGroupSetting;
	protected bool m_bCharacterWaitingToRespawn;
	
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
		m_bCharacterWaitingToRespawn = false;
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
	
	bool GetCharacterWaitingToRespawn()
	{
		return m_bCharacterWaitingToRespawn;
	}
	
	void SetCharacterWaitingToRespawn(bool state)
	{
		m_bCharacterWaitingToRespawn = state;
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
	
	//------------------------------------------------------------------------------------------------
	bool spawnIn(map<string, ref array<SCR_AIGroup>> groups)
	{
		// TODO: Decisions for which spawnpoint to pick
		SCR_SpawnPoint sp =  SCR_SpawnPoint.GetRandomSpawnPointForFaction(this.GetFactionKey());
		World world = GetGame().GetWorld();
		// TODO: Logic for when there are no spawns atm
		if( !sp ) return false;
		
		//float spawnDelay = 10  //sp.GetRespawnTime();
		
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
		
		const protected float distance_cutoff = 500.00;
		
		if(m_iGroupSetting == 0)
		{  // Based on Distance
			protected vector sp_coords = IEntity.Cast(sp).GetOrigin();
			
			ref SCR_AIGroup closest_group = null;
			float closest_group_distance = 0.00;
			if(faction_groups)
			{
				foreach(SCR_AIGroup grp : faction_groups)
				{
					vector coords = grp.GetLeaderEntity().GetOrigin();
					float iterator_distance = vector.Distance(coords, sp_coords);
					if( !coords ) continue;
					if( !closest_group )
					{
						if( iterator_distance < distance_cutoff )
						{
							closest_group = grp;
							closest_group_distance = iterator_distance;
						}
						continue;
					}
					if( iterator_distance < distance_cutoff & iterator_distance < closest_group_distance)
					{
						closest_group = grp;
						closest_group_distance = iterator_distance;
					}
				}
				if( !closest_group )
				{
					// TODO: Make own group
					// no groups yet in faction
					faction_groups = new array<SCR_AIGroup>();
				}
				else
				{
					// TODO: Add npc to group
					closest_group.AddAIEntityToGroup(SCR_ChimeraCharacter.Cast(this.GetCurrentCharacter()));
					faction_groups.Insert(closest_group);
					groups.Set(this.GetFactionKey(), faction_groups);
					terminate = true;
				}
			}
			else
			{
				// no groups yet in faction
				faction_groups = new array<SCR_AIGroup>();
			}
		}
		
		if(m_iGroupSetting == 1)
		{  // Supposibly, Choose player groups first
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
					i++;
				}
			} 
			else
			{
				// no groups yet in faction
				faction_groups = new array<SCR_AIGroup>();
			}
		}
		
		if(m_iGroupSetting == 2)
		{  // Have fun coding the structured groups
			//SCR_AIGroup.SetMaxMembers();
			int desired_group_sizes = Math.Sqrt(CLINTON_RoboPlayerManager.GetPlayersCount());
			
			if(faction_groups)
			{
				// check for fullness
				int i = 0;
				while( !terminate && i < faction_groups.GetSizeOf() )
				{
					current_group = faction_groups[i];
					if( current_group && current_group.GetPlayerAndAgentCount() <= desired_group_sizes )
					{  
						current_group.AddAIEntityToGroup(SCR_ChimeraCharacter.Cast(this.GetCurrentCharacter()));
						faction_groups.Set(i, current_group);
						groups.Set(this.GetFactionKey(), faction_groups);
						terminate = true;
					}
					i++;
				}
			} 
			else
			{
				// no groups yet in faction
				faction_groups = new array<SCR_AIGroup>();
			}
		}
		
		// All groups were full
		if( !terminate )
		{
			// spawnParams  = new EntitySpawnParams();
			// spawnParams.TransformMode      = ETransformMode.WORLD;
			// spawnParams.Transform[3]       = IEntity.Cast(sp).GetOrigin();
			
			ref Resource emptGroupResource = Resource.Load("{395114220D070BE7}Prefabs/CLINTON_Group_Base_Three.et");
			ref IEntity emptGroupEnt       = GetGame().SpawnEntityPrefab(emptGroupResource, world, spawnParams);
			ref SCR_AIGroup emptGroup      = SCR_AIGroup.Cast(emptGroupEnt);
			
			emptGroup.SetFaction(fm.GetFactionByKey(this.GetFactionKey()));
			// emptGroup.SetCanDeleteIfNoPlayer(false);
			// emptGroup.SetDeleteWhenEmpty(true);
			
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