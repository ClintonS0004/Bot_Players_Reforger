[EntityEditorProps(category: "Tutorial/Entities", description: "Farts and poos")]
class CLINTON_BotsManagerEntityClass : GenericEntityClass
{
	// See: https://community.bistudio.com/wiki/Arma_Reforger:Create_an_Entity
}

class CLINTON_BotsManagerEntity : GenericEntity
{
	protected float m_fWaitingTime = float.INFINITY;	// trigger Print on start
	protected int m_iCycleDuration = 2;				// in seconds
	
	[RplProp(onRplName: "OnRoboManagerUpdated")]
	ref CLINTON_RoboPlayerManager robot_manager;
	
	protected static CLINTON_BotsManagerEntity s_Instance;
	
	//------------------------------------------------------------------------------------------------
	void check_that_the_groups_are_alive()
	{
		/*
		array<SCR_AIGroup> debug_me = {};
		SCR_GroupsManagerComponent.GetInstance().GetAllPlayableGroups(debug_me);
		Print("Number of groups are: " + debug_me.Count(), LogLevel.DEBUG);
		*/
		
		robot_manager.check_respawns();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		m_fWaitingTime += timeSlice;
		if (m_fWaitingTime < m_iCycleDuration)
			return;

		m_fWaitingTime = 0;
		check_that_the_groups_are_alive();
	}

	//------------------------------------------------------------------------------------------------
	void CLINTON_BotsManagerEntity(IEntitySource src, IEntity parent)
	{
		if (s_Instance)
		{
			Print("Only one instance of CLINTON_BotsManagerEntity is allowed in the world!", LogLevel.WARNING);
			delete this;
			return;
		}

		s_Instance = this;
		
		// SetEventMask(EntityEvent.FRAME);
		
		// We must belong to some RplComponent in order for replication to work.
        // We search for it and warn user when we can't find it.
        auto rplComponent = BaseRplComponent.Cast(GenericEntity.Cast(this).FindComponent(RplComponent));
        if (!rplComponent)
        {
            Print("This example requires that the entity has an RplComponent.", LogLevel.WARNING);
            return;
        }
 
        // We only perform simulation on the authority instance, while all proxy
        // instances just show result of the simulation. Therefore, we only have to
        // subscribe to "frame" events on authority, leaving proxy instances as
        // passive components that do something only when necessary.
        if (rplComponent.Role() == RplRole.Authority)
        {
            SetEventMask(EntityEvent.FRAME);
        }
		
		// Initialise
		World world = GetGame().GetWorld();
		robot_manager = new CLINTON_RoboPlayerManager();
		robot_manager.check_respawns();
	}
	
	static CLINTON_BotsManagerEntity GetInstance()
	{
		return s_Instance;
	}
	
	// See: https://community.bistudio.com/wiki/Arma_Reforger:Multiplayer_Scripting#RplRpc
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_AddBots(string faction, int group_setting, bool customNames)
	{
		Print("authority-side code | RpcAsk_AddBots");
		robot_manager.add_bot( faction, group_setting, customNames);
		Replication.BumpMe();
	}
		
	//------------------------------------------------------------------------------------------------
	void RequestAddBots(string faction, int group_setting = 0, bool customNames = false)  // #### Are the different functions necessary?
	{
		Print("client-side code | RequestAddBots");
		Rpc(RpcAsk_AddBots, faction, group_setting, customNames);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRoboManagerUpdated()
	{
		Print("proxy-side code | OnRoboManagerUpdated");	
		SPK_myMenuUI menu = SPK_myMenuUI.GetInstance();
		if (menu)
			menu.UpdatePlayerList();
	}
}