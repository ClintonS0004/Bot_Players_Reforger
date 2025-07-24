class CLINTON_MoveIndividual : AITaskScripted
{
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		//IEntity myEntity = owner.GetControlledEntity();
		//vector myPos = myEntity.GetOrigin();
		SCR_AIGroup gp = SCR_AIGroup.Cast(owner);
		gp.GetOnAgentAdded().Insert(OnAgentAdded);

		array<SCR_ChimeraCharacter> debug_me = gp.GetAIMembers();
		SCR_AIGroupUtilityComponent groupUtility = gp.GetGroupUtilityComponent();
		array<SCR_AIInfoComponent> debug_me_two = groupUtility.m_aInfoComponents;
		
		PrintFormat("Hello world. Agent %1", owner);
		return ENodeResult.SUCCESS;
	}
	
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	void OnAgentAdded(AIAgent agent)
	{
	// do what you need to do
		PrintFormat("Hello world. Agent %1", agent);
	}
}