class SCR_AISupplyBehavior : SCR_AIBehaviorBase
{
	protected ref SCR_BTParam<IEntity> m_DepositTarget = new SCR_BTParam<IEntity>(SCR_AIActionTask.TARGET_PORT);

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] ent
	void InitParameters(IEntity ent)
	{
		m_DepositTarget.Init(this, ent);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] utility
	//! \param[in] groupActivity unused
	//! \param[in] depositTarget
	//! \param[in] priority must be between normal Move and AttackMove and replaced by enum later, maybe evaluated dynamically
	//! \param[in] priorityLevel
	void SCR_AISupplyBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, IEntity depositTarget, float priority = PRIORITY_BEHAVIOR_MOVE, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Supply.bt";

		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;

		InitParameters(depositTarget);
	}
}

class SCR_AIGetSupplyBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AISupplyBehavior(null, null, null)).GetPortNames();

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
