// Generated from SCR_AIMessage_Loot class
class SCR_AISendGoalMessage_Carpool : SCR_AISendMessageGenerated
{
	[Attribute("0")]
	protected float m_fPriorityLevel;

	[Attribute("0")]
	protected bool m_bIsWaypointRelated;
	
	[Attribute("0")]
	protected vector m_vPosition;

	protected static ref TStringArray s_aVarsIn = {
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"Waypoint Origin"
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
		SCR_AIMessage_Carpool msg = new SCR_AIMessage_Carpool();
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);

		msg.SetText(m_sText);

		if (!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		msg.m_fPriorityLevel = 200.0;

		if (!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;

		if (!GetVariableIn("Waypoint Origin", msg.m_vPosition))
			msg.m_vPosition = m_vPosition;
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (msg.m_MessageType != 31)
			Print("Warning enums are working funni", LogLevel.WARNING);
		
		//SCR_AIConfigComponent cfgComp = SCR_AIConfigComponent.Cast(IEntity.Cast(owner).FindComponent(SCR_AIConfigComponent));
		SCR_AIConfigComponent cfgComp;
		
		/* When run on units
		AIGroup grp = AIGroup.Cast(owner);
		array<AIAgent> outAgents = new array<AIAgent>();
		grp.GetAgents(outAgents);
		if( !outAgents ) Print("Warning no Agents in this Group", LogLevel.WARNING);*/
		
		array<AIAgent> outAgents = new array<AIAgent>();
		AIGroup grp = AIGroup.Cast(owner);
		grp.GetAgents(outAgents);
		if( !outAgents ) Print("Warning no Agents in this Group", LogLevel.WARNING);
		
		foreach( AIAgent a : outAgents )  // Is this meant to be run on a group ( /run once? )
		{
			if (!a)
				Print("Error SendGoalMessage_CLINTON.c",LogLevel.ERROR);
			
			SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(a.FindComponent(SCR_AIUtilityComponent));
			if (!utility)
				Print("Error SendGoalMessage_CLINTON.c no Agent utilities",LogLevel.ERROR);
			
			cfgComp = SCR_AIConfigComponent.Cast(IEntity.Cast(a).FindComponent(SCR_AIConfigComponent));
			if (!cfgComp)
				Print("Error SendGoalMessage_CLINTON.c can't find config",LogLevel.ERROR);
			
			cfgComp.PerformGoalReaction(utility, msg);  // is the config a group? should it be for a unit?
		}
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}

	//------------------------------------------------------------------------------------------------
	override string GetNodeMiddleText()
	{
		return string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel)
			+ string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated)
			+ string.Format("m_vPosition: %1\n", m_vPosition);
	}

	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}
}