class SCR_CarpoolWaypointClass : SCR_TimedWaypointClass
{
}

/*
 * Car Pool Waypoint
 *  Use this like a move waypoint. The group's units will board vehicles near them if their distance to the waypoint is greater then the value listed somewhere in the behaviou trees
 */

class SCR_CarpoolWaypoint : SCR_TimedWaypoint
{
	//------------------------------------------------------------------------------------------------
	override SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new SCR_CarpoolWaypointState(groupUtilityComp, this);
	}
}

class SCR_CarpoolWaypointState : SCR_AIWaypointState
{
	protected SCR_AICarpoolActivity m_LootActivity;

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

		TryCancelLootActivity();
		AddLootActivity();
	}

	//------------------------------------------------------------------------------------------------
	override void OnDeselected()
	{
		super.OnDeselected();

		TryCancelLootActivity();
	}

	//------------------------------------------------------------------------------------------------
	void OnWaypointPropertiesChanged()
	{
		TryCancelLootActivity();
		AddLootActivity();
	}

	//------------------------------------------------------------------------------------------------
	protected void AddLootActivity()
	{
		float priorityLevel = 80;
		SCR_AIWaypoint wp = SCR_AIWaypoint.Cast(m_Waypoint);
		if (wp)
			priorityLevel = wp.GetPriorityLevel();

		SCR_AICarpoolActivity activity = new SCR_AICarpoolActivity(m_Utility, m_Waypoint, wp.GetOrigin(), priorityLevel: priorityLevel);
		m_Utility.AddAction(activity);

		m_LootActivity = activity;
	}

	//------------------------------------------------------------------------------------------------
	protected void TryCancelLootActivity()
	{
		if (!m_LootActivity)
			return;

		if (m_LootActivity.GetActionState() == EAIActionState.FAILED)
			return;

		m_LootActivity.SetFailReason(EAIActionFailReason.CANCELLED);
		m_LootActivity.Fail();
	}
}
