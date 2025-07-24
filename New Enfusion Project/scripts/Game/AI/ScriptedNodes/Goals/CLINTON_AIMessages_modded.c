modded enum EMessageType_Goal
{
	CLINTON
}

class CLINTON_AIMessage : SCR_AIMessageGoal
{
	//------------------------------------------------------------------------------------------------
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_RelatedWaypoint);
	}

	//------------------------------------------------------------------------------------------------
	//! Factory method to create an SCR_AIMessage_Loot instance
	//! \param[in] waypoint the loot waypoint
	//! \return the created instance
	static CLINTON_AIMessage Create(SCR_TimedWaypoint waypoint)
	{
		CLINTON_AIMessage msg = new CLINTON_AIMessage();
		msg.m_RelatedWaypoint = waypoint;
		return msg;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void CLINTON_AIMessage()
	{
		m_MessageType = EMessageType_Goal.CLINTON;
	}
}