[EntityEditorProps(category: "Tutorial/Entities", description: "Farts and poos")]
class CLINTON_BotWaypointManagerEntityClass : GenericEntityClass
{
	// See: https://community.bistudio.com/wiki/Arma_Reforger:Create_an_Entity
}

class CLINTON_BotWaypointManagerEntity : GenericEntity
{
	// Works: Faction(x) => Tasks(x) => Groups[...]
	protected ref map<SCR_CaptureAndHoldArea,array<SCR_AIGroup>> task_to_group_record;
	protected ref map<string, map<SCR_CaptureAndHoldArea, array<SCR_AIGroup> >> faction_and_tasks;
	
	// Help distance computations
	protected static ref array<SCR_CaptureAndHoldArea> m_aCaptureAreas;
	protected static ref map<SCR_CaptureAndHoldArea,SCR_AIWaypoint> m_mCaptureAreaToWaypoint;
	
	protected static CLINTON_BotWaypointManagerEntity s_Instance;
	
	void CLINTON_BotWaypointManagerEntity(IEntitySource src, IEntity parent)
	{
		if (s_Instance)
		{
			Print("Only one instance of CLINTON_BotWaypointManagerEntity is allowed in the world!", LogLevel.WARNING);
			delete this;
			return;
		}
		s_Instance = this;
		setting_mems();
	}
	
	static CLINTON_BotWaypointManagerEntity getInstance(){ return s_Instance; }
	
	void setting_mems()
	{ 
		// Pweese debug
		
		// Begin with: tasks(x) => empty set
		task_to_group_record = new map<SCR_CaptureAndHoldArea,array<SCR_AIGroup>>();
		
		// First get gamemode
		BaseGameMode gMode = GetGame().GetGameMode();
		
		// if the gamemode as an SCR_Cap... component
		SCR_CaptureAndHoldManager chm = SCR_CaptureAndHoldManager.Cast(gMode.FindComponent(SCR_CaptureAndHoldManager));
		if( chm )
		{
			ref array<SCR_CaptureAndHoldArea> outAreas = {};
			chm.GetAreas(outAreas);
			m_aCaptureAreas = outAreas;
			m_mCaptureAreaToWaypoint = new map<SCR_CaptureAndHoldArea,SCR_AIWaypoint>();
			faction_and_tasks = new map<string, map<SCR_CaptureAndHoldArea, array<SCR_AIGroup> >>();
			foreach( SCR_CaptureAndHoldArea area : outAreas)
			{
				// posibly insert area into m_aCaptureAreas
				task_to_group_record.Insert(area, new array<SCR_AIGroup>());
				m_mCaptureAreaToWaypoint.Insert(area, null);
			}
			
			// Give each faction: Faction(x) => tasks(x)
			FactionManager fm = GetGame().GetFactionManager();
			ref array<Faction> m_aFactions = {};
			SCR_Faction scrFaction;
			if (fm)
			{
				fm.GetFactionsList(m_aFactions);
			}
			foreach (Faction faction : m_aFactions)
			{
				if (!faction)
					continue;
	
				scrFaction = SCR_Faction.Cast(faction);
				if (scrFaction && !scrFaction.IsPlayable())
					continue; //--- ToDo: Refresh dynamically when a new faction is added/removed
				
				string factionKey = faction.GetFactionKey();
				// Make a copy of task_to_group_record
				
				map<SCR_CaptureAndHoldArea,array<SCR_AIGroup>> empty_set_of_tasks = new map<SCR_CaptureAndHoldArea,array<SCR_AIGroup>>();
				empty_set_of_tasks.Copy(task_to_group_record);
				
				faction_and_tasks.Set(factionKey, empty_set_of_tasks);  // Make sure we are passing by value and not reference
			}
		}
	}
	
