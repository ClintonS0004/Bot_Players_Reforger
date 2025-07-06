[EntityEditorProps(category: "Tutorial/Entities", description: "Farts and poos")]
class CLINTON_BotWaypointManagerEntityClass : GenericEntityClass
{
	// See: https://community.bistudio.com/wiki/Arma_Reforger:Create_an_Entity
}

class CLINTON_BotWaypointManagerEntity : GenericEntity
{
	protected ref ScriptInvoker onWaypointCompleted;
	
	// Works: Faction(x) => Tasks(x) => Groups[...]
	protected static ref map<SCR_CaptureAndHoldArea, ref array<SCR_AIGroup>> task_to_group_record;
	protected static ref map<string, ref map<SCR_CaptureAndHoldArea, ref array<SCR_AIGroup>> > faction_and_tasks;
	protected static ref map<string, ref map<SCR_CaptureAndHoldArea, AIWaypoint>> faction_and_waypoints;
	
	// Help distance computations
	protected static ref array<SCR_CaptureAndHoldArea> m_aCaptureAreas;
	protected static ref map<SCR_CaptureAndHoldArea, AIWaypoint> m_mCaptureAreaToWaypoint;
	protected static ref map<AIWaypoint, SCR_CaptureAndHoldArea> m_mWaypointToCapturepoint;
	protected static ref map<AIWaypoint, ref array<SCR_AIGroup>> m_mWaypointToGroup; 

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
	
	//------------------------------------------------------------------------------------------------
	void setting_mems()
	{ 	
		// Begin with: tasks(x) => empty set
		task_to_group_record = new map<SCR_CaptureAndHoldArea, ref array<SCR_AIGroup>>();
		
		// First get gamemode
		BaseGameMode gMode = GetGame().GetGameMode();
		
		// if the gamemode as an SCR_Cap... component
		SCR_CaptureAndHoldManager chm = SCR_CaptureAndHoldManager.Cast(gMode.FindComponent(SCR_CaptureAndHoldManager));
		if( chm )
		{
			ref array<SCR_CaptureAndHoldArea> outAreas = {};
			chm.GetAreas(outAreas);
			m_aCaptureAreas = outAreas;
			m_mCaptureAreaToWaypoint = new map<SCR_CaptureAndHoldArea,AIWaypoint>();
			faction_and_tasks = new map<string, ref map<SCR_CaptureAndHoldArea, ref array<SCR_AIGroup> >>();
			foreach( SCR_CaptureAndHoldArea area : outAreas)
			{
				// possibly insert area into m_aCaptureAreas
				task_to_group_record.Insert(area, new array<SCR_AIGroup>());
				m_mCaptureAreaToWaypoint.Insert(area, null);
			}
			
			// Give each faction: Faction(x) => tasks(x)
			FactionManager fm = GetGame().GetFactionManager();
			ref array<Faction> m_aFactions = {};
			ref SCR_Faction scrFaction;
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
					continue;
				
				string factionKey = faction.GetFactionKey();
				
				// Make a copy of task_to_group_record
				map<SCR_CaptureAndHoldArea, ref array<SCR_AIGroup>> area_to_group = new map<SCR_CaptureAndHoldArea, ref array<SCR_AIGroup>>();
				map<SCR_CaptureAndHoldArea, AIWaypoint> empty_set_of_waypoints = new map<SCR_CaptureAndHoldArea, AIWaypoint>();
				foreach( SCR_CaptureAndHoldArea area : outAreas)
				{
					// possibly insert area into m_aCaptureAreas
					area_to_group.Insert(area, new array<SCR_AIGroup>());
					empty_set_of_waypoints.Insert(area, null);
				}
				faction_and_tasks.Set(factionKey, area_to_group);
				
				if( !faction_and_waypoints ) 
				{ 
					faction_and_waypoints = new map<string, ref map<SCR_CaptureAndHoldArea, AIWaypoint>>();
				}
				faction_and_waypoints.Set(factionKey, empty_set_of_waypoints)
			}
			m_mWaypointToGroup = new map<AIWaypoint, ref array<SCR_AIGroup>>();
			m_mWaypointToCapturepoint = new map<AIWaypoint, SCR_CaptureAndHoldArea>();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool register_a_group(SCR_AIGroup grp, string factionKey, vector leaderPosition)
	{
		int LeaderID = grp.GetLeaderID();
		SCR_CaptureAndHoldArea closest_point = null;
		if(m_aCaptureAreas)  // If CaP area's exist and Initialised
		{
			foreach( SCR_CaptureAndHoldArea cap_i : m_aCaptureAreas)
					{
						Faction ownerFaction = SCR_CaptureArea.Cast(cap_i).GetOwningFaction();						
						if( ownerFaction.GetFactionKey() == factionKey) continue;
						if( !closest_point )
						{
							closest_point = cap_i;
							continue;
						}
						if( vector.Distance(leaderPosition, closest_point.GetOrigin()) > vector.Distance(leaderPosition, cap_i.GetOrigin()))
						{
							closest_point = SCR_CaptureAndHoldArea.Cast(cap_i);
						}
					}
					if( !closest_point )
					{
						// TODO: Make a Snd or defend waypoint or soomthing
					}
			// Possibly factor in Conflict bases
			
			// Instead of making a waypoint for each group, make a waypoint for an area
			AIWaypoint marker;
			if( faction_and_waypoints )
			{
				faction_and_waypoints.Get(factionKey).Find(closest_point, marker);
			}
			
			// {04E9DFE7245455FF}Prefabs/AI/Waypoints/CLINTON_GetInNearestLarger_NoAuto.et
			// {B049D4C74FBC0C4D}Prefabs/AI/Waypoints/AIWaypoint_GetInNearest.et
			if( GetGame().GetWorld().IsEditMode()) return false;
			EntitySpawnParams spawnParamsT = new EntitySpawnParams();
			spawnParamsT.TransformMode = ETransformMode.WORLD;
			spawnParamsT.Transform[3] = leaderPosition; // interpreted as a vector                     I can't get a GetInNearest waypoint setup that works with a scattered group too far to all board a vehicle
																										// Try changing the radius setting on the waypoint, and even setting the completion condition to any member (or coding your own condition prehaps? :) )
			IEntity new_waypointT = GetGame().SpawnEntityPrefab(Resource.Load("{B049D4C74FBC0C4D}Prefabs/AI/Waypoints/AIWaypoint_GetInNearest.et"), GetGame().GetWorld(), spawnParamsT);
			grp.AddWaypoint(SCR_BoardingTimedWaypoint.Cast(new_waypointT));
			
			if( !marker )
			{  // Make waypoint
				if( GetGame().GetWorld().IsEditMode() ) return false;
				
				EntitySpawnParams spawnParams = new EntitySpawnParams();
				spawnParams.TransformMode = ETransformMode.WORLD;
				spawnParams.Transform[3] = closest_point.GetOrigin(); // interpreted as a vector
				
				IEntity new_waypoint = GetGame().SpawnEntityPrefab(Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et"), GetGame().GetWorld(), spawnParams);
				AIWaypoint new_waypoint_waypoint = AIWaypoint.Cast(new_waypoint);
				grp.AddWaypoint(new_waypoint_waypoint);
				faction_and_waypoints.Get(factionKey).Set(closest_point, new_waypoint_waypoint);
				
				ref array<SCR_AIGroup> mem_group_array = {};
				mem_group_array.Insert(grp);
				m_mWaypointToGroup.Set( new_waypoint_waypoint, mem_group_array );
				
				m_mWaypointToCapturepoint.Set( new_waypoint_waypoint, closest_point );
			} 
			else 
			{
				grp.AddWaypoint(marker);
				ref array<SCR_AIGroup> mem_group_array = {};
				m_mWaypointToGroup.Find(marker, mem_group_array);
				if (!mem_group_array)
				{
					Print("I don't think this should be happening", LogLevel.ERROR);
				} else {
					mem_group_array.Insert(grp);
					m_mWaypointToGroup.Set(marker, mem_group_array);
				}
			}
		}
		map<SCR_CaptureAndHoldArea, ref array<SCR_AIGroup>> temp_task_to_group_record = faction_and_tasks.Get(factionKey);
		ref array<SCR_AIGroup> mem_groups = new array<SCR_AIGroup>();
		temp_task_to_group_record.Find(closest_point, mem_groups);  // No warning given for no initialisation
		mem_groups.Insert(grp);
		temp_task_to_group_record.Set(closest_point, mem_groups);
		
		faction_and_tasks.Set(factionKey, temp_task_to_group_record);
		return true;
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
				continue;
			
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
	
	//------------------------------------------------------------------------------------------------
	bool update_group(AIGroup grp, string factionKey)
	{
		AIWaypoint checking = grp.GetCurrentWaypoint();
		vector coords = grp.GetLeaderEntity().GetOrigin();
		
		if( !m_aCaptureAreas ) setting_mems();
		if( !coords ) return true;
		if( !checking ) register_a_group( SCR_AIGroup.Cast(grp), factionKey, coords);
		
		return false;
		EventHandlerManagerComponent eventHandler = EventHandlerManagerComponent.Cast(  // Can't find an Event Handler to create an invoker, see youtbe
		SCR_AIGroup.Cast(grp).FindComponent(EventHandlerManagerComponent));
		
		// Alternatively I can make my own listerner. But I have tried that and it doesn't like to work
		// eventHandler.RegisterScriptHandler("OnWaypointCompleted", grp.GetCurrentWaypoint(), NextObjective);
		// https://youtu.be/f-eok217WmU?si=kKRLPYEkkwpW7_yN&t=521
		
		array<BaseEventHandler> base = {};
		int number = eventHandler.GetEventHandlers(base);
		if( number != 0 )
		{
			onWaypointCompleted = SCR_AIGroup.Cast(grp).GetOnWaypointCompleted();
			onWaypointCompleted.Insert( NextObjective );
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void NextObjective(AIWaypoint wp)  
	{
		/*
		* All groups with this waypoint will need to be given a new waypoint
		*/
		if(!wp)
		{
			Print("Missing waypoint |CLINTON_BotWaypointManagerEntity.NextObjective", LogLevel.ERROR);
			return;
		}
		string debug_me = SCR_AIWaypoint.Cast(wp).Type().ToString();
		if(!(debug_me == "SCR_AIWaypoint")) 
		{
			return;
			// or even if not moveWP
			// "SCR_AIWaypoint"
			// "SCR_BoardingTimeWaypoint"
		}
		SCR_CaptureAndHoldArea cahArea = SCR_CaptureAndHoldArea.Cast(m_mWaypointToCapturepoint.Get(wp));
		if(!cahArea)
		{
			Print("Missing SCR_Waypoint |CLINTON_BotWaypointManagerEntity.NextObjective", LogLevel.ERROR);
			return;
		}
		ref array<SCR_AIGroup> groups;
		m_mWaypointToGroup.Find(wp, groups);
		if(!groups)
		{
			Print("Missing SCR_AIGRoup array |CLINTON_BotWaypointManagerEntity.NextObjective", LogLevel.ERROR);
			return;
		}
		Faction factionWhichOwnsWP = Faction.Cast(groups.Get(0).GetFaction());
		string factionWhichOwnsWP_key = factionWhichOwnsWP.GetFactionKey();
		
		SCR_CaptureAndHoldArea current_area = SCR_CaptureAndHoldArea.Cast(m_mWaypointToCapturepoint.Get(wp));
		m_aCaptureAreas = new array<SCR_CaptureAndHoldArea>();
		SCR_CaptureAndHoldManager.GetAreaManager().GetAreas(m_aCaptureAreas);  // Remove m_aCaptureAreas and use this instead potentionally
		
		if( m_aCaptureAreas.Count() > 1)  // TODO: Reduce the number of nests
		{
			foreach(SCR_AIGroup grp : groups) 
			{
				if( !grp ) continue;  // Groups that are player's cause problems when debugging
				AIWaypoint marker = null;
				vector leaderPosition = grp.GetLeaderEntity().GetOrigin();
				SCR_CaptureAndHoldArea closest_point = null;
				
				if(m_aCaptureAreas)  // If CaP area's exist and Initialised
				{
					// TODO: Add sharing objectives around to each group
					foreach( SCR_CaptureAndHoldArea cap_i : m_aCaptureAreas)
					{
						// Considers the Capture Area taken when triggered to be owned by you
						Faction ownerFaction = SCR_CaptureArea.Cast(cap_i).GetOwningFaction();
						
						if( ownerFaction == factionWhichOwnsWP) continue;
						if( cap_i == current_area) continue;
						if( !closest_point )
						{
							closest_point = cap_i;
							continue;
						}
						if( vector.Distance(leaderPosition, closest_point.GetOrigin()) > vector.Distance(leaderPosition, cap_i.GetOrigin()))
						{
							closest_point = SCR_CaptureAndHoldArea.Cast(cap_i);
						}
					}
					if( !closest_point )
					{
						// TODO: Make a Snd or defend waypoint or soomthing
					}
					// Possibly factor in Conflict bases
					
					// Instead of making a waypoint for each group, make a waypoint for an area
					ref map<SCR_CaptureAndHoldArea, AIWaypoint> captureAreaToWaypoint = new map<SCR_CaptureAndHoldArea, AIWaypoint>();
					faction_and_waypoints.Find(factionWhichOwnsWP_key, captureAreaToWaypoint);
					captureAreaToWaypoint.Find(closest_point, marker);					
				}
				
				if ( false && grp.m_aStaticVehicles.Count() == 0 && !ChimeraCharacter.Cast(grp.GetLeaderEntity()).IsInVehicle())
				{
					// {13749E0FA05C1D5B}Prefabs/AI/Waypoints/CLINTON_GetInNearestLarger_Any.et
					// {B049D4C74FBC0C4D}Prefabs/AI/Waypoints/AIWaypoint_GetInNearest.et
					if( GetGame().GetWorld().IsEditMode()) return;
					EntitySpawnParams spawnParamsT = new EntitySpawnParams();
					spawnParamsT.TransformMode = ETransformMode.WORLD;
					spawnParamsT.Transform[3] = leaderPosition; // interpreted as a vector
					
					IEntity new_waypointT = GetGame().SpawnEntityPrefab(Resource.Load("{04E9DFE7245455FF}Prefabs/AI/Waypoints/CLINTON_GetInNearestLarger_NoAuto.et"), GetGame().GetWorld(), spawnParamsT);
					grp.AddWaypoint(SCR_BoardingTimedWaypoint.Cast(new_waypointT));
				}
					
				if( !marker )
				{  // If the closest area hasn't been given a waypoint
					if( GetGame().GetWorld().IsEditMode()) return;
			
					EntitySpawnParams spawnParams = new EntitySpawnParams();
					spawnParams.TransformMode = ETransformMode.WORLD;
					spawnParams.Transform[3] = closest_point.GetOrigin(); // interpreted as a vector
					
					IEntity new_waypoint = GetGame().SpawnEntityPrefab(Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et"), GetGame().GetWorld(), spawnParams);
					grp.AddWaypoint(AIWaypoint.Cast(new_waypoint));
					
					ref map<SCR_CaptureAndHoldArea, AIWaypoint> captureAreaToWaypoint = new map<SCR_CaptureAndHoldArea, AIWaypoint>();
					faction_and_waypoints.Find(factionWhichOwnsWP_key, captureAreaToWaypoint);
					
					marker = AIWaypoint.Cast(new_waypoint);
					
					captureAreaToWaypoint.Set(closest_point, marker);
					faction_and_waypoints.Set(factionWhichOwnsWP_key, captureAreaToWaypoint);
					
					m_mWaypointToCapturepoint.Set( marker, closest_point);
				}
				if(marker != wp)
				{  // If the new Waypoint is not the Waypoint which was finished to trigger this
					/*
					 * TODO: Here we can add GetIn waypoints in the future
					 */
					
					grp.RemoveWaypointFromGroup(wp);
					
					ref map<SCR_CaptureAndHoldArea, ref array<SCR_AIGroup>> task_to_group = new map<SCR_CaptureAndHoldArea, ref array<SCR_AIGroup>>();
					faction_and_tasks.Find(factionWhichOwnsWP_key, task_to_group);
					
					array<SCR_AIGroup>.Cast(task_to_group.Get(current_area)).RemoveItem(grp);
					array<SCR_AIGroup> giveUs = array<SCR_AIGroup>.Cast(m_mWaypointToGroup.Get(wp));
					giveUs.RemoveItem(grp);
					m_mWaypointToGroup.Set(wp, giveUs);
					m_mWaypointToCapturepoint.Remove(wp);
					
					grp.AddWaypointToGroup(marker);  
					array<SCR_AIGroup>.Cast(task_to_group.Get(closest_point)).Insert(grp);
					ref array<SCR_AIGroup> markus = array<SCR_AIGroup>.Cast(m_mWaypointToGroup.Get(marker));
					if(!markus) markus = new array<SCR_AIGroup>();
					markus.Insert(grp);
					m_mWaypointToGroup.Set(marker, markus);
					m_mWaypointToCapturepoint.Set( marker, closest_point);
				}
			}
		} else {
			// Maybe defend waypoint for a single area match
			Print("One area in the array, this shouldn't be happening.", LogLevel.ERROR);
		}
	}
} 