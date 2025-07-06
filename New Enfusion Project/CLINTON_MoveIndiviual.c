class CLINTON_MoveIndividual : AITaskScripted
{
override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		PrintFormat("Hello world. Agent %1", owner);
		return ENodeResult.SUCCESS;
	}
	
protected static override bool VisibleInPalette(){
	return true;}
}