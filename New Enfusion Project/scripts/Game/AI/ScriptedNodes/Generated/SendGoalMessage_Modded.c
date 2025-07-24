// Generated from SCR_AIMessage_Loot class
class SCR_AISendGoalMessage_Carpool : SCR_AISendMessageGenerated
{
	[Attribute("")]
	float m_fPriorityLevel;
	
	[Attribute("")]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.ComboBox, enumType: EMovementType)]
	EMovementType m_eMovementType;
	
	[Attribute("")]
	bool m_bUseVehicles;

	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"FollowEntity",
		"MovePosition",
		"UseVehicles"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }

	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Carpool msg = new SCR_AIMessage_Carpool();

		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);

		msg.SetText(m_sText);

		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("FollowEntity", msg.m_FollowEntity);
		
		GetVariableIn("MovePosition", msg.m_MovePosition);
		
		msg.m_eMovementType = m_eMovementType;
		
		if(!GetVariableIn("UseVehicles", msg.m_bUseVehicles))
			msg.m_bUseVehicles = m_bUseVehicles;
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);

		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}

	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_eMovementType: %1\n", typename.EnumToString(EMovementType, m_eMovementType));
		s = s + string.Format("m_bUseVehicles: %1\n", m_bUseVehicles);
		return s;
	}
	static override bool VisibleInPalette() { return true; }
}

// Generated from SCR_AIMessage_Supply class
class SCR_AISendGoalMessage_Supply : SCR_AISendMessageGenerated
{
	[Attribute("0")]
	protected float m_fPriorityLevel;

	[Attribute("0")]
	protected bool m_bIsWaypointRelated;

	[Attribute()]
	protected IEntity m_DepositTarget;

	protected static ref TStringArray s_aVarsIn = {
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"DepositTarget"
	};

	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Supply msg = new SCR_AIMessage_Supply();

		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);

		msg.SetText(m_sText);

		if (!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;

		if (!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;

		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);

		GetVariableIn("DepositTarget", msg.m_DepositTarget);

		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}

	//------------------------------------------------------------------------------------------------
	override string GetNodeMiddleText()
	{
		return string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel)
			+ string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
	}

	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}
}
