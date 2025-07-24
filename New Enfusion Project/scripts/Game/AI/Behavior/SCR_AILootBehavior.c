class SCR_AICarpoolBehavior : SCR_AIMoveBehaviorBase
{ 
	ref SCR_BTParamAssignable<IEntity> m_Entity = new SCR_BTParamAssignable<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParamAssignable<float> m_Radius = new SCR_BTParamAssignable<float>(SCR_AIActionTask.RADIUS_PORT);
		
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveIndividuallyBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector pos, float priority = PRIORITY_BEHAVIOR_MOVE_INDIVIDUALLY, float priorityLevel = PRIORITY_LEVEL_NORMAL, IEntity ent = null, float radius = 1.0)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/CLINTON_MoveIndividually.bt";
		m_Entity.Init(this, ent);
		if (ent)
			m_vPosition.m_Value = ent.GetOrigin();
		m_Radius.Init(this, radius);
	}
	
	//-----------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " moving to " + m_vPosition.m_Value.ToString();
	}
};