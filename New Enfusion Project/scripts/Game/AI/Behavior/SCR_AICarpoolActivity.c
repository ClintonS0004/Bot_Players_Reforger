class SCR_AICarpoolActivity : SCR_AIActivityBase
{
	protected ref SCR_BTParam<AIWaypoint> m_Waypoint = new SCR_BTParam<AIWaypoint>(SCR_AIActionTask.WAYPOINT_PORT);
	protected ref SCR_BTParam<vector> m_Position = new SCR_BTParam<vector>(SCR_AIActionTask.POSITION_PORT);
	const static float PRIORITY_ACTIVITY_CARPOOL					= 80;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] relatedWaypoint
	//! \param[in] priorityLevel
	void InitParameters(AIWaypoint relatedWaypoint, float priorityLevel, vector position)
	{
		m_Waypoint.Init(this, relatedWaypoint);
		m_Position.Init(this, position);
		m_fPriorityLevel.Init(this, priorityLevel);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] utility
	//! \param[in] relatedWaypoint
	//! \param[in] priority
	//! \param[in] priorityLevel
	void SCR_AICarpoolActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, vector position, float priority = PRIORITY_ACTIVITY_CARPOOL, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_sBehaviorTree = "{29A0ABBFA6056863}AI/BehaviorTrees/Chimera/Group/ActivityCarpool.bt";
		SetPriority(priority);
		InitParameters(relatedWaypoint, priorityLevel, position);

		AIWaypoint wp = relatedWaypoint;
		if (wp)
			m_Waypoint.m_Value = SCR_CarpoolWaypoint.Cast(wp);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		SendCancelMessagesToAllAgents();
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		SendCancelMessagesToAllAgents();
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		SendCancelMessagesToAllAgents();
	}
}

class SCR_AICarpoolActivityParameters : SCR_AIGetActionParameters
{
	protected static ref TStringArray s_aVarsOut = (new SCR_AICarpoolActivity(null, null, vector.Zero)).GetPortNames();

	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}

	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}
}

class SCR_AISetCarpoolActivityParameters : SCR_AISetActionParameters
{
	protected static ref TStringArray s_aVarsIn = (new SCR_AICarpoolActivity(null, null, vector.Zero)).GetPortNames();

	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}

	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}
}
