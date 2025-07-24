modded enum EMessageType_Goal
{
	CARPOOL,
	SUPPLY,
}

class SCR_AIMessage_Carpool : SCR_AIMessageGoal
{
	IEntity m_FollowEntity; // VARIABLE(NodePort, FollowEntity)
	vector m_MovePosition; // VARIABLE(NodePort, MovePosition)
	EMovementType m_eMovementType; // VARIABLE(NodePropertyEnum, m_eMovementType)
	bool m_bUseVehicles; // VARIABLE(NodePort, UseVehicles, NodeProperty, m_bUseVehicles)	
	
	void SCR_AIMessage_Carpool()
	{
		m_MessageType = EMessageType_Goal.CARPOOL;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_FollowEntity);
		if (!node.GetVariableIn(node.PORT_VECTOR, m_MovePosition))
			m_MovePosition = node.m_vector;
		if (!node.GetVariableIn(node.PORT_BOOL, m_bUseVehicles))
			m_bUseVehicles = node.m_bool;
		if (!node.GetVariableIn(node.PORT_PRIORITY_LEVEL,m_fPriorityLevel))
			m_fPriorityLevel = node.m_fPriorityLevel;
	}

	static SCR_AIMessage_Carpool Create(IEntity entity, vector position, EMovementType movementType, bool useVehicles, SCR_AIActivityBase relatedActivity)
	{
		SCR_AIMessage_Carpool msg = new SCR_AIMessage_Carpool();
		msg.m_FollowEntity = entity;
		msg.m_MovePosition = position;
		msg.m_eMovementType = movementType;
		msg.m_bUseVehicles = useVehicles;
		msg.m_RelatedGroupActivity = relatedActivity;
		return msg;
	}
	//------------------------------------------------------------------------------------------------
	//! Factory method to create an SCR_AIMessage_Loot instance
	//! \param[in] waypoint the loot waypoint
	//! \return the created instance 
	/*
	static SCR_AIMessage_Carpool Create(SCR_CarpoolWaypoint waypoint)
	{
		SCR_AIMessage_Carpool msg = new SCR_AIMessage_Carpool();
		msg.m_RelatedWaypoint = waypoint;
		return msg;
	}
	*/
};

class SCR_AIMessage_Supply : SCR_AIMessageGoal
{
	IEntity m_DepositTarget; // not protected

	//------------------------------------------------------------------------------------------------
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_DepositTarget);
	}

	//------------------------------------------------------------------------------------------------
	//! Factory method to create an SCR_AIMessage_Supply instance
	//! \param[in] depositTarget the deposit target
	//! \return the created instance
	static SCR_AIMessage_Supply Create(IEntity depositTarget)
	{
		SCR_AIMessage_Supply msg = new SCR_AIMessage_Supply();
		msg.m_DepositTarget = depositTarget;
		return msg;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_AIMessage_Supply()
	{
		m_MessageType = EMessageType_Goal.SUPPLY;
	}
}
