class SCR_AITestHelloWorld : AITaskScripted
{
	//------------------------------------------------------------------------------------------------
	// this event is triggered when tree is loaded or executed by RunBT node
	// again it is triggered if RunBT has attribute "always reinit"
	//! \param[in] owner - AIAgent on which the node is executed
	override void OnInit(AIAgent owner)
	{
		super.OnInit(owner);
	}
		
	//------------------------------------------------------------------------------------------------
	// this event is triggered on start of simulation if the node state is not "RUNNING" 
	//! \param[in] owner - AIAgent on which the node is executed
	override void OnEnter(AIAgent owner)
	{
		super.OnEnter(owner);
	}	

	//------------------------------------------------------------------------------------------------
	// this event is triggered when node is simulated	
	//! \param[in] owner AIAgent on which the node is executed
	//! \param[in] dt time delta between last update of tree simulation and current simulation
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		PrintFormat("Hello world! Agent %1", owner);
		return ENodeResult.SUCCESS;	
	}

	//------------------------------------------------------------------------------------------------
	// this event is triggered when parent calls Abort on child nodes	
	//! \param[in] owner AIAgent on which the node is executed
	//! \param[in] nodeCausingAbort usually decorator that called abort
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		super.OnAbort(owner, nodeCausingAbort);		
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Testing Node to say hello world!";
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText()
	{
		return "Hello World!";
	}
}