	// Return: was there a waypoint given?
	bool register_a_group(SCR_AIGroup grp, string factionKey, vector leaderPosition)
	{
		int LeaderID = grp.GetLeaderID();  // measuring from leader to capture area
		// same as Entity ID?
		if(m_aCaptureAreas)  // If CaP area's exist and Initialised
		{
			SCR_CaptureAndHoldArea closest_point = this.m_aCaptureAreas.Get(0);
			foreach( SCR_CaptureAndHoldArea cap_i : m_aCaptureAreas)
			{
				if( vector.Distance(leaderPosition, closest_point.GetOrigin()) > vector.Distance(leaderPosition, cap_i.GetOrigin()))
				{
					closest_point = SCR_CaptureAndHoldArea.Cast(cap_i);
				}
			}
			// Possibly factor in Conflict bases
			
			// Instead of making a waypoint for each group, make a waypoint for an area
			SCR_AIWaypoint marker;
			m_mCaptureAreaToWaypoint.Find(closest_point, marker);
		
			
			if( !marker )
			{  // Make waypoint
				if( GetGame().GetWorld().IsEditMode()) return false;
		
				// {B3E7B8DC2BAB8ACC}Prefabs/AI/Waypoints/AIWaypoint_SearchAndDestroy.et
				// {750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et
				EntitySpawnParams spawnParams = new EntitySpawnParams();
				spawnParams.TransformMode = ETransformMode.WORLD;
				spawnParams.Transform[3] = closest_point.GetOrigin(); // interpreted as a vector
				
				
				IEntity new_waypoint = GetGame().SpawnEntityPrefab(Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et"), GetGame().GetWorld(), spawnParams);
				grp.AddWaypoint(AIWaypoint.Cast(new_waypoint));
			} else {
				grp.AddWaypoint(AIWaypoint.Cast(marker));
			}
		}
		faction_and_tasks.Get(factionKey);  // what missing if m_aCaptureAreas is missing
		return true;
		
		// Try using: SCR_AIGroup.Event_OnWaypointCompleted to create future waypoints
	}
	
	void update()
	{  // Update but for every faction and group
		/*
		Check attributes are initialised
		When Respawns and Objective captures, give Groups waypoints
		*/
		
		// Check if groups have any waypoints
		map<string, ref array<SCR_AIGroup>> skibidi = CLINTON_RoboPlayerManager.getInstance().GetGroups();
		
		// Loop through factions
		FactionManager fm = GetGame().GetFactionManager();
		array<Faction> m_aFactions;
		SCR_Faction scrFaction;
		if (fm)
		{
			fm.GetFactionsList(m_aFactions);
		} else{
			return;
		}
		foreach (Faction faction : m_aFactions)
		{
			if (!faction)
				continue;

			scrFaction = SCR_Faction.Cast(faction);
			if (scrFaction && !scrFaction.IsPlayable())
				continue; //--- ToDo: Refresh dynamically when a new faction is added/removed
			
			string factionKey = faction.GetFactionKey();
			foreach(SCR_AIGroup grp : skibidi.Get(factionKey))
			{
				AIWaypoint checking = grp.GetCurrentWaypoint();
				vector coords = grp.GetLeaderEntity().GetOrigin();
				if( !coords )
				{
					continue;
				}
				if( !checking )
				{
					register_a_group( SCR_AIGroup.Cast(grp), factionKey, coords);
				}
			} 
		}
		// array< ref CLINTON_Virtual_Player> list_of_bots = CLINTON_RoboPlayerManager.GetPlayers();
	}
	
	bool update_group(AIGroup grp, string factionKey)
	{
		AIWaypoint checking = grp.GetCurrentWaypoint();
		vector coords = grp.GetLeaderEntity().GetOrigin();
		if( !m_aCaptureAreas )
		{
			setting_mems();
		}
		if( !coords )
		{
			return true;
		}
		if( !checking )
		{
			register_a_group( SCR_AIGroup.Cast(grp), factionKey, coords);
		}
		EventHandlerManagerComponent toilet = EventHandlerManagerComponent.Cast(  // Can't find an Event Handler to create an invoker, see youtbe
		SCR_AIGroup.Cast(grp).FindComponent(EventHandlerManagerComponent));
		//grp.OnWaypointCompleted()
		//SCR_AIGroup.Cast(grp).GetOnWaypointCompleted().Insert(OnWaypointCompleted);
		
		// toilet is null
		if(!toilet)
		{
			return true;
		}
		
		// https://youtu.be/f-eok217WmU?si=kKRLPYEkkwpW7_yN&t=521
		toilet.RegisterScriptHandler("OnWaypointCompleted",grp,giveMe); 
		
		return false;
	}// Try using: SCR_AIGroup.Event_OnWaypointCompleted to create future waypoints
	// idk the type
	// event OnWaypointCompleted()
	
	//------------------------------------------------------------------------------------------------
	protected void giveMe(AIGroup grp)
	{
		// I haven't been able to debug this
		string faction = SCR_AIGroup.Cast(grp).GetFactionName();
		
		update_group(grp, faction);
		//update_group(grp, factionKey);
		Print("Bing Bing Wahoo", LogLevel.DEBUG); // NEXT: toilet is not getting set. See break point
	}
} 