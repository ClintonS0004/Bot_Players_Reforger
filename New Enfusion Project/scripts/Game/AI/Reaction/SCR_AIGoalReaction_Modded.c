[BaseContainerProps()]
class SCR_AIGoalReaction_Carpool : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Carpool.Cast(message);
		if (!msg)
			return;
		
		auto behavior = new SCR_AICarpoolBehavior(utility, msg.m_RelatedGroupActivity, msg.m_MovePosition);

		utility.WrapBehaviorOutsideOfVehicle(behavior);
		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{		
		auto msg = SCR_AIMessage_Carpool.Cast(message);
		if (!msg)
			return;
		if(!msg.m_MovePosition)
			msg.m_MovePosition = msg.m_FollowEntity.GetOrigin();
		
		auto activity = new SCR_AICarpoolActivity(utility, msg.m_RelatedWaypoint, msg.m_MovePosition,
			msg.m_FollowEntity, msg.m_eMovementType, msg.m_bUseVehicles, SCR_AIActionBase.PRIORITY_ACTIVITY_MOVE, priorityLevel: msg.m_fPriorityLevel);
		
		utility.SetStateAllActionsOfType(SCR_AISearchAndDestroyActivity,EAIActionState.FAILED); // move fails seek and destroy
		utility.AddAction(activity);
	}
	
}

[BaseContainerProps()]
class SCR_AIGoalReaction_Supply : SCR_AIGoalReaction
{
	//------------------------------------------------------------------------------------------------
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_Supply msg = SCR_AIMessage_Supply.Cast(message);
		if (!msg)
			return;

		utility.AddAction(new SCR_AISupplyBehavior(utility, msg.m_RelatedGroupActivity, msg.m_DepositTarget));
	}
}
