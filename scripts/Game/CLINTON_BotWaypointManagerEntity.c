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
	
	void ~CLINTON_BotWaypointManagerEntity(IEntitySource src, IEntity parent)
	{
		BaseGameMode gMode = GetGame().GetGameMode();
		SCR_CaptureAndHoldManager chm = SCR_CaptureAndHoldManager.Cast(gMode.FindComponent(SCR_CaptureAndHoldManager));
		if( chm )
		{
			ref array<SCR_CaptureAndHoldArea> outAreas = {};
			chm.GetAreas(outAreas);
			foreach( SCR_CaptureAndHoldArea area : outAreas)
			{
				CaptureAreaOwnershipEvent eventHandler = area.GetOwnershipChangedEvent();
				eventHandler.Remove(OnCapturePointChange);
			}
		}
	}
	
	static CLINTON_BotWaypointManagerEntity getInstance(){ return s_Instance; }
	
	// Here place other capture point and bases managers
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
				CaptureAreaOwnershipEvent eventHandler = area.GetOwnershipChangedEvent();
				eventHandler.Insert(OnCapturePointChange);		
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
		// int LeaderID = grp.GetLeaderID();
		SCR_CaptureAndHoldArea closestNeutralPoint  = null;
		SCR_CaptureAndHoldArea closestEnemyPoint 	= null;
		SCR_CaptureAndHoldArea closestFriendlyPoint = null;
		SCR_CaptureAndHoldArea sector;
		if(m_aCaptureAreas)  // If CaP area's exist and Initialised
		{
			foreach( SCR_CaptureAndHoldArea cap_i : m_aCaptureAreas)
			{
					Faction ownerFaction = SCR_CaptureArea.Cast(cap_i).GetOwningFaction();
					if( !ownerFaction )  // if it's null still consider it
					{
						if( !closestNeutralPoint)
						{
							closestNeutralPoint = SCR_CaptureAndHoldArea.Cast(cap_i);
							continue;
						}
						if( vector.Distance(leaderPosition, closestNeutralPoint.GetOrigin()) > vector.Distance(leaderPosition, cap_i.GetOrigin()))
						{
							closestNeutralPoint = SCR_CaptureAndHoldArea.Cast(cap_i);
						} else { continue; }
					}
					else if( ownerFaction.GetFactionKey() != factionKey)
					{
						if( !closestEnemyPoint)
						{
							closestEnemyPoint = SCR_CaptureAndHoldArea.Cast(cap_i);
							continue;
						}
						if( vector.Distance(leaderPosition, closestEnemyPoint.GetOrigin()) > vector.Distance(leaderPosition, cap_i.GetOrigin()))
						{
							closestEnemyPoint = SCR_CaptureAndHoldArea.Cast(cap_i);
						} else { continue; }
					}
					else if( ownerFaction.GetFactionKey() == factionKey )
					{
						if( !closestFriendlyPoint)
						{
							closestFriendlyPoint = SCR_CaptureAndHoldArea.Cast(cap_i);
							continue;
						}
						if( vector.Distance(leaderPosition, closestFriendlyPoint.GetOrigin()) > vector.Distance(leaderPosition, cap_i.GetOrigin()))
						{
							closestFriendlyPoint = SCR_CaptureAndHoldArea.Cast(cap_i);
						} else { continue; }
					}
			}
			if( closestNeutralPoint != null )
			{
				sector = closestNeutralPoint;
			} else if ( closestEnemyPoint != null)
			{
				sector = closestEnemyPoint;
			} else if ( closestFriendlyPoint != null)
			{
				sector = closestFriendlyPoint;
			}
			AIWaypoint marker;
			if( faction_and_waypoints )
			{
				faction_and_waypoints.Get(factionKey).Find(sector, marker);
			}
			// {04E9DFE7245455FF}Prefabs/AI/Waypoints/CLINTON_GetInNearestLarger_NoAuto.et
			// {B049D4C74FBC0C4D}Prefabs/AI/Waypoints/AIWaypoint_GetInNearest.et
			if( GetGame().GetWorld().IsEditMode()) return false;
			EntitySpawnParams spawnParamsT = new EntitySpawnParams();
			spawnParamsT.TransformMode = ETransformMode.WORLD;
			spawnParamsT.Transform[3] = sector.GetOrigin(); // interpreted as a vector                 I can't get a GetInNearest waypoint setup that works with a scattered group too far to all board a vehicle
			// spawnParamsT.Transform[3] = leaderPosition; // interpreted as a vector                     I can't get a GetInNearest waypoint setup that works with a scattered group too far to all board a vehicle
																										// Try changing the radius setting on the waypoint, and even setting the completion condition to any member (or coding your own condition prehaps? :) )
			// {20EB568072BC0ADB}scripts/Game/CLINTON_CarpoolWaypointEntity.et
			// {B049D4C74FBC0C4D}Prefabs/AI/Waypoints/AIWaypoint_GetInNearest.et
			IEntity new_waypointT = GetGame().SpawnEntityPrefab(Resource.Load("{20EB568072BC0ADB}scripts/Game/CLINTON_CarpoolWaypointEntity.et"), GetGame().GetWorld(), spawnParamsT);
			grp.AddWaypoint(SCR_CarpoolWaypoint.Cast(new_waypointT));
			
			if( !marker )
			{  // Make waypoint
				if( GetGame().GetWorld().IsEditMode() ) return false;
				
				EntitySpawnParams spawnParams = new EntitySpawnParams();
				spawnParams.TransformMode = ETransformMode.WORLD;
				spawnParams.Transform[3] = sector.GetOrigin(); // interpreted as a vector
				
				IEntity new_waypoint = GetGame().SpawnEntityPrefab(Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et"), GetGame().GetWorld(), spawnParams);
				AIWaypoint new_waypoint_waypoint = AIWaypoint.Cast(new_waypoint);
				grp.AddWaypoint(new_waypoint_waypoint);
				faction_and_waypoints.Get(factionKey).Set(sector, new_waypoint_waypoint);
				
				ref array<SCR_AIGroup> mem_group_array = {};
				mem_group_array.Insert(grp);
				m_mWaypointToGroup.Set( new_waypoint_waypoint, mem_group_array );
				
				m_mWaypointToCapturepoint.Set( new_waypoint_waypoint, sector );
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
		temp_task_to_group_record.Find(sector, mem_groups);  // No warning given for no initialisation
		mem_groups.Insert(grp);
		temp_task_to_group_record.Set(sector, mem_groups);
		
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
		map<string, ref array<SCR_AIGroup>> skibidi = CLINTON_VirtualPlayerManager.GetInstance().GetGroups();
		
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
		
		// return false;
		EventHandlerManagerComponent eventHandler = EventHandlerManagerComponent.Cast(  // Can't find an Event Handler to create an invoker, see youtbe
		SCR_AIGroup.Cast(grp).FindComponent(EventHandlerManagerComponent));
		
		// Alternatively I can make my own listerner. But I have tried that and it doesn't like to work
		// eventHandler.RegisterScriptHandler("OnWaypointCompleted", grp.GetCurrentWaypoint(), NextObjective);
		// https://youtu.be/f-eok217WmU?si=kKRLPYEkkwpW7_yN&t=521
		
		array<BaseEventHandler> base = {};
		int number = eventHandler.GetEventHandlers(base);
		if( number != 0 )
		{
			//onWaypointCompleted = SCR_AIGroup.Cast(grp).GetOnWaypointCompleted();
			//onWaypointCompleted.Insert( NextObjective );
		}
		return false;
	}
	
	protected void OnCapturePointChange(SCR_CaptureArea area, Faction previousOwner, Faction newOwner)
	{
		// void CaptureAreaOwnerFactionEventDelegate(SCR_CaptureArea area, Faction previousOwner, Faction newOwner);
		FactionManager fm = GetGame().GetFactionManager();
		ref array<Faction> m_aFactions = {};
		ref SCR_Faction scrFaction;
		vector areaVector = area.GetOrigin();
		if (fm)
		{
			fm.GetFactionsList(m_aFactions);
		}
		// if group is this faction then attack another point
		// if not then measure distance of. 
		ref array<SCR_AIGroup> groupsInaFaction;
		string factionKey;
		AIWaypoint groupsCurrentWaypoint;
		vector groupsCurrentWaypointVector;
		vector groupsLeaderCurrentVector;
		foreach (Faction faction : m_aFactions)
		{
			scrFaction = SCR_Faction.Cast(faction);
			if (scrFaction && !scrFaction.IsPlayable()) continue;  // is this good?
			factionKey = faction.GetFactionKey();
			groupsInaFaction = CLINTON_VirtualPlayerManager.GetInstance().GetGroups(faction);
			if( !groupsInaFaction) continue;
			if( newOwner != faction )
			{
				foreach(SCR_AIGroup grp : groupsInaFaction)
				{
					groupsCurrentWaypoint = grp.GetCurrentWaypoint();
					if(!groupsCurrentWaypoint) continue;
					groupsCurrentWaypointVector = groupsCurrentWaypoint.GetOrigin();
					groupsLeaderCurrentVector = grp.GetLeaderEntity().GetOrigin();
					if( vector.Distance(groupsLeaderCurrentVector, areaVector) < vector.Distance(groupsLeaderCurrentVector, groupsCurrentWaypointVector))
					{
						GiveGroupaSector(grp, factionKey, area, groupsCurrentWaypoint);
					}
				}
			}
			else
			{
				//NextObjective(wp) but with data
				// if this is the current wp then choose another
				ref map<SCR_CaptureAndHoldArea, ref array<SCR_AIGroup>> task_to_group;
				task_to_group = new map<SCR_CaptureAndHoldArea, ref array<SCR_AIGroup>>();
				faction_and_tasks.Find(factionKey, task_to_group);
				array<SCR_AIGroup> capturers = new array<SCR_AIGroup>();
				task_to_group.Find(SCR_CaptureAndHoldArea.Cast( area), capturers);
				foreach(SCR_AIGroup grp : capturers)
				{
					vector leaderPosition = grp.GetLeaderEntity().GetOrigin();
					NextObjective(grp, faction, leaderPosition);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GiveGroupaSector(SCR_AIGroup grp, string factionKey, SCR_CaptureArea ca, AIWaypoint cw)
	{
		vector areaVector = ca.GetOrigin();
		AIWaypoint existingWaypoint = null;
		ref map<SCR_CaptureAndHoldArea, ref array<SCR_AIGroup>> task_to_group;
		ref map<SCR_CaptureAndHoldArea, AIWaypoint> captureAreaToWaypoint = 
			new map<SCR_CaptureAndHoldArea, AIWaypoint>();
		faction_and_waypoints.Find(factionKey, captureAreaToWaypoint);
		captureAreaToWaypoint.Find(SCR_CaptureAndHoldArea.Cast(ca), existingWaypoint);	
						
		//if ( false && grp.m_aStaticVehicles.Count() == 0 && !ChimeraCharacter.Cast(grp.GetLeaderEntity()).IsInVehicle())
		if( GetGame().GetWorld().IsEditMode()) return;
		EntitySpawnParams spawnParamsT = new EntitySpawnParams();
		spawnParamsT.TransformMode = ETransformMode.WORLD;
		spawnParamsT.Transform[3] = areaVector;
		
		IEntity new_waypointT = GetGame().SpawnEntityPrefab(
			Resource.Load("{20EB568072BC0ADB}scripts/Game/CLINTON_CarpoolWaypointEntity.et"),
			 GetGame().GetWorld(),
			 spawnParamsT);
		grp.AddWaypointAt(SCR_BoardingTimedWaypoint.Cast(new_waypointT),0);
		if( !existingWaypoint )
		{  
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform[3] = areaVector;
			
			IEntity new_waypoint = GetGame().SpawnEntityPrefab(
				Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et"),
				 GetGame().GetWorld(),
				 spawnParams);
			grp.AddWaypointAt(AIWaypoint.Cast(new_waypoint), 1);
			//ref map<SCR_CaptureAndHoldArea, AIWaypoint> captureAreaToWaypoint = new map<SCR_CaptureAndHoldArea, AIWaypoint>();
			faction_and_waypoints.Find(factionKey, captureAreaToWaypoint);
			existingWaypoint = AIWaypoint.Cast(new_waypoint);
			captureAreaToWaypoint.Set(SCR_CaptureAndHoldArea.Cast(ca), existingWaypoint);
			faction_and_waypoints.Set(factionKey, captureAreaToWaypoint);
			m_mWaypointToCapturepoint.Set( existingWaypoint, SCR_CaptureAndHoldArea.Cast(ca));
		}
		else
		{
			task_to_group = new map<SCR_CaptureAndHoldArea, ref array<SCR_AIGroup>>();
			faction_and_tasks.Find(factionKey, task_to_group);
			array<SCR_AIGroup>.Cast(task_to_group.Get(SCR_CaptureAndHoldArea.Cast(ca))).RemoveItem(grp);
			array<SCR_AIGroup> giveUs = array<SCR_AIGroup>.Cast(m_mWaypointToGroup.Get(cw));
			giveUs.RemoveItem(grp);
			m_mWaypointToGroup.Set(cw, giveUs);
			m_mWaypointToCapturepoint.Remove(cw);
			
			grp.AddWaypointAt(existingWaypoint, 1);  
			array<SCR_AIGroup>.Cast(task_to_group.Get(SCR_CaptureAndHoldArea.Cast(ca))).Insert(grp);
			ref array<SCR_AIGroup> markus = array<SCR_AIGroup>.Cast(m_mWaypointToGroup.Get(existingWaypoint));
			if(!markus) markus = new array<SCR_AIGroup>();
			markus.Insert(grp);
			m_mWaypointToGroup.Set(existingWaypoint, markus);
			m_mWaypointToCapturepoint.Set( existingWaypoint, SCR_CaptureAndHoldArea.Cast(ca));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void NextObjective(SCR_AIGroup grp, Faction f, vector leaderPosition)  
	{
		string factionKey = f.GetFactionKey();
		SCR_CaptureAndHoldArea closestNeutralPoint  = null;
		SCR_CaptureAndHoldArea closestEnemyPoint 	= null;
		SCR_CaptureAndHoldArea closestFriendlyPoint = null;
		SCR_CaptureAndHoldArea sector;
		if(m_aCaptureAreas)  // If CaP area's exist and Initialised
		{
			foreach( SCR_CaptureAndHoldArea cap_i : m_aCaptureAreas)
			{
					Faction ownerFaction = SCR_CaptureArea.Cast(cap_i).GetOwningFaction();
					if( !ownerFaction)  // if it's null still consider it
					{
						if( !closestNeutralPoint)
						{
							closestNeutralPoint = SCR_CaptureAndHoldArea.Cast(cap_i);
							continue;
						}
						if( vector.Distance(leaderPosition, closestNeutralPoint.GetOrigin()) > vector.Distance(leaderPosition, cap_i.GetOrigin()))
						{
							closestNeutralPoint = SCR_CaptureAndHoldArea.Cast(cap_i);
						} else { continue; }
					}
					else if( ownerFaction.GetFactionKey() != factionKey)
					{
						if( !closestEnemyPoint)
						{
							closestEnemyPoint = SCR_CaptureAndHoldArea.Cast(cap_i);
							continue;
						}
						if( vector.Distance(leaderPosition, closestEnemyPoint.GetOrigin()) > vector.Distance(leaderPosition, cap_i.GetOrigin()))
						{
							closestEnemyPoint = SCR_CaptureAndHoldArea.Cast(cap_i);
						} else { continue; }
					}
					else if( ownerFaction.GetFactionKey() == factionKey )
					{
						if( !closestFriendlyPoint)
						{
							closestFriendlyPoint = SCR_CaptureAndHoldArea.Cast(cap_i);
							continue;
						}
						if( vector.Distance(leaderPosition, closestFriendlyPoint.GetOrigin()) > vector.Distance(leaderPosition, cap_i.GetOrigin()))
						{
							closestFriendlyPoint = SCR_CaptureAndHoldArea.Cast(cap_i);
						} else { continue; }
					}
			}
			if( closestNeutralPoint != null )
			{
				sector = closestNeutralPoint;
			} else if ( closestEnemyPoint != null)
			{
				sector = closestEnemyPoint;
			} else if ( closestFriendlyPoint != null)
			{
				sector = closestFriendlyPoint;
			}
			AIWaypoint marker;
			if( faction_and_waypoints )
			{
				faction_and_waypoints.Get(factionKey).Find(sector, marker);
			}
			// {04E9DFE7245455FF}Prefabs/AI/Waypoints/CLINTON_GetInNearestLarger_NoAuto.et
			// {B049D4C74FBC0C4D}Prefabs/AI/Waypoints/AIWaypoint_GetInNearest.et
			if( GetGame().GetWorld().IsEditMode()) return;
			EntitySpawnParams spawnParamsT = new EntitySpawnParams();
			spawnParamsT.TransformMode = ETransformMode.WORLD;
			spawnParamsT.Transform[3] = sector.GetOrigin(); // interpreted as a vector                 I can't get a GetInNearest waypoint setup that works with a scattered group too far to all board a vehicle
			// spawnParamsT.Transform[3] = leaderPosition; // interpreted as a vector                     I can't get a GetInNearest waypoint setup that works with a scattered group too far to all board a vehicle
																										// Try changing the radius setting on the waypoint, and even setting the completion condition to any member (or coding your own condition prehaps? :) )
			// {20EB568072BC0ADB}scripts/Game/CLINTON_CarpoolWaypointEntity.et
			// {B049D4C74FBC0C4D}Prefabs/AI/Waypoints/AIWaypoint_GetInNearest.et
			IEntity new_waypointT = GetGame().SpawnEntityPrefab(Resource.Load("{20EB568072BC0ADB}scripts/Game/CLINTON_CarpoolWaypointEntity.et"), GetGame().GetWorld(), spawnParamsT);
			grp.AddWaypoint(SCR_CarpoolWaypoint.Cast(new_waypointT));
			
			if( !marker )
			{  // Make waypoint
				if( GetGame().GetWorld().IsEditMode() ) return;
				
				EntitySpawnParams spawnParams = new EntitySpawnParams();
				spawnParams.TransformMode = ETransformMode.WORLD;
				spawnParams.Transform[3] = sector.GetOrigin(); // interpreted as a vector
				
				IEntity new_waypoint = GetGame().SpawnEntityPrefab(Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et"), GetGame().GetWorld(), spawnParams);
				AIWaypoint new_waypoint_waypoint = AIWaypoint.Cast(new_waypoint);
				grp.AddWaypoint(new_waypoint_waypoint);
				faction_and_waypoints.Get(factionKey).Set(sector, new_waypoint_waypoint);
				
				ref array<SCR_AIGroup> mem_group_array = {};
				mem_group_array.Insert(grp);
				m_mWaypointToGroup.Set( new_waypoint_waypoint, mem_group_array );
				
				m_mWaypointToCapturepoint.Set( new_waypoint_waypoint, sector );
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
	}
} 