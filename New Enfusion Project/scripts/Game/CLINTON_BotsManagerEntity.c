[EntityEditorProps(category: "Tutorial/Entities", description: "Farts and poos")]
class CLINTON_BotsManagerEntityClass : GenericEntityClass
{
	// See: https://community.bistudio.com/wiki/Arma_Reforger:Create_an_Entity
}

class CLINTON_BotsManagerEntity : GenericEntity
{
	protected float m_fWaitingTime = float.INFINITY;	// trigger Print on start
	protected int m_iCycleDuration = 2;				// in seconds
	
	ref CLINTON_RoboPlayerManager robot_manager;
	
	protected static CLINTON_BotsManagerEntity s_Instance;
	
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
		
		SetEventMask(EntityEvent.FRAME);
		
		// Initialise
		World world = GetGame().GetWorld();
		robot_manager = new CLINTON_RoboPlayerManager(world);
		robot_manager.check_respawns();
	}
}