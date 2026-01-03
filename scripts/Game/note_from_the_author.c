/*
Dear Clinton 	26-02-2025

modded class ArmaReforgerScripted
{
	override event void OnWorldPostProcess(World world)
	{
		super.OnWorldPostProcess(world);
		// big help from the bootcamps https://youtu.be/edZ22fzxFv4?si=etOs4NLtiEy1z2AY&t=2179
		
		if( world.IsEditMode()) return;
		
		// {C012BB3488BEA0C2}Prefabs/Vehicles/Wheeled/BTR70/BTR70.et
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = "256 1 127"; // interpreted as a vector
		
		GetGame().SpawnEntityPrefab(Resource.Load("{C012BB3488BEA0C2}Prefabs/Vehicles/Wheeled/BTR70/BTR70.et"), world, spawnParams);
		// try like this
		
		// https://youtu.be/edZ22fzxFv4?si=OwGWifxGLBf7OQlW&t=3297
		
		// Be carefull of non-static data deleting itself
		// also make sure class constructors are returning instances when needed
		
		CLINTON_VirtualPlayerManager manager_mandy = new CLINTON_VirtualPlayerManager(world);
		manager_mandy.discover_loadouts(world);
		manager_mandy.add_goober();
		manager_mandy.check_respawns();
		manager_mandy.Create_Groups();
		ref array<SCR_AIGroup> debug_me = {};
		SCR_GroupsManagerComponent.GetInstance().GetAllPlayableGroups(debug_me);
		
		// SCR_AIGroupUIInfo ? it's null thus far, but the group remains
		
		// SCR_AIGroup.m_bDeleteIfNoPlayer is currently = 1
		
        Print("This is all", LogLevel.ERROR);
		return;
		// manager_mandy is having it's destructor run
		// maybe make a bot player manager entity and run the spawn logic around ticks
		
		// The fellow on Discord mentioned really effective tools to use
		// THis is where I'm getting the 'resources' which are called
		//  prefabs, not to be mistaken for the prefabs folder.
		//  explaination: https://community.bistudio.com/wiki/Arma_Reforger:Prefab_Data
		
		// SCR_AIGroup.AddAgentFromControlledEntity();
		// SCR_AIGroup.IsFull();
	}
}

*/
	
/*

Initial Group Idea

array<SCR_AIGroup> the_big_group_in_the_sky = m_aGroups.Get(vPlayer.GetFactionKey());
ref SCR_AIGroup current_group;

if(the_big_group_in_the_sky == null)
{		
	ref Resource    emptGroupResource  = Resource.Load("{8B4D49A9F324E7D5}Prefabs/Groups/PlayableGroup.et");
	ref IEntity     emptGroupEnt       = GetGame().SpawnEntityPrefab(emptGroupResource, GetGame().GetWorld(), spawnParams);
	ref SCR_AIGroup emptGroup          = SCR_AIGroup.Cast(emptGroupEnt);
	
	emptGroup.SetFaction(fm.GetFactionByKey(vPlayer.GetFactionKey()));
	// SCR_EditableGroupComponent bigSource = emptGroupEnt.FindComponent(SCR_EditableGroupComponent);
	the_big_group_in_the_sky = {};  // Do I need the arguements of the constructor?
	current_group = emptGroup;
	// current_group = new SCR_AIGroup(bigSource, emptGroupEnt);  // Yes, this is causing a crash
	current_group.AddAgentFromControlledEntity(AIAgent.Cast(vPlayer.GetCurrentCharacter()));  // try using a base group prefab
	the_big_group_in_the_sky.Insert(current_group);
	m_aGroups.Set(vPlayer.GetFactionKey(), the_big_group_in_the_sky);
} else {
	// check for fullness
	int i = 0;
	bool term = false;
	
	while( i < the_big_group_in_the_sky.GetSizeOf() && !term)
	{
		current_group = the_big_group_in_the_sky[i];
		if( !current_group.IsFull() )
		{
			current_group.AddAgent(AIAgent.Cast(vPlayer.GetCurrentCharacter()));
			the_big_group_in_the_sky.Set(i, current_group);
			term = true;
		}
	}
	if( !term )
	{
		current_group = new SCR_AIGroup(null, null);
		current_group.AddAgent(AIAgent.Cast(vPlayer.GetCurrentCharacter()));
		the_big_group_in_the_sky.Insert(current_group);
		m_aGroups.Set(vPlayer.GetFactionKey(), the_big_group_in_the_sky);
	}
}
return true;
*/

/*
      if (!Replication.IsServer()){ return false; }
      else
      {
          ref BaseRplComponent char_rplc = BaseRplComponent.Cast(emptGroup.FindComponent(RplComponent));
          if (!char_rplc)
          {
              Print("This example requires that the entity has an RplComponent.", LogLevel.WARNING);
              return false;
          }
          SCR_ChimeraCharacter chim = SCR_ChimeraCharacter.Cast(user);
          char_rplc.Give(Replication.FindOwner(Replication.FindId(chim)));
          Print("RPC Auth :" + char_rplc.IsOwner(), LogLevel.ERROR);
      }
      ref BaseRplComponent char_rplc    = BaseRplComponent.Cast(emptGroup.FindComponent(RplComponent));
      Print("RPC Auth :" + char_rplc.IsOwner(), LogLevel.ERROR);

char_rplc = BaseRplComponent.Cast(user.FindComponent(RplComponent));
ref BaseRplComponent group_rplc   = BaseRplComponent.Cast(emptGroupEnt.FindComponent(RplComponent));

gm.AskAddAiMemberToGroup(
	Replication.FindId(group_rplc),
	Replication.FindId(char_rplc)
);*/


// {7DBFFCF40E113B9F}Prefabs/CLINTON_Group_Base.et
// {8B4D49A9F324E7D5}Prefabs/Groups/PlayableGroup.et
// Might need the Components from an implemented group
// (Group_US_Base.et)

//gm.RegisterGroup(emptGroup);		

// emptGroup.m_aUnitPrefabSlots.Insert(defaultUSLoadout.GetLoadoutResource());	

/*
I had to override the config file of the fullscreen map. This is found as a 
setting for a (map) component of the gamemode entity
*/

