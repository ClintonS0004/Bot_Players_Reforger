class BotsWorldSystem : BaseSystem // 1.
{
	//------------------------------------------------------------------------------------------------
    override static void InitInfo(WorldSystemInfo outInfo)
    {
        // 2.
        outInfo
            .SetAbstract(false)
            .SetLocation(ESystemLocation.Server)
            .AddPoint(ESystemPoint.Frame)
            .AddController(BotsWorldController);
    }

	//------------------------------------------------------------------------------------------------
    override void OnInit()
	{
		done = false;
		Print("OnInit 	| 	BotsWorldSystem");
	}

    static ref map<RplIdentity, BotsWorldController> m_Controllers = new map<RplIdentity, BotsWorldController>();
	static bool done;
	// [RplProp(onRplName: "OnNumberChanged")]
	static int m_iNumber;
	static ref CLINTON_VirtualPlayerManager m_VirtualPlayerManager;
	
	//------------------------------------------------------------------------------------------------
	void SetNumber(int x)
	{
		m_iNumber = x;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddBot(string faction, int group_setting, bool customNames, int loadouts_setting)
	{
		m_VirtualPlayerManager.add_bot(faction, group_setting, customNames, loadouts_setting);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveBot(string faction)
	{
		m_VirtualPlayerManager.remove_bot(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	void ChangeBotName(int id, string name)
	{
		m_VirtualPlayerManager.rename_bot(id, name);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnNumberChanged()
	{
		//auto serverSideOwner = this.GetOwnerPlayerId();
        // This message will appear on server and all clients.
        //PrintFormat("Player %1 and number equals '%2'", serverSideOwner, m_iNumber);
		Print("Number is at %1",m_iNumber);
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void OnUpdate(ESystemPoint point)
    {
		if (!done)
		{
	        WorldSystem m_system = WorldSystem.Cast(this);
			if (!m_system) 
				Print("NNNNOOOOOOOO!",LogLevel.ERROR);
			RplRole mode = m_system.GetNode().GetRole();
			WorldSystems systems = m_system.GetSystems();
			BotsWorldController controller  = BotsWorldController.Cast(systems.FindMyController(BotsWorldController));
			if (mode == RplMode.None || mode == RplMode.Listen || mode == RplMode.Client) // <--- Modified || mode == RplMode.Client 
	        {
				//controller = m_Controllers.Get(RplIdentity.Local());
				//m_Controllers.Set(RplIdentity.Local(), controller);
	        }
	        if (mode == RplRole.Authority)
	        {
				m_iNumber = 0;
				m_VirtualPlayerManager = new CLINTON_VirtualPlayerManager();
				//controller.RequestNameChange("This is my name.");
				done = true;
	        } else {
				done = true;
			}
		}
    }
	//------------------------------------------------------------------------------------------------
    private bool RplGiven(ScriptBitReader reader)
    {
        Print("PlayerNameInputController.RplGiven()");
        return true;
    }
}

class BotsWorldController : WorldController // 1.
{
	//[RplProp(onRplName: "OnNumberRecieved")]
	static int numberCopy;
	//------------------------------------------------------------------------------------------------
	void OnNumberRecieved()
	{
		Print(numberCopy.ToString());
	}
	//------------------------------------------------------------------------------------------------
	void RequestGetNumber()
	{
		Rpc(Rpc_GetNumber_S);
	}
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void Rpc_GetNumber_S()
    {
		BotsWorldSystem botSystem = BotsWorldSystem.Cast(
			GetGame().GetWorld().FindSystem(BotsWorldSystem));
		numberCopy = botSystem.m_iNumber;
		//Replication.BumpMe();
		Rpc(Rpc_SendResultNumber_O, numberCopy);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void Rpc_SendResultNumber_O(int num)
	{
		numberCopy = num;
//		Replication.BumpMe();
		
		// Here you should start your action
		
	}
	
	// Testing that the server can send back new values
	//------------------------------------------------------------------------------------------------
	void RequestGetNumberForMenu()
	{
		Rpc(Rpc_GetNumberForMenu_S);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void Rpc_GetNumberForMenu_S()
    {
		BotsWorldSystem botSystem = BotsWorldSystem.Cast(
			GetGame().GetWorld().FindSystem(BotsWorldSystem));
		numberCopy = botSystem.m_iNumber;
		Rpc(Rpc_SendResultNumberForMenu_O, numberCopy);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void Rpc_SendResultNumberForMenu_O(int num)
	{
		numberCopy = num;
		// Here you should start your action
		// Check that the Menu is open
		CLINTON_BotMenuUI ins = CLINTON_BotMenuUI.GetInstance();
		if (ins)
		{
			ins.RecieveNumber(num);
		} // if else just ignore
	}
	
	array<ref CLINTON_Virtual_Player> botsListForMenu;
	
	// Client's request a copy of the list from the server.
	// The server uses Rpc() to send back a copy.
	//------------------------------------------------------------------------------------------------
	void RequestGetPlayerManagerForMenu()
	{
		Rpc(Rpc_GetPlayerManagerForMenu_S);
	}
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void Rpc_GetPlayerManagerForMenu_S()
    {
		BotsWorldSystem botSystem = BotsWorldSystem.Cast(
			GetGame().GetWorld().FindSystem(BotsWorldSystem));
		botsListForMenu = botSystem.m_VirtualPlayerManager.GetPlayers();
		Rpc(Rpc_SendResultPlayerManagerForMenu_O, botsListForMenu);
		CLINTON_BotMenuUI ins = CLINTON_BotMenuUI.GetInstance();
		if (ins)
		{
			//ins.RecievePlayerList_Open(botsListForMenu);
		} // if else just ignore
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void Rpc_SendResultPlayerManagerForMenu_O(array<ref CLINTON_Virtual_Player> num) // am I meant to remove the ref?
	{
		botsListForMenu = num;
		// Here you should start your action
		// Check that the Menu is open
		CLINTON_BotMenuUI ins = CLINTON_BotMenuUI.GetInstance();
		if (ins)
		{
			ins.RecievePlayerList_Open(botsListForMenu);
		} // if else just ignore
	}
		
	//------------------------------------------------------------------------------------------------
	void RequestGetAddMenu()
	{
		Rpc(Rpc_GetAddMenu_S);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void Rpc_GetAddMenu_S()
    {
		BotsWorldSystem botSystem = BotsWorldSystem.Cast(
			GetGame().GetWorld().FindSystem(BotsWorldSystem));
		
		botsListForMenu = botSystem.m_VirtualPlayerManager.GetPlayers();

		Rpc(Rpc_SendResultAddMenu_O, botsListForMenu);
		CLINTON_BotMenuUI ins = CLINTON_BotMenuUI.GetInstance();
		if (ins)
		{
			ins.RecievePlayerList_Add(botsListForMenu);
		} // if else just ignore
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void Rpc_SendResultAddMenu_O(array<ref CLINTON_Virtual_Player> copyMenu)
	{
		botsListForMenu = copyMenu;
		// Here you should start your action
		// Check that the Menu is open
		CLINTON_BotMenuUI ins = CLINTON_BotMenuUI.GetInstance();
		if (ins)
		{
			ins.RecievePlayerList_Add(botsListForMenu);
		} // if else just ignore
	}
	
	
	//------------------------------------------------------------------------------------------------
	void RequestGetRemoveMenu()
	{
		Rpc(Rpc_GetRemoveMenu_S);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void Rpc_GetRemoveMenu_S()
    {
		BotsWorldSystem botSystem = BotsWorldSystem.Cast(
			GetGame().GetWorld().FindSystem(BotsWorldSystem));
		
		botsListForMenu = botSystem.m_VirtualPlayerManager.GetPlayers();

		Rpc(Rpc_SendResultRemoveMenu_O, botsListForMenu);
		CLINTON_BotMenuUI ins = CLINTON_BotMenuUI.GetInstance();
		if (ins)
		{
			ins.RecievePlayerList_Subtract(botsListForMenu);
		} // if else just ignore
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void Rpc_SendResultRemoveMenu_O(array<ref CLINTON_Virtual_Player> copyMenu)
	{
		botsListForMenu = copyMenu;
		// Here you should start your action
		// Check that the Menu is open
		CLINTON_BotMenuUI ins = CLINTON_BotMenuUI.GetInstance();
		if (ins)
		{
			ins.RecievePlayerList_Subtract(copyMenu);
		} // if else just ignore
	}
		
	//------------------------------------------------------------------------------------------------
	void RequestSetNumber(int x)
	{
		Rpc(Rpc_SetNumber_S, x);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void Rpc_SetNumber_S(int x)
    {
		BotsWorldSystem botSystem = BotsWorldSystem.Cast(
			GetGame().GetWorld().FindSystem(BotsWorldSystem));
		botSystem.SetNumber(botSystem.m_iNumber + 1);
		
		//m_iNumber = x;
		//Replication.BumpMe();
		//this.OnNumberChanged();
	}
	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldControllerInfo outInfo)
    {
        outInfo.SetPublic(true); // 3.
    }
	
	//------------------------------------------------------------------------------------------------
	void RequestAddBots(string faction, int group_setting = 0, bool customNames = false, int loadouts_setting = 0)
	{
		Rpc(Rpc_AddBots_S, faction, group_setting, customNames, loadouts_setting);
	}

	// When you add or subtract bots. You will seperately ask for an updated version of the player list.
	// This allows you to broadcast to all players who have the menu open
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void Rpc_AddBots_S(string faction, int group_setting, bool customNames, int loadouts_setting)
    {
		BotsWorldSystem botSystem = BotsWorldSystem.Cast(
			GetGame().GetWorld().FindSystem(BotsWorldSystem));
		botSystem.AddBot(faction, group_setting, customNames, loadouts_setting);
		// this.OnVirtualPlayerManagerChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestRemoveBots(string faction)
	{
		Rpc(Rpc_RemoveBots_S, faction);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void Rpc_RemoveBots_S(string faction)
    {
		BotsWorldSystem botSystem = BotsWorldSystem.Cast(
			GetGame().GetWorld().FindSystem(BotsWorldSystem));
		botSystem.RemoveBot(faction);
		// this.OnVirtualPlayerManagerChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnAuthorityReady()
    {
		Print("PlayerNameInputController.OnAuthorityReady()");
    }
 
	//------------------------------------------------------------------------------------------------
    private bool RplGiven(ScriptBitReader reader)
    {
		Print("PlayerNameInputController.RplGiven()");
        return true;
    }
	
	// [RplProp(onRplName: "OnPlayerNameChanged")] **** Maybe this needs the new id+name params
    string m_PlayerName;
 
    void RequestNameChange(int botId, string newPlayerName)
    {
        Rpc(Rpc_NameChange_S, botId, newPlayerName);
    }
 
    [RplRpc(RplChannel.Reliable, RplRcver.Server)]
    private void Rpc_NameChange_S(int botId, string newPlayerName)
    {
        //if (m_PlayerName == newPlayerName)
        //    return;
 		BotsWorldSystem botSystem = BotsWorldSystem.Cast(
			GetGame().GetWorld().FindSystem(BotsWorldSystem));
		botSystem.ChangeBotName(botId, newPlayerName);
		
		Rpc(Rpc_NameChangeMenu_O, botId, newPlayerName);
		
		// Check that the Menu is open
		CLINTON_BotMenuUI ins = CLINTON_BotMenuUI.GetInstance();
		if (ins)
		{
			ins.RecievePlayerUpdate(botId, newPlayerName);
		} // if else just ignore
		
        auto serverSideOwner = this.GetOwnerPlayerId();
        // This message will appear on server and all clients.
        PrintFormat("Player %1 bot ID %2 name %3", serverSideOwner, botId, newPlayerName);
		//this.m_PlayerName = m_PlayerName;
		
        //m_PlayerName = newPlayerName;
        //Replication.BumpMe();
        // We invoke callback explicitly on server. On clients, it will be invoked
        // automatically when replication changes m_PlayerName value.
        //this.OnPlayerNameChanged();
    }
 
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
    private void Rpc_NameChangeMenu_O(int botId, string newName)  // Hopefully this runs on everyone
    {
		// Check that the Menu is open
		CLINTON_BotMenuUI ins = CLINTON_BotMenuUI.GetInstance();
		if (ins)
		{
			ins.RecievePlayerUpdate(botId, newName);
		} // if else just ignore
		
        auto serverSideOwner = this.GetOwnerPlayerId();
        // This message will appear on server and all clients.
        PrintFormat("Player %1 bot ID %2 name %3", serverSideOwner, botId, newName);
		//this.m_PlayerName = m_PlayerName;
    }
}