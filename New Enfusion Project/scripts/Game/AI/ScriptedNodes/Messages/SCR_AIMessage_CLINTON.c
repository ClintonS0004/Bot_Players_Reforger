modded enum EMessageType_Goal
{
	CARPOOL,
}

class SCR_AIMessage_Carpool : SCR_AIMessageGoal
{
	vector m_vPosition;
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
	static SCR_AIMessage_Carpool Create(SCR_CarpoolWaypoint waypoint)
	{
		SCR_AIMessage_Carpool msg = new SCR_AIMessage_Carpool();
		msg.m_RelatedWaypoint = waypoint;
		return msg;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_AIMessage_Carpool()
	{
		m_MessageType = EMessageType_Goal.CARPOOL;
	}
}