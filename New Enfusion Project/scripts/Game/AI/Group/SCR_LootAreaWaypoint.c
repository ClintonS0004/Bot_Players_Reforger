class SCR_CarpoolWaypointClass : SCR_BoardingTimedWaypointClass
{
}

class SCR_CarpoolWaypoint : SCR_BoardingTimedWaypoint
{
	//------------------------------------------------------------------------------------------------
	override SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new SCR_CarpoolWaypointState(groupUtilityComp, this);
	}
}

class SCR_CarpoolWaypointState : SCR_AIWaypointState
{
	protected SCR_AICarpoolActivity m_CarpoolActivity;  // ####

	//------------------------------------------------------------------------------------------------
	override void OnSelected()
	{
		super.OnSelected();

		SCR_AIWaypoint wp = SCR_AIWaypoint.Cast(m_Waypoint);
		if (wp)
		{
			wp.GetOnWaypointPropertiesChanged().Remove(OnWaypointPropertiesChanged);
			wp.GetOnWaypointPropertiesChanged().Insert(OnWaypointPropertiesChanged);
		}

		TryCancelCarpoolActivity();
		AddCarpoolActivity();
	}

	//------------------------------------------------------------------------------------------------
	override void OnDeselected()
	{
		super.OnDeselected();

		TryCancelCarpoolActivity();
	}

	//------------------------------------------------------------------------------------------------
	void OnWaypointPropertiesChanged()
	{
		TryCancelCarpoolActivity();
		AddCarpoolActivity();
	}

	//------------------------------------------------------------------------------------------------
	protected void AddCarpoolActivity()
	{
		float priorityLevel = 0;
		SCR_AIWaypoint wp = SCR_AIWaypoint.Cast(m_Waypoint);
		if (wp)
			priorityLevel = wp.GetPriorityLevel();
		
		int debug_me = this.m_Utility.m_Owner.GetTotalAgentCount();  // =2
		
		//array<SCR_ChimeraCharacter> debug_me_two = this.m_Utility.m_Owner.GetAIMembers();  this too =null
		SCR_AIGroup debug_group = SCR_AIGroup.Cast(m_Utility.GetAIAgent());  // =null but why?
		
		ref SCR_AIGroup gp = this.m_Utility.m_Owner;

		SCR_AICarpoolActivity activity = new SCR_AICarpoolActivity(m_Utility, m_Waypoint, vector.Zero, gp);
		//auto activity = new SCR_AIMoveActivity(m_Utility, m_Waypoint, msg.m_MovePosition,
		//	msg.m_FollowEntity, msg.m_eMovementType, msg.m_bUseVehicles, SCR_AIActionBase.PRIORITY_ACTIVITY_MOVE, priorityLevel: msg.m_fPriorityLevel);
		
		// SCR_AICarpoolActivity(m_Utility, m_Waypoint, priorityLevel: priorityLevel);
		
		m_Utility.AddAction(activity);

		m_CarpoolActivity = activity;
	}

	//------------------------------------------------------------------------------------------------
	protected void TryCancelCarpoolActivity()
	{
		if (!m_CarpoolActivity)
			return;

		if (m_CarpoolActivity.GetActionState() == EAIActionState.FAILED)
			return;

		m_CarpoolActivity.SetFailReason(EAIActionFailReason.CANCELLED);
		m_CarpoolActivity.Fail();
	}
}
