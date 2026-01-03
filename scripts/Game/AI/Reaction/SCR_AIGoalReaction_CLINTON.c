[BaseContainerProps()]
class SCR_AIGoalReaction_Carpool : SCR_AIGoalReaction
{
	void SCR_AIGoalReaction_Carpool(){
		return;
	}
	//------------------------------------------------------------------------------------------------
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_Carpool msg = SCR_AIMessage_Carpool.Cast(message);
		if (!msg)
			return;
		Print("It's working!", LogLevel.ERROR);

		utility.AddAction(new SCR_AICarpoolBehavior(utility, msg.m_RelatedGroupActivity, msg.m_RelatedWaypoint, msg.m_vPosition));
	}
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{		
		auto msg = SCR_AIMessage_Carpool.Cast(message);
		if (!msg)
			return;
		Print("Shit the bed!", LogLevel.ERROR);
	}
}