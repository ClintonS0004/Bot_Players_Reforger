class CLINTON_AIDecoTestGroupStillCarpoolCondition : DecoratorScripted
{
	protected override bool TestFunction(AIAgent owner)
	{
		array<AIAgent> outAgents = new array<AIAgent>();
		AIGroup grp = AIGroup.Cast(owner);
		grp.GetAgents(outAgents);
		if( !outAgents ) 
			Print("Warning no Agents in this Group", LogLevel.WARNING);
		foreach( AIAgent a : outAgents )
		{
			SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(a.FindComponent(SCR_AIUtilityComponent));
			if (!utility)
				Print("Error SendGoalMessage_CLINTON.c no Agent utilities",LogLevel.ERROR);
			AIActionBase current_action = utility.GetCurrentAction();
			if(SCR_AICarpoolBehavior.Cast(current_action) || SCR_AIGetInVehicle.Cast(current_action))
				return false;
		}
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}	
	
	//-----------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Checks time since target was detected and compares it with threshold. Returns true when time is below the threshold.";
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		
	};
	protected override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
}