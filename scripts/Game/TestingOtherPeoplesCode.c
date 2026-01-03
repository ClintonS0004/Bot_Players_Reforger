/*
class SF_RandomScenarioManagerClass: GenericEntityClass
{
}

class SF_RandomScenarioManager: GenericEntity
{
    void Start()
    {
         ShowHint()
    }

    protected void ShowHint()
    {
         Rpc(RpcDo_ShowHint);
    }

    [RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
    protected void RpcDo_ShowHint()
    {
			SCR_HintManagerComponent.GetInstance().ShowCustomHint(
                "You have joined the HVK_D15 factionssss!", "Faction  Assigned", 3.0
            );
    }
}

protected ScrServerCmdResult StartScenarioAction(array<string> argv, int playerId = 0)
{
    //DynamicScenarioManager is the name of my spawned entity in world with type SF_RandomScenarioManager
    SF_RandomScenarioManager mng = SF_RandomScenarioManager.Cast(GetGame().GetWorld().FindEntityByName("DynamicScenarioManager"));
    mng.Start();

    return ScrServerCmdResult(string.Format("Scenario started"), EServerCmdResultType.OK);
}*/