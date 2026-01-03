class SCR_AICarpoolBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParamAssignable<vector> m_vPosition = new SCR_BTParamAssignable<vector>(SCR_AIActionTask.POSITION_PORT);
	protected ref SCR_BTParam<SCR_AIActivityBase> m_Activity = new SCR_BTParam<SCR_AIActivityBase>("Activity");
	protected ref SCR_BTParam<SCR_AIBoardingParameters> m_Boarding = new SCR_BTParam<SCR_AIBoardingParameters>("Boarding Parameters");
	
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] relatedActivity
	void InitParameters(SCR_AIActivityBase relatedActivity, vector position)
	{
		m_vPosition.Init(this, position);
		m_Activity.Init(this, relatedActivity);
		SCR_AIBoardingParameters hello = new SCR_AIBoardingParameters();
		// I'm not sure anything needs to change in SCR_AIBoardingParameters
		m_Boarding.Init(this, hello);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] utility
	//! \param[in] groupActivity
	//! \param[in] relatedWaypoint
	//! \param[in] priority must be between normal Move and AttackMove and replaced by enum later, maybe evaluated dynamically
	//! \param[in] priorityLevel
	void SCR_AICarpoolBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, AIWaypoint relatedWaypoint, vector position, float priority = PRIORITY_BEHAVIOR_MOVE, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_sBehaviorTree = "{C2D96DE4F1B19B4C}AI/BehaviorTrees/Chimera/Soldier/Carpool.bt";
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;

		InitParameters(groupActivity, position);		
	}
}

class SCR_AIGetCarpoolBehaviorParameters : SCR_AIGetActionParameters
{
	protected static ref TStringArray s_aVarsOut = (new SCR_AICarpoolBehavior(null, null, null, vector.Zero)).GetPortNames();

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