/*********************************************************
 NextObjective original implementation
if( !grp ) continue;  // Groups that are player's cause problems when debugging
		AIWaypoint marker = null;
		vector leaderPosition = grp.GetLeaderEntity().GetOrigin();
		SCR_CaptureAndHoldArea closestNeutralPoint = null;
		
		if(m_aCaptureAreas)  // If CaP area's exist and Initialised
		{
			// TODO: Add sharing objectives around to each group
			foreach( SCR_CaptureAndHoldArea cap_i : m_aCaptureAreas)
			{
				// Considers the Capture Area taken when triggered to be owned by you
				Faction ownerFaction = SCR_CaptureArea.Cast(cap_i).GetOwningFaction();
				
				if( ownerFaction == factionWhichOwnsWP) continue;
				if( cap_i == current_area) continue;
				if( !closestNeutralPoint )
				{
					closestNeutralPoint = cap_i;
					continue;
				}
				if( vector.Distance(leaderPosition, closestNeutralPoint.GetOrigin()) > vector.Distance(leaderPosition, cap_i.GetOrigin()))
				{
					closestNeutralPoint = SCR_CaptureAndHoldArea.Cast(cap_i);
				}
			}
			if( !closestNeutralPoint )
			{
				// TODO: Make a Snd or defend waypoint or soomthing
				
			}
			// Possibly factor in Conflict bases
			
			// Instead of making a waypoint for each group, make a waypoint for an area
			ref map<SCR_CaptureAndHoldArea, AIWaypoint> captureAreaToWaypoint = new map<SCR_CaptureAndHoldArea, AIWaypoint>();
			faction_and_waypoints.Find(factionWhichOwnsWP_key, captureAreaToWaypoint);
			captureAreaToWaypoint.Find(closestNeutralPoint, marker);					
		}
		
		if ( false && grp.m_aStaticVehicles.Count() == 0 && !ChimeraCharacter.Cast(grp.GetLeaderEntity()).IsInVehicle())
		{
			// {04E9DFE7245455FF}Prefabs/AI/Waypoints/CLINTON_GetInNearestLarger_NoAuto.et
			// {20EB568072BC0ADB}scripts/Game/CLINTON_CarpoolWaypointEntity.et
			if( GetGame().GetWorld().IsEditMode()) return;
			EntitySpawnParams spawnParamsT = new EntitySpawnParams();
			spawnParamsT.TransformMode = ETransformMode.WORLD;
			spawnParamsT.Transform[3] = closestNeutralPoint.GetOrigin(); // interpreted as a vector
			//spawnParamsT.Transform[3] = leaderPosition; // interpreted as a vector
			
			IEntity new_waypointT = GetGame().SpawnEntityPrefab(Resource.Load("{20EB568072BC0ADB}scripts/Game/CLINTON_CarpoolWaypointEntity.et"), GetGame().GetWorld(), spawnParamsT);
			grp.AddWaypoint(SCR_BoardingTimedWaypoint.Cast(new_waypointT));
		}
			
		if( !marker )
		{  // If the closest area hasn't been given a waypoint
			if( GetGame().GetWorld().IsEditMode()) return;
	
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform[3] = closestNeutralPoint.GetOrigin(); // interpreted as a vector
			
			IEntity new_waypoint = GetGame().SpawnEntityPrefab(Resource.Load("{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et"), GetGame().GetWorld(), spawnParams);
			grp.AddWaypoint(AIWaypoint.Cast(new_waypoint));
			
			ref map<SCR_CaptureAndHoldArea, AIWaypoint> captureAreaToWaypoint = new map<SCR_CaptureAndHoldArea, AIWaypoint>();
			faction_and_waypoints.Find(factionWhichOwnsWP_key, captureAreaToWaypoint);
			
			marker = AIWaypoint.Cast(new_waypoint);
			
			captureAreaToWaypoint.Set(closestNeutralPoint, marker);
			faction_and_waypoints.Set(factionWhichOwnsWP_key, captureAreaToWaypoint);
			
			m_mWaypointToCapturepoint.Set( marker, closestNeutralPoint);
		}
		if(marker != wp)
		{  // If the new Waypoint is not the Waypoint which was finished to trigger this
			//
			// TODO: Here we can add GetIn waypoints in the future
			 
			
			grp.RemoveWaypointFromGroup(wp);
			
			ref map<SCR_CaptureAndHoldArea, ref array<SCR_AIGroup>> task_to_group = new map<SCR_CaptureAndHoldArea, ref array<SCR_AIGroup>>();
			faction_and_tasks.Find(factionWhichOwnsWP_key, task_to_group);
			
			array<SCR_AIGroup>.Cast(task_to_group.Get(current_area)).RemoveItem(grp);
			array<SCR_AIGroup> giveUs = array<SCR_AIGroup>.Cast(m_mWaypointToGroup.Get(wp));
			giveUs.RemoveItem(grp);
			m_mWaypointToGroup.Set(wp, giveUs);
			m_mWaypointToCapturepoint.Remove(wp);
			
			grp.AddWaypointToGroup(marker);  
			array<SCR_AIGroup>.Cast(task_to_group.Get(closestNeutralPoint)).Insert(grp);
			ref array<SCR_AIGroup> markus = array<SCR_AIGroup>.Cast(m_mWaypointToGroup.Get(marker));
			if(!markus) markus = new array<SCR_AIGroup>();
			markus.Insert(grp);
			m_mWaypointToGroup.Set(marker, markus);
			m_mWaypointToCapturepoint.Set( marker, closestNeutralPoint);
		}*/