class CLINTON_VirtualPlayerManager
{
	protected FactionManager fm;
	protected SCR_LoadoutManager lm;
	
	ref protected static array<ref CLINTON_Virtual_Player> 			   m_aBots 		   = {};
	
	protected int m_iAmount;
	protected int m_iFactionListSize;
	
	ref protected static set<string> 								   m_aFactionsList = new set<string>();
	ref protected static map<string, ref array<SCR_BasePlayerLoadout>> loadouts 	   = new map<string, ref array<SCR_BasePlayerLoadout>>();
	ref protected SCR_BasePlayerLoadout 			  defaultUSLoadout = null;
	ref protected map<string, ref array<SCR_AIGroup>> m_aGroups        = new map<string, ref array<SCR_AIGroup>>();
	// a ref to a null object signifies a released/free compartment
	protected ref map<AIAgent,ref BaseCompartmentSlot> m_mAgentsReservedCompartment;
	
	protected static CLINTON_VirtualPlayerManager s_Instance;
	protected static float scenarioRespawnTime;
	
	void CLINTON_VirtualPlayerManager()
	{
		// if( GetGame().GetWorld().IsEditMode()) return;
		
		if (s_Instance)
		{
			Print("Only one instance of CLINTON_VirtualPlayerManager is allowed in the world!", LogLevel.WARNING);
			//delete this;
			return;
		}
		
		s_Instance = this;
		
		// rest of the init code
		
		m_iAmount = 0;
		m_aBots = {};
		
		World world = GetGame().GetWorld();
		if (!world) return;
		
		edit_world(world);
		discover_loadouts(world);
		
		SCR_RespawnTimerComponent respawnTimerComp = SCR_RespawnTimerComponent.Cast(
		SCR_BaseGameMode.Cast(GetGame().GetGameMode()).FindComponent(SCR_RespawnTimerComponent));
		if( respawnTimerComp )
		{
			scenarioRespawnTime = respawnTimerComp.GetRespawnTime();
			if( scenarioRespawnTime == float.INFINITY )
			{
				scenarioRespawnTime = 10.0;
			}
		}
		m_mAgentsReservedCompartment = new map<AIAgent,ref BaseCompartmentSlot>();
	}
	
	void ~CLINTON_VirtualPlayerManager()
	{
		// m_iAmount = null;
		// m_aBots = null;
	}
	
	static CLINTON_VirtualPlayerManager GetInstance(){ return s_Instance;}
	
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
	
	// Not currently in use
	void add_bots_on_faction(string faction, int quantity, int group_setting = 0, int loadout_setting = 0)
	{
		Print("authority-side code | add_bot");
		for(int i = 0; i < quantity; i++)
		{
			CLINTON_Virtual_Player newPlayer = new CLINTON_Virtual_Player();
			newPlayer.SetFactionKey(faction);
			newPlayer.SetGroupSettings(group_setting);
			newPlayer.SetLoadoutPreference(loadout_setting);
			m_aBots.Insert(newPlayer);
		}
		//Replication.BumpMe();
	}
	
	// Not currently in use
	void add_bots_on_each_team(int quantity, int group_setting = 0, int loadout_setting = 0)
	{
		Print("authority-side code");
		foreach(string faction : m_aFactionsList)
		{
			for(int i = 0; i < quantity; i++)
			{
				CLINTON_Virtual_Player newPlayer = new CLINTON_Virtual_Player();
				newPlayer.SetFactionKey(faction);
				newPlayer.SetGroupSettings(group_setting);
				newPlayer.SetLoadoutPreference(loadout_setting);
				m_aBots.Insert(newPlayer);
				
			}
		}
		//Replication.BumpMe();
	}
	
	void add_bot(string faction, int group_setting = 0, bool customNames = false, int loadout_setting = 0)
	{
		Print("authority-side code | add_bot");
		CLINTON_Virtual_Player newPlayer = new CLINTON_Virtual_Player();
		newPlayer.SetFactionKey(faction);
		newPlayer.SetGroupSettings(group_setting);
		newPlayer.SetLoadoutPreference(loadout_setting);
		m_aBots.Insert(newPlayer);
		// Replication.BumpMe();
	}
	
	void remove_bot(string faction)
	{
		Print("authority-side code | remove_bot");
		
		int i = m_aBots.Count() -1;
		
		while (i > -1 )
		{
			ref CLINTON_Virtual_Player player_i = m_aBots.Get(i);
			string i_factionKey = player_i.GetFactionKey();
			if (i_factionKey == faction)
			{
				m_aBots.RemoveOrdered(i);
				return;
			}
			i = i - 1;
		}
		return;
	}

	array<int> remove_bots_on_each_team(int quantity, array<ref SCR_BotPlayerListEntry> widgets_representing_bot)  // Retconned
	{
		array<int> removed_bots = {};
		
		map<string, int> each_faction_count = new map<string,int>();  // Sorry Computational Complexity
		
		foreach(string faction : m_aFactionsList)
		{
			each_faction_count.Set(faction,0);
		}
		
		int total_count = m_iFactionListSize * quantity;
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
			if( each_faction_count.Get(bot_faction) < quantity )
			{
				each_faction_count.Set(bot_faction, each_faction_count.Get(bot_faction) +1 ); 
				m_aBots.RemoveOrdered(bot_list_iterator);  // hsould this be ordered?
				widgets_representing_bot[bot_list_iterator].m_wRow.RemoveFromHierarchy();
				widgets_representing_bot.RemoveOrdered(bot_list_iterator);
				// Reverse engineer the indexes
				removed_bots.Insert(bot_list_iterator + ((m_iFactionListSize * quantity)) - total_count);
				
				bot_list_limit = bot_list_limit -1; // Do not move iterator forward
				total_count = total_count -1;
			} 
			else 
			{
				bot_list_iterator = bot_list_iterator +1;
			}
		}
		return removed_bots;
	}
	
	
	array<int> remove_bots_on_faction(string faction, int quantity, array<ref SCR_BotPlayerListEntry> widgets_representing_bot)  // Retconned
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
	
	bool rename_bot(int id, string name)
	{
		ref CLINTON_Virtual_Player p = GetPlayer(id);
		if (!p) return false;
		
		if (name == "") return false;
		p.SetPlayerName(name);
		
		return true;
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
				GetGame().GetCallqueue().CallLater(queue_respawn, scenarioRespawnTime*10.24, false, player);
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
	
	void delete_character_from_world(int id)
	{
		IEntity chara = IEntity.Cast(GetPlayer(id).GetCurrentCharacter());
		if (chara)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(chara);
			GetPlayer(id).inoffencive_name();
		} else {
			Print("Can not delete a character! 	| 	CLINTON_VirtualPlayerManager.c", LogLevel.ERROR);
		}
	}
	
	/*  Not in use
	void check_deaths(CLINTON_Virtual_Player persun)
	{
		SCR_CharacterControllerComponent what_happended_to_my_good_controller = SCR_CharacterControllerComponent
		.Cast(ChimeraCharacter
			.Cast(persun.GetCurrentCharacter())
			.GetCharacterController()
		);
		what_happended_to_my_good_controller.m_OnPlayerDeath.Insert(persun.inoffencive_name);
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
	
	ref array<SCR_AIGroup> GetGroups(Faction f)
	{
		return this.m_aGroups.Get(f.GetFactionKey());
	}
	
	void SetGroups( string key, array<SCR_AIGroup> val )
	{
		this.m_aGroups.Set(key, val);
	}
	
	string GetVirtualPlayerName(int id)
	{
		return GetPlayer(id).GetPlayerName();
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
	
	// Networking Stuff
		
	// IDK what this is, I just copied the 6th Modding Bootcamp
	static bool Extract(CLINTON_VirtualPlayerManager instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		bool tern = true;
		foreach (CLINTON_Virtual_Player b : instance.m_aBots)
		{
			tern = tern && CLINTON_Virtual_Player.Extract(b, ctx, snapshot);
		}
		return tern;
	}
	
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, CLINTON_VirtualPlayerManager instance)
	{
		bool tern = true;
		foreach (CLINTON_Virtual_Player b : instance.m_aBots)
		{
			tern = tern && CLINTON_Virtual_Player.Inject(snapshot, ctx, b);
		}
		return tern;
	}
	
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		CLINTON_Virtual_Player.Encode(snapshot, ctx, packet);
	}
	
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		return CLINTON_Virtual_Player.Decode(packet, ctx, snapshot);
	}
	
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		return true;
		return CLINTON_Virtual_Player.SnapCompare(lhs, rhs, ctx);
	}
	
	static bool PropCompare(CLINTON_VirtualPlayerManager instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return true;
		bool tern = true;
		// int length = 0; Maybe different lengths will cause PropCompare to return false?
		foreach (CLINTON_Virtual_Player b : instance.m_aBots)
		{
			tern = tern && CLINTON_Virtual_Player.PropCompare(b, snapshot, ctx);  // Check video for whatr should return false and modify/inject data
			// length = length + 1;
		}
		
		return tern;
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
	protected string m_sCharacterName;
	protected string m_sFirstName;
	protected string m_sAlias;
	protected string m_sSurname;
	protected bool m_bUseCustomNames;
	protected int m_iLoadout;
	protected int m_iLoadoutPreference;
	
	float m_fSpawnDelay;
	
	// string factionKey = "US", int group_setting = 0, bool useCustomNames = false
	CLINTON_Virtual_Player CLINTON_Virtual_Player()
	{		
		// A bot's Id is just their position in an array storing them
		m_bCharacterAlive = false;
		m_bCharacterWaitingToRespawn = false;
		m_fSpawnDelay = 0.0;
		m_cCurrentCharacter = null;
		m_sFactionKey    = "US";
		m_iGroupSetting = 0;  // TODO: Implement group creating settings
		m_sCharacterName = "";
		m_sFirstName	 = "";
		m_sAlias 		     = "";
		m_sSurname 	 = "";
		m_bUseCustomNames = false;
		m_iLoadoutPreference = 0;
		m_iLoadout = -1;
		
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
		
	ScriptInvokerVoid inoffencive_name()
	{
		// https://community.bistudio.com/wiki/Arma_Reforger:ScriptInvoker_Usage
		if (!m_OnCharacterDeath)
		{
			m_OnCharacterDeath = new ScriptInvokerVoid();
		}
		// oh they humanity
		SetCharacterAlive(false);
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
	
	void SetFactionKey(string k)
	{
		m_sFactionKey = k;
	}
	
	//------------------------------------------------------------------------------------------------
	bool spawnIn(map<string, ref array<SCR_AIGroup>> groups)
	{
		// TODO: Decisions for which spawnpoint to pick
		SCR_SpawnPoint sp =  SCR_SpawnPoint.GetRandomSpawnPointForFaction(this.GetFactionKey());
		World world = GetGame().GetWorld();
		// TODO: Logic for when there are no spawns atm
		if( !sp ) return false;
		
		Print("Here is the script", LogLevel.ERROR);
		int debug_me = this.GetLoadoutPreference();
		PrintFormat("Preference is %1", debug_me);  // This is not printing! ####
		// Set loadout number in the class so that the UI can find it easily
		switch(this.GetLoadoutPreference()) 
		{
			// Even Loadouts
			case 0:
			{
				Print("Case Zero", LogLevel.ERROR);
				// array<SCR_AIGroup> team = groups.Get(this.GetFactionKey());
				array<ref CLINTON_Virtual_Player> players = CLINTON_VirtualPlayerManager.GetInstance().GetPlayers();
				Print("One", LogLevel.ERROR);
				if (!players)
				{
					Print("Error in the spawnIn() logic 	| 	CLINON_VirtualPlayerManager.c", LogLevel.ERROR);
					SetLoadoutIndex(0);  // Continue
					break;
				}
				Print("Two", LogLevel.ERROR);
				int size = CLINTON_VirtualPlayerManager.GetLoadouts().Get(GetFactionKey()).Count();
				PrintFormat("Size is %1", size);
				Print("Three", LogLevel.ERROR);
				if (!size)	
				{
					Print("No loadouts found! 	| 	CLINTON_VirtualPlayerManager.c", LogLevel.ERROR);
					SetLoadoutIndex(0);  // Continue
					break;
				}
				Print("Four", LogLevel.ERROR);
				array<int> loadout_count = new array<int>();  // new array<int>;  // Make basic array of [0] *x then work from there
				int i = 0;
				while (i < size)  // ik this reads like babies first program but I like knowing the code won't cause slow downs
				{
					loadout_count.Insert(0);
					i = i + 1;
				}
				
				int loadout_index;
				
				PrintFormat("The faction key is %1", this.GetFactionKey());
				
				foreach( CLINTON_Virtual_Player p : players) // Find what is causing this to not end! ####
				{
					PrintFormat("The current players key is %1", p.GetFactionKey());
					if (p.GetFactionKey() != this.GetFactionKey())
					{
						continue;
					}
					loadout_index = p.GetLoadoutIndex();
					if( loadout_index > -1)  // If (val) is failing when the index is 0
					{
						loadout_count.Set(loadout_index, loadout_count.Get(loadout_index)+1);  // Sorry readability
						PrintFormat("The loadout %1 has a population of %2", loadout_index, loadout_count.Get(loadout_index));
					}
				}
				int lowest_val = loadout_count.Get(0);
				int lowest_index = 0;
				i = 1;
				
				while (i < size)
				{
					if (lowest_val > loadout_count.Get(i))
					{
						lowest_val = loadout_count.Get(i);
						lowest_index = i;
					}
					i = i + 1;
				}
				
				PrintFormat("The lowest value is %1", lowest_val);
				PrintFormat("The key is %1", lowest_index);
				this.SetLoadoutIndex(lowest_index);
				break;
			}  // Yes this is all case zero
			
			// Random on every respawn
			case 1:
			{
				int i = Math.RandomInt(1,100) % CLINTON_VirtualPlayerManager.GetLoadouts().Count();
				this.SetLoadoutIndex(i);
				break;
			}
			
			// Random but decided only once
			case 2:
			{
				int i = this.GetLoadoutIndex();
				if( i == -1)
				{
					i = Math.RandomInt(1,100) % CLINTON_VirtualPlayerManager.GetLoadouts().Count() ;
					this.SetLoadoutIndex(i);
				}
				break;
			}
		}
		
		Resource reso = Resource.Load(CLINTON_VirtualPlayerManager.GetLoadouts()
			.Get(this.GetFactionKey())[GetLoadoutIndex()]
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
		
		// Let's give them a name
		ChimeraCharacter char;
		if(m_sCharacterName == "")
		{
			if(m_bUseCustomNames != true)
			{
				char = ChimeraCharacter.Cast(GetCurrentCharacter());
				if(!char)
				{
					GetGame().GetCallqueue().CallLater(delayedSetPlayerName, 1024, false);
					return false;
				}
				
				SCR_CharacterIdentityComponent idComp = SCR_CharacterIdentityComponent.Cast(
					char.FindComponent(SCR_CharacterIdentityComponent));
				if(!idComp)
				{
					GetGame().GetCallqueue().CallLater(delayedSetPlayerName, 1024, false);
					return false;
				}
				string format = "";
				string name = "";
				string alias = "";
				string surname = "";
				idComp.GetFormattedFullName(format,name,alias,surname);
				
				string preferedName;
				string form = WidgetManager.Translate(format);  // Working out that localisations exist took forever
				if (form != WidgetManager.Translate("#AR-Idenity_Name_Format_Full"))
				{  // If no alias
					preferedName = string.Format("%1 %2", WidgetManager.Translate(name), WidgetManager.Translate(surname));  // There's probably a nicer way to write this
				} else {
					preferedName = string.Format("%1 %2 %3", WidgetManager.Translate(name), WidgetManager.Translate(alias), WidgetManager.Translate(surname));  // Do alias's need ' marks?
				}
				SetPlayerName( preferedName );
			}
		}
		char = ChimeraCharacter.Cast(GetCurrentCharacter());
		// We need to override each characters name
		SCR_CharacterIdentityComponent idComp = SCR_CharacterIdentityComponent.Cast(
					char.FindComponent(SCR_CharacterIdentityComponent));
		ref Identity ident = idComp.GetIdentity();  // maybe a reference works better?
		ident.SetName(m_sCharacterName);
		ident.SetAlias("");
		ident.SetSurname("");
		// idComp.SetIdentity(i); i is a ref
		
		SCR_BaseGameMode 			   bgm = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		ref SCR_GroupsManagerComponent gmc = SCR_GroupsManagerComponent.GetInstance();
		FactionManager 				   fm  = FactionManager.Cast(world.FindEntityByName("FactionManager"));
		if(!gmc)
		{
			Print("Missing Group Manager Component",  LogLevel.WARNING);
			return false;
		}		
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
					
					current_group = closest_group;
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
			int desired_group_sizes = Math.Sqrt(CLINTON_VirtualPlayerManager.GetPlayersCount());
			
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
	
	string GetPlayerName(){ return m_sCharacterName; }
	
	void SetPlayerName(string name)  // string format
	{
		// Please ask about input sanitisation on the discord
		if(name != "")
		{
			m_sCharacterName = name;  // string.Format(format, firstName, alias, surname);
		}
		return;
	}
	
	void delayedSetPlayerName()
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetCurrentCharacter());
		if(!char)
		{
			GetGame().GetCallqueue().CallLater(delayedSetPlayerName, 1024, false);
			return;
		}
		SCR_CharacterIdentityComponent idComp = SCR_CharacterIdentityComponent.Cast(
			char.FindComponent(SCR_CharacterIdentityComponent));
		if(!idComp)
		{
			GetGame().GetCallqueue().CallLater(delayedSetPlayerName, 1024, false);
			return;
		}
		string format = "";
		string name = "";
		string alias = "";
		string surname = "";
		idComp.GetFormattedFullName(format,name,alias,surname);
		
		string preferedName;
		string form = WidgetManager.Translate(format);  // Working out that localisations exist took forever
		if (form != WidgetManager.Translate("#AR-Idenity_Name_Format_Full"))
		{  // If no alias
			preferedName = string.Format("%1 %2", WidgetManager.Translate(name), WidgetManager.Translate(surname));  // There's probably a nicer way to write this
		} else {
			preferedName = string.Format("%1 %2 %3", WidgetManager.Translate(name), WidgetManager.Translate(alias), WidgetManager.Translate(surname));  // Do alias's need ' marks?
		}
		SetPlayerName( preferedName );
	}
	
	void SetGroupSettings(int setting)
	{
		m_iGroupSetting = setting;
	}
	
	int GetLoadoutIndex()
	{
		return m_iLoadout;
	}
	
	void SetLoadoutIndex( int x )
	{
		this.m_iLoadout = x;
	}
	
	int GetLoadoutPreference()
	{
		return m_iLoadoutPreference;
	}
	
	void SetLoadoutPreference( int x )
	{
		this.m_iLoadoutPreference = x;
	}
	
	// IDK what this is, I just copied the 6th Modding Bootcamp
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.EncodeString(packet);
		snapshot.EncodeString(packet);
	}
	
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.DecodeString(packet);
		snapshot.DecodeString(packet);
		
		return true;
	}
	
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		return true;
		return lhs.CompareStringSnapshots(rhs)  // m_String
			&& lhs.CompareStringSnapshots(rhs);  // m_String
	}
	
	static bool PropCompare(CLINTON_Virtual_Player instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return true;
		return snapshot.CompareString(instance.m_sFactionKey)
		    && snapshot.CompareString(instance.m_sCharacterName);
	}
	
	static bool Extract(CLINTON_Virtual_Player instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
        snapshot.SerializeString(instance.m_sFactionKey);
        snapshot.SerializeString(instance.m_sCharacterName);
		
		return true;
	}
	
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, CLINTON_Virtual_Player instance)
	{
		snapshot.SerializeString(instance.m_sFactionKey);
        snapshot.SerializeString(instance.m_sCharacterName);
		
		return true;
	}
}