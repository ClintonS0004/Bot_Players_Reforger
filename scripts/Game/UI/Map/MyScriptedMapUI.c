modded enum ChimeraMenuPreset {
    SCR_MapsBotsMenuUI
}

//------------------------------------------------------------------------------------------------
class CLINTON_WidgetOnClickHandler : ScriptedWidgetEventHandler
{
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w,x,y,button);
		return true;
	}
	
	override bool OnFocusLost( Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		return true;
	}
}

class myNewLayoutClass: SCR_SuperMenuBase
{
  //your GUI code ...    
	
	// Add GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.myTestGui_ID); 
	// somewhere else
}

class SCR_BotPlayerListEntry
{
	SCR_ScoreInfo m_Info;
	int m_iID;
	Widget m_wRow;
	Faction m_Faction;
	int m_iSortFrequency;
	bool m_toBeDeleted;

	SCR_ButtonBaseComponent m_Mute;
	SCR_ButtonBaseComponent m_Friend;
	SCR_ComboBoxComponent m_PlayerActionList;
	SCR_ComboBoxComponent m_LoadoutPreferenceList;
	SCR_ComboBoxComponent m_SpecLoadoutList;

	EditBoxWidget m_wName;
	TextWidget m_wKills;
	TextWidget m_wFreq;
	TextWidget m_wDeaths;
	TextWidget m_wScore;
	ImageWidget m_wLoadoutIcon;
	ImageWidget m_wPlatformIcon;
	Widget m_wTaskIcon;
	Widget m_wFactionImage;
	Widget m_wVotingNotification;
	Widget m_wBlockedIcon;
};

class CLINTON_BotMenuUI: SCR_SuperMenuBase
{    
	// please copy SCR_PlayerListMenu into here
	// this class is registered in chimeraMenus.conf
		
	protected ref array<ref SCR_BotPlayerListEntry> m_aEntries = new array<ref SCR_BotPlayerListEntry>();
	protected ref map<int, SCR_ScoreInfo> m_aAllPlayersInfo = new map<int, SCR_ScoreInfo>();
	// protected ResourceName m_sScoreboardRow = "{65369923121A38E7}UI/layouts/Menus/PlayerList/PlayerListEntry.layout";
	protected ResourceName m_sScoreboardRow = "{DC9E20F1ACA98BC4}UI/layouts/Menus/PlayerList/BotPlayerListEntry.layout";
	protected Widget m_wTable;
	protected ref array<Faction> m_aFactions = {null};
	
	protected static const ResourceName FACTION_COUNTER_LAYOUT = "{5AD2CE85825EDA11}UI/layouts/Menus/PlayerList/FactionPlayerCounter.layout";
	
	protected string m_sGameMasterIndicatorName = "GameMasterIndicator";
	
	protected const int DEFAULT_SORT_INDEX = 1;
	
	protected SCR_InputButtonComponent m_Friend;
	protected SCR_InputButtonComponent m_Block;
	protected SCR_InputButtonComponent m_Mute;
	protected SCR_InputButtonComponent m_Vote;
	protected SCR_InputButtonComponent m_Invite;
 	protected SCR_InputButtonComponent m_Unblock;
	protected SCR_InputButtonComponent m_ViewProfile;
	
	protected SCR_VotingManagerComponent m_VotingManager;
	protected SCR_PlayerControllerGroupComponent m_PlayerGroupController;
	
	protected PlayerController m_PlayerController;
	protected SCR_BaseScoringSystemComponent m_ScoringSystem;
	SCR_SortHeaderComponent m_Header;
	protected SCR_BotPlayerListEntry m_SelectedEntry;
	protected SocialComponent m_SocialComponent;
	
	protected const string FILTER_NAME = "Name";
	protected const string FILTER_FREQ = "Freq";
	
	protected const string MUTE = "#AR-PlayerList_Mute";
	protected const string UNMUTE = "#AR-PlayerList_Unmute";
	protected const string ADD_FRIEND = "#AR-PlayerList_AddFriend";
	protected const string REMOVE_FRIEND = "#AR-PlayerList_RemoveFriend";
	
	protected static ref ScriptInvoker s_OnPlayerListMenu = new ScriptInvoker();
	
	protected ref Color m_PlayerNameSelfColor = new Color(0.898, 0.541, 0.184, 1);
	
	protected static CLINTON_BotMenuUI s_Instance;
	
	static CLINTON_VirtualPlayerManager pm;
	static string testName;
	
	static ref array<string> m_mBotNames = new array<string>();
	
	void CLINTON_BotMenuUI()
	{
		if (s_Instance)
		{
			Print("Only one instance of CLINTON_BotsManagerEntity is allowed in the world!", LogLevel.WARNING);
			delete this;
			return;
		}

		s_Instance = this;
	}
	
	static CLINTON_BotMenuUI GetInstance()
	{
		return s_Instance;
	}
	
	/*!
	Get event called when player list opens or closes.
	\return Script invoker
	*/
	static ScriptInvoker GetOnPlayerListMenu()
	{
		return s_OnPlayerListMenu;
	}

	//------------------------------------------------------------------------------------------------
	protected void InitSorting()
	{
		if (!GetRootWidget())
			return;

		Widget w = GetRootWidget().FindAnyWidget("SortHeader");
		if (!w)
			return;

		m_Header = SCR_SortHeaderComponent.Cast(w.FindHandler(SCR_SortHeaderComponent));
		if (!m_Header)
			return;

		m_Header.m_OnChanged.Insert(OnHeaderChanged);

		if (m_ScoringSystem)
			return;

		// Hide K/D/S sorting headers if the re is no scoreboard
		ButtonWidget sortKills = ButtonWidget.Cast(w.FindAnyWidget("sortKills"));
		ButtonWidget sortDeaths = ButtonWidget.Cast(w.FindAnyWidget("sortDeaths"));
		ButtonWidget sortScore = ButtonWidget.Cast(w.FindAnyWidget("sortScore"));

		if (sortKills)
			sortKills.SetOpacity(0);
		if (sortDeaths)
			sortDeaths.SetOpacity(0);
		if (sortScore)
			sortScore.SetOpacity(0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHeaderChanged(SCR_SortHeaderComponent sortHeader)
	{
		string filterName = sortHeader.GetSortElementName();
		bool sortUp = sortHeader.GetSortOrderAscending();
		Sort(filterName, sortUp);
	}

	
	//------------------------------------------------------------------------------------------------
	void SortByName(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;

		array<string> names = {};
		foreach (SCR_BotPlayerListEntry entry : m_aEntries)
		{
			if (entry.m_wName)
				names.Insert(entry.m_wName.GetText());
		}

		names.Sort();

		foreach (SCR_BotPlayerListEntry entry : m_aEntries)
		{
			if (!entry.m_wName)
				continue;

			string text = entry.m_wName.GetText();

			foreach (int i, string s : names)
			{
				if (s != text)
					continue;

				if (entry.m_wRow)
					entry.m_wRow.SetZOrder(i * direction);
				continue;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void SortByFrequency(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;

		foreach (SCR_BotPlayerListEntry entry : m_aEntries)
		{
			entry.m_wRow.SetZOrder(entry.m_iSortFrequency * direction);
		}
	}
	//------------------------------------------------------------------------------------------------
	protected void Sort(string filterName, bool sortUp)
	{
		if (filterName == FILTER_NAME)
			SortByName(sortUp);
		else if (filterName == FILTER_FREQ)
			SortByFrequency(sortUp);
	}
	
	//------------------------------------------------------------------------------------------------
	/*CLINTON_VirtualPlayerManager GetBotsManager()
	{
		//World world = GetGame().GetWorld();
		//BotsWorldSystem system = BotsWorldSystem.Cast(world.FindSystem(BotsWorldSystem));
		
		BotsWorldController controller = GetBotsController();
		CLINTON_VirtualPlayerManager manny = controller.GetVirtualPlayerManager();
		
		return manny;
	} */
	
	//------------------------------------------------------------------------------------------------
	BotsWorldController GetBotsController()
	{
		BotsWorldController controller = BotsWorldController.Cast(
		GetGame().GetWorld().GetSystems().FindMyController(BotsWorldController));
		if (!controller)
			Print("No Controller Found! | MyScriptedMapUI.c", LogLevel.ERROR);
		return controller;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		//if (!m_ChatPanel)
		//	m_ChatPanel = SCR_ChatPanel.Cast(m_wRoot.FindAnyWidget("ChatPanel").FindHandler(SCR_ChatPanel));		
		
		m_PlayerController = GetGame().GetPlayerController();
		if (m_PlayerController)
			m_SocialComponent = SocialComponent.Cast(m_PlayerController.FindComponent(SocialComponent));
		
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.Cast(m_PlayerController.FindComponent(SCR_HUDManagerComponent));
		hudManager.SetVisibleLayers(hudManager.GetVisibleLayers() & ~EHudLayers.HIGH);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		m_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		m_PlayerController = GetGame().GetPlayerController();


		FactionManager fm = GetGame().GetFactionManager();
		if (fm)
		{
			fm.GetFactionsList(m_aFactions);
		}

		m_wTable = GetRootWidget().FindAnyWidget("Table");

		// Create navigation buttons
		Widget footer = GetRootWidget().FindAnyWidget("FooterLeft");
		Widget footerBack = GetRootWidget().FindAnyWidget("Footer");
		SCR_InputButtonComponent back = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK, footerBack);
		if (back)
		{
			back.m_OnActivated.Insert(OnBack);  // It's OnActivated not OnClicked
			
			//back.SetLabel("Make my Tab");
		}
		// Set button actionsS
		/*
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sScoreboardRow, m_wTable);
		SCR_ButtonBaseComponent add = SCR_ButtonBaseComponent.GetButtonBase("Mute", w);
		if (add)
		{
			add.SetEnabled(true);
			add.m_OnClicked.Insert(OnAddClick);
		} else
		{
			Print("Add button could not be made!", LogLevel.ERROR);
		}*/
		
		// Add my buttons
		m_Mute = SCR_InputButtonComponent.GetInputButtonComponent("Mute", footer);
		if (m_Mute)
		{
			m_Mute.SetEnabled(true);
			m_Mute.m_OnActivated.Insert(OnAddClick);
			m_Mute.SetLabel(" Add ");
		}
		
		m_Block = SCR_InputButtonComponent.GetInputButtonComponent("Block", footer);
		if (m_Block)
		{
			m_Block.SetEnabled(true);
			m_Block.m_OnActivated.Insert(OnRemoveClick);
			m_Block.SetLabel("Remove");
		}
		
		//SetupComboBoxDropdown(m_aFactions);

		
		// Create table
		if (!m_wTable || m_sScoreboardRow == string.Empty)
			return;

		//Get editor Delegate manager to check if has editor rights
		SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));

		//use RPC, on the controller call RPC to server to get the data, send it back to teh owner's controller then call CreateEntry on here
		BotsWorldController controller = GetBotsController();
		controller.RequestGetNumberForMenu();
		controller.RequestGetPlayerManagerForMenu();
		
		
		/*
		array<int> ids = {};
		array<ref CLINTON_Virtual_Player> players;
		
		CLINTON_VirtualPlayerManager manny = GetBotsManager();
		if (manny){
			players = manny.GetPlayers();
		} else {
			players = {};}
		
		for (int i = 0; i < players.Count(); i++) // starts from 1 to not have 0-based index miscalculation
		{
				CreateEntry(i, editorDelegateManager);
		}
		
		InitSorting(); */
		
		m_SuperMenuComponent.GetTabView().GetOnChanged().Insert(OnTabChanged);
		
		// Create new tabs
		SCR_Faction scrFaction;
		foreach (Faction faction : m_aFactions)
		{
			if (!faction)
				continue;

			scrFaction = SCR_Faction.Cast(faction);
			if (scrFaction && !scrFaction.IsPlayable())
				continue; //--- ToDo: Refresh dynamically when a new faction is added/removed
			
			string name = faction.GetFactionName();
			m_SuperMenuComponent.GetTabView().AddTab(ResourceName.Empty,name);
			
			AddFactionPlayerCounter(faction);
		}

		//handle groups tab
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		Faction playerFaction;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			playerFaction = factionManager.GetLocalPlayerFaction();
		
		
		if (!groupsManager || !playerFaction)
			m_SuperMenuComponent.GetTabView().SetTabVisible(EPlayerListTab.GROUPS, false);
		//UpdateFrequencies();
		
		//---------------------------------------------------------------------------
		// Get Combo box widget and populate with the factions list
		// add invokers for choosing an option and initialise the memory
		// TODO: Add a Faction key as data
		// scr.AddItem( "Each Faction", false );
		XComboBoxWidget factionBox = XComboBoxWidget.Cast(GetRootWidget().FindAnyWidget("CLINTON_ComboBox"));
		factionBox.AddItem("Each Faction");
		foreach (Faction faction : m_aFactions)
		{
			if (!faction)
				continue;

			scrFaction = SCR_Faction.Cast(faction);
			if (scrFaction && !scrFaction.IsPlayable())
				continue; //--- ToDo: Refresh dynamically when a new faction is added/removed
			
			string name = faction.GetFactionName();
			factionBox.AddItem( name );  // returns int as index
		}
		Widget w;
		SCR_ComboBoxComponent scr;
		
		w = GetRootWidget().FindAnyWidget("CLINTON_ComboRoot");
		// Idk why the ComboBoxComponent is the first Handler
		// if this breaks find a way to get the specific Handler irespective of pos
		scr = SCR_ComboBoxComponent.Cast(w.GetHandler(0));  // **** This seems important
		if(!scr) Print("Wrong Handler / Controller", LogLevel.ERROR);
		
		scr.AddItem( "Join Groups based on distance", false );  // **** Let's turn this into localised strings before publishing
		scr.AddItem( "Fill Player groups first", false );
		scr.AddItem( "Cohesive Groups", true );
		
		// scr.GetCurrentIndex(); wot?
		
		w = GetRootWidget().FindAnyWidget("CLINTON_ComboLoadoutMode");
		scr = SCR_ComboBoxComponent.Cast(w.GetHandler(0));  // **** This seems important
		if(!scr) Print("Wrong Handler / Controller", LogLevel.ERROR);
		
		// If I add individual Loadouts then the problem occurs that loadouts are separate for each faction
		scr.AddItem( "Even Loadouts", false );
		scr.AddItem( "Random Loadouts Each Spawn", false );
		scr.AddItem( "Random Loadouts", true );
		
		// sihdoahdohowahoho
		// scr.GetCurrentIndex();
		
		s_OnPlayerListMenu.Invoke(true);
	}
	
	void RecieveNumber(int x)
	{
		Print("Hello and welcome %1",x);
		// CreateEntry(data);
	}
	
	void RecievePlayerList_Open(array<ref CLINTON_Virtual_Player> botsListForMenu)
	{
		Print("Number of members %1", botsListForMenu.Count());
		
		if (m_aEntries.Count() > 0)
		{
			Print("Still data? 	| 		MyScriptedMapUI.c", LogLevel.ERROR);
		}
		
		//m_aEntries.Clear();
		
		for (int i = 0; i < botsListForMenu.Count(); i++)
		{
			ref CLINTON_Virtual_Player p = botsListForMenu.Get(i);
			
			CreateEntry(p, i);
			
			
		}
		
		InitSorting();
	}
	
	void RecievePlayerList_Add(array<ref CLINTON_Virtual_Player> botsListForMenu)
	{
		Print("Number of members %1", botsListForMenu.Count());
		
		int start = m_aEntries.Count();
		
		if (start >= botsListForMenu.Count())
			Print("Arithmatic Epic Fail. 	| 		MyScriptedMapUI.c", LogLevel.ERROR);
		
		CLINTON_Virtual_Player p;
		for (int i = start; i < botsListForMenu.Count(); i++)
		{
			p = botsListForMenu.Get(i);
			CreateEntry(p, i);  // **** ID IS WRONG ****
		}
		
		InitSorting();
	}
	
	void RecievePlayerList_Subtract(array<ref CLINTON_Virtual_Player> botsListForMenu)  // Maybe hash maps are bettre
	{
		Print("Number of members %1", botsListForMenu.Count());
		int i = botsListForMenu.Count() - 1;
		int j = m_aEntries.Count() - 1 ;
		string characterName;
	
		while(i > -1 && j > -1) // Maybe the botsListForMenu.Count() condition is being broken by Entries being deleted
		{
			if( j < i )
			{
				Print("Subtraction Error. 	| 	MyScriptedMapUI.c", LogLevel.ERROR);
			}
			CLINTON_Virtual_Player p = botsListForMenu.Get(i);
			SCR_BotPlayerListEntry bp = m_aEntries.Get(j);
			characterName = string.Format("%1", p.GetPlayerName());
			if (characterName != bp.m_wName.GetText())  // Current comparison function is just compare the names
			{
				Print("name: "+ p.GetPlayerName() +"not equal to"+ bp.m_wName.GetText(), LogLevel.ERROR);  // **** WHen comparing the names, LHS is the fancy format and RHS is my string concatination slop
				RemoveEntry(m_aEntries.Get(j));
				m_mBotNames.RemoveOrdered(j);
			} else {
				i = i - 1;
			}
			j = j - 1;
		}
		if( i > 0 )
		{
			Print("Not enough m_aEntries. 	| 	MyScriptedMapUI.c", LogLevel.ERROR);
		}
		// Remove from the end of the list
		while ( j > -1)
		{
			RemoveEntry(m_aEntries.Get(j));
			m_mBotNames.RemoveOrdered(j);
			j = j - 1
		}
		// Hopefully it's worked
		InitSorting();
	}
	
	void RecievePlayerUpdate(int botId, string newName)  // Maybe do an enum and have different types of updates
	{
		UpdateEntry(botId, newName);
	}
	
	void InsertName(int i, CLINTON_Virtual_Player p, EditBoxWidget w)
	{
			if( i >= m_mBotNames.Count())
			{
				m_mBotNames.Insert(p.GetPlayerName());
			} else {
				m_mBotNames.Set(i, p.GetPlayerName());  // Still need this?
			}
			string latest_name = m_mBotNames.Get(i);
			if(latest_name)
			{
				//delayed_name_set(entry, id);
				w.SetText(latest_name);
			}
			else
			{
				Print("Names are not working! 	| 	MyScriptedMapUI.c", LogLevel.DEBUG);
			}
	}
	
	void SetLoadoutImage(int i, CLINTON_Virtual_Player p, ImageWidget img)
	{
		SCR_BotPlayerListEntry entry = m_aEntries.Get(i);
		
		ref ImageWidget w_img = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("LoadoutIcon"));// ehfoiehjoij
		w_img.SetVisible(true);
		
		SCR_BasePlayerLoadout playerLoadout;
		int loadoutIndex = p.GetLoadoutIndex();
		playerLoadout = SCR_BasePlayerLoadout.Cast( CLINTON_VirtualPlayerManager.GetLoadouts().Get(p.GetFactionKey()).Get(loadoutIndex));
		
		Resource res = Resource.Load(playerLoadout.GetLoadoutResource());
		IEntityComponentSource source = SCR_BaseContainerTools.FindComponentSource(res, "SCR_EditableCharacterComponent");
		if (!source)
			return;
		BaseContainer container = source.GetObject("m_UIInfo");
		SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		info.SetIconTo(w_img);
		//info.SetIconTo(entry.m_wLoadoutIcon);
		
		
		/*
		SCR_BasePlayerLoadout playerLoadout;
		int i = p.GetLoadoutIndex();
		playerLoadout = SCR_BasePlayerLoadout.Cast( CLINTON_VirtualPlayerManager.GetLoadouts().Get(i));
		
		ResourceName res = playerLoadout.GetLoadoutImageResource()
		
		image.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, icon);
		if (glow)
			glow.LoadImageFromSet(0, UIConstants.ICONS_GLOW_IMAGE_SET, icon);
		
		if (!setVisible)
			return;
		
		image.SetVisible(true);
		if (glow)
			glow.SetVisible(true);*/
	}

	void WaitForFirstSpawn(int botID, CLINTON_Virtual_Player p, EditBoxWidget w, ImageWidget img)
	{
		string latest_name = p.GetPlayerName();
		if (!latest_name || latest_name == "")
		{
			GetGame().GetCallqueue().CallLater(WaitForFirstSpawn, 1024, false, botID, p, w, img);
			return;
		}
		InsertName(botID, p, w);		
			
		// Get the loadout image resource name thing
		SetLoadoutImage(botID, p, img);
	}
	
	
	//------------------------------------------------------------------------------------------------
	void m_OnFocusLost(Widget w)
	{
		foreach (SCR_BotPlayerListEntry entry : m_aEntries)
		{
			if (!entry)
				continue;

			Widget row = entry.m_wName;
			if (row != w)
				continue;

			m_SelectedEntry = entry;
			break;
		}
		if (m_SelectedEntry)
		{
			string selectedEntryText = m_SelectedEntry.m_wName.GetText();
			if (selectedEntryText == "")
				m_SelectedEntry.m_wName.SetText(m_mBotNames.Get(m_SelectedEntry.m_iID));
			if (selectedEntryText != m_mBotNames.Get(m_SelectedEntry.m_iID))
			{
				//use RPC, on the controller call RPC to server to get the data, send it back to teh owner's controller then call CreateEntry on here
				BotsWorldController controller = GetBotsController();
				
				if (!controller)
				{
					Print("Can't get Controller!	|	MyScriptedMapUI.c", LogLevel.ERROR);
					return;
				}
				controller.RequestNameChange(m_SelectedEntry.m_iID, m_SelectedEntry.m_wName.GetText());
				// controller.RequestGetPlayerManagerForMenu(); Instead the server will call an update function on all/some clients
			}
		}
	}
	
	// Parity check when doing replication
	
	// Consider 'updating' the player list by directly deleting or adding layouts
	// Avoids events like deleting one player in a list of one hundred
	//------------------------------------------------------------------------------------------------
	void CreateEntry(CLINTON_Virtual_Player p, int id, Faction faction = null)
	{
		//check for existing entry, return if it exists already
		foreach (SCR_BotPlayerListEntry entry : m_aEntries)
		{
			if (entry.m_iID == id)
			{
				entry.m_toBeDeleted = false;
				return;
			}
		}

		ImageWidget badgeTop, badgeMiddle, badgeBottom;

		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sScoreboardRow, m_wTable);
		if (!w)
			return;

		SCR_BotPlayerListEntry entry = new SCR_BotPlayerListEntry();
		entry.m_iID = id;
		entry.m_wRow = w;
		entry.m_toBeDeleted = false;

		//--- return the voting combo box
		entry.m_PlayerActionList = SCR_ComboBoxComponent.GetComboBoxComponent("VotingCombo", w); 
		if (true)  // m_VotingManager
		{
			entry.m_PlayerActionList.m_OnOpened.Insert(SetupPlayerActionList);
			entry.m_PlayerActionList.m_OnChanged.Insert(OnComboBoxConfirm); // Scat corn
			entry.m_PlayerActionList.SetEnabled(true);

			// entry.m_wVotingNotification = entry.m_wRow.FindAnyWidget("VotingNotification");
			//entry.m_wVotingNotification.SetVisible(IsVotedAbout(entry));
		}
		else
		{
			entry.m_PlayerActionList.SetVisible(false);
		}
		
		entry.m_LoadoutPreferenceList = SCR_ComboBoxComponent.GetComboBoxComponent("PreferenceCombo", w); 
		if (true)
		{
			entry.m_LoadoutPreferenceList.SetEnabled(true);
			entry.m_LoadoutPreferenceList.SetVisible(true);
		}

		entry.m_SpecLoadoutList = SCR_ComboBoxComponent.GetComboBoxComponent("SpecificCombo", w); 
		if (true)
		{
			entry.m_SpecLoadoutList.SetEnabled(true);
			entry.m_SpecLoadoutList.SetVisible(true);
		}

		SCR_ButtonBaseComponent handler = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		if (handler)
		{
			handler.m_OnFocus.Insert(OnEntryFocused);
			handler.m_OnFocusLost.Insert(OnEntryFocusLost);
		}

		if (m_aAllPlayersInfo)
		{
			foreach (int playerId, SCR_ScoreInfo info : m_aAllPlayersInfo)
			{
				if (!info || playerId != id)
					continue;

				entry.m_Info = info;
				break;
			}
		}

		// Find faction
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!faction)
		{
			//faction = factionManager.GetPlayerFaction(entry.m_iID);
			string factionK = p.GetFactionKey();
			faction = factionManager.GetFactionByKey(factionK);
		}
		entry.m_Faction = faction;

		Widget factionImage = w.FindAnyWidget("FactionImage");

		if (factionImage)
		{
			if (entry.m_Faction)
				factionImage.SetColor(entry.m_Faction.GetFactionColor());
			else
				factionImage.SetVisible(false);
		}

		entry.m_wFreq = TextWidget.Cast(w.FindAnyWidget("Freq"));
		// entry.m_wKills = TextWidget.Cast(w.FindAnyWidget("Kills"));
		// entry.m_wDeaths = TextWidget.Cast(w.FindAnyWidget("Deaths"));
		// entry.m_wScore = TextWidget.Cast(w.FindAnyWidget("Score"));
		if (entry.m_Info)
		{
			if (entry.m_wKills)
				entry.m_wKills.SetText(entry.m_Info.m_iKills.ToString());
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText(entry.m_Info.m_iDeaths.ToString());
			if (entry.m_wScore)
			{
				// Use modifiers from scoring system where applicable!!!
				int score;
				if (m_ScoringSystem)
					score = m_ScoringSystem.GetPlayerScore(id);

				entry.m_wScore.SetText(score.ToString());
			}
		}
		else
		{

			if (entry.m_wKills)
				entry.m_wKills.SetText("");
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText("");
			if (entry.m_wScore)
				entry.m_wScore.SetText("");
			// Unfortunately the parent that must be hidden is two parents above the text widgets
			/*
			if (entry.m_wKills)
				entry.m_wKills.GetParent().GetParent().SetVisible(false);
			if (entry.m_wDeaths)
				entry.m_wDeaths.GetParent().GetParent().SetVisible(false);
			if (entry.m_wScore)
				entry.m_wScore.GetParent().GetParent().SetVisible(false);
			*/
		}
		
		entry.m_wLoadoutIcon = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("LoadoutIcon"));
		entry.m_wPlatformIcon = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("PlatformIcon"));
		
		entry.m_wPlatformIcon.SetVisible(false);
		
		ImageWidget background = ImageWidget.Cast(w.FindAnyWidget("Background"));

		badgeTop = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeTop"));
		badgeMiddle = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeMiddle"));
		badgeBottom = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeBottom"));
		Color factionColor;

		if (badgeTop && badgeMiddle && badgeBottom && entry.m_Faction)
		{
			factionColor = entry.m_Faction.GetFactionColor();
			badgeTop.SetColor(factionColor);
			badgeMiddle.SetColor(factionColor);
			badgeBottom.SetColor(factionColor);
		}
		
		m_aEntries.Insert(entry);
		
		entry.m_wName = EditBoxWidget.Cast(w.FindAnyWidget("PlayerName"));
		string latest_name = p.GetPlayerName();
		
		//SCR_ButtonBaseComponent handler = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		//if (handler)
		//{
		//	handler.m_OnFocus.Insert(OnEntryFocused);
		//	handler.m_OnFocusLost.Insert(OnEntryFocusLost);
		//}
		
		SCR_ButtonBaseComponent nameTextHandler = SCR_ButtonBaseComponent.Cast(entry.m_wName.FindHandler(SCR_ButtonBaseComponent));
		if (nameTextHandler)
		{
			// nameTextHandler.m_OnFocus.Insert(OnEntryFocused);
			nameTextHandler.m_OnFocusLost.Insert(OnNameFocusLost);
		} else {
			/*
				I need to make my own handler class that extends from ScriptedWidgetEventHandler and overrides the OnFocusLost Function (or maybe another runs when you press enter in the text edit)
			 */
			//ScriptedWidgetEventHandler
			Print("Handler maker time! ****", LogLevel.ERROR);
			nameTextHandler = new SCR_ButtonBaseComponent();
			entry.m_wName.AddHandler(nameTextHandler);
			nameTextHandler.m_OnFocusLost.Insert(OnNameFocusLost);
			
		}
		
		if (!latest_name || latest_name == "")
		{
			GetGame().GetCallqueue().CallLater(WaitForFirstSpawn, 1024, false, id, p, entry.m_wName, entry.m_wLoadoutIcon);
			return;
		}
		
		InsertName(id, p, entry.m_wName);
		SetLoadoutImage(id, p, entry.m_wLoadoutIcon);
		
		//CLINTON_VirtualPlayerManager pm = CLINTON_VirtualPlayerManager.getInstance();
		
		if (entry.m_iID == m_PlayerController.GetPlayerId())
		{
			entry.m_wName.SetColor(m_PlayerNameSelfColor);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateEntry(CLINTON_Virtual_Player p, int id, Faction faction = null)
	{
		if (!m_aEntries.Get(id))
		{
			Print("Trying to update a missing row! 		| 		MyScriptedMapUI.c", LogLevel.ERROR);
			return;
		}
		
		ref SCR_BotPlayerListEntry entry = m_aEntries.Get(id);
		
		entry.m_toBeDeleted = false;

		ImageWidget badgeTop, badgeMiddle, badgeBottom;
		
		
		string botName = m_mBotNames.Get(id);
		if(botName == "")
		{
			GetGame().GetCallqueue().CallLater(CreateEntry, 512, false, p, id, faction);
			return;
		}

		// Widget w = GetGame().GetWorkspace().CreateWidgets(m_sScoreboardRow, m_wTable);
		ref Widget w = entry.m_wRow;
		if (!w)
			return;

		//SCR_BotPlayerListEntry entry = new SCR_BotPlayerListEntry();
		//entry.m_iID = id;
		//entry.m_wRow = w;
		//entry.m_toBeDeleted = false;

		//--- 'rid the voting

		//SCR_ButtonBaseComponent handler = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		//if (handler)
		//{
		//	handler.m_OnFocus.Insert(OnEntryFocused);
		//	handler.m_OnFocusLost.Insert(OnEntryFocusLost);
		//}

		if (m_aAllPlayersInfo)
		{
			foreach (int playerId, SCR_ScoreInfo info : m_aAllPlayersInfo)  // ?
			{
				if (!info || playerId != id)
					continue;

				entry.m_Info = info;
				break;
			}
		}

		// Find faction
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!faction)
		{
			//faction = factionManager.GetPlayerFaction(entry.m_iID);
			string factionK = p.GetFactionKey();
			faction = factionManager.GetFactionByKey(factionK);
		}
		entry.m_Faction = faction;

		Widget factionImage = w.FindAnyWidget("FactionImage");

		if (factionImage)
		{
			if (entry.m_Faction)
				factionImage.SetColor(entry.m_Faction.GetFactionColor());
			else
				factionImage.SetVisible(false);
		}

		//entry.m_wName = EditBoxWidget.Cast(w.FindAnyWidget("PlayerName"));
		string latest_name;
		
		if (entry.m_wName)
		{
			//CLINTON_VirtualPlayerManager pm = CLINTON_VirtualPlayerManager.getInstance();
			latest_name = m_mBotNames.Get(id);
			if(latest_name)
			{
				//delayed_name_set(entry, id);
				entry.m_wName.SetText(latest_name);
			}
			else
			{
				Print("Names are not working! 	| 	MyScriptedMapUI.c", LogLevel.DEBUG);
			}
			//if (entry.m_iID == m_PlayerController.GetPlayerId())
			//{
			//	entry.m_wName.SetColor(m_PlayerNameSelfColor);
			//}
		}
		else
		{
			Print("Missing name Widget! 	| 	MyScriptedMapUI.c", LogLevel.ERROR);
		}


		entry.m_wFreq = TextWidget.Cast(w.FindAnyWidget("Freq"));
		// entry.m_wKills = TextWidget.Cast(w.FindAnyWidget("Kills"));
		// entry.m_wDeaths = TextWidget.Cast(w.FindAnyWidget("Deaths"));
		// entry.m_wScore = TextWidget.Cast(w.FindAnyWidget("Score"));
		if (entry.m_Info)
		{
			if (entry.m_wKills)
				entry.m_wKills.SetText(entry.m_Info.m_iKills.ToString());
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText(entry.m_Info.m_iDeaths.ToString());
			if (entry.m_wScore)
			{
				// Use modifiers from scoring system where applicable!!!
				int score;
				if (m_ScoringSystem)
					score = m_ScoringSystem.GetPlayerScore(id);

				entry.m_wScore.SetText(score.ToString());
			}
		}
		else
		{

			if (entry.m_wKills)
				entry.m_wKills.SetText("");
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText("");
			if (entry.m_wScore)
				entry.m_wScore.SetText("");
			// Unfortunately the parent that must be hidden is two parents above the text widgets
			/*
			if (entry.m_wKills)
				entry.m_wKills.GetParent().GetParent().SetVisible(false);
			if (entry.m_wDeaths)
				entry.m_wDeaths.GetParent().GetParent().SetVisible(false);
			if (entry.m_wScore)
				entry.m_wScore.GetParent().GetParent().SetVisible(false);
			*/
		}
		ImageWidget background = ImageWidget.Cast(w.FindAnyWidget("Background")); /*
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup group = groupManager.GetPlayerGroup(m_PlayerController.GetPlayerId());

		if (group && group.HasRequesterID(id))
			background.SetColor(m_PlayerNameSelfColor);		// TODO: Idk I stole SCR_PlayerListMenu

		if (entry.m_wTaskIcon && GetTaskManager())
		{
			SCR_BaseTaskExecutor taskExecutor = SCR_BaseTaskExecutor.FindTaskExecutorByID(entry.m_iID);
			if (taskExecutor.GetAssignedTask())
			{
				entry.m_wTaskIcon.SetColor(entry.m_Faction.GetFactionColor());
			}
			else
			{
				entry.m_wTaskIcon.SetOpacity(0);
			}
		}

		Faction playerFaction;		
		Faction entryPlayerFaction;
		if (!faction)
		{
			// CLINTON_VirtualPlayerManager pm = CLINTON_VirtualPlayerManager.getInstance();
			playerFaction = factionManager.GetFactionByKey(p.GetFactionKey());
			entry.m_Faction = playerFaction;
		} Already did faction */

		badgeTop = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeTop"));
		badgeMiddle = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeMiddle"));
		badgeBottom = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeBottom"));
		Color factionColor;

		if (badgeTop && badgeMiddle && badgeBottom && entry.m_Faction)
		{
			factionColor = entry.m_Faction.GetFactionColor();
			badgeTop.SetColor(factionColor);
			badgeMiddle.SetColor(factionColor);
			badgeBottom.SetColor(factionColor);
		}
		
		// m_aEntries.Insert(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateEntry(int id, string newName, Faction faction = null)
	{
		//check for existing entry
		SCR_BotPlayerListEntry entry;
		foreach (SCR_BotPlayerListEntry i_entry : m_aEntries)
		{
			if (i_entry.m_iID == id)
			{
				entry = i_entry;
				break;
			}
		}
		if (!entry) return;

		ImageWidget badgeTop, badgeMiddle, badgeBottom;
		
		if(newName == "")
		{
			// GetGame().GetCallqueue().CallLater(UpdateEntry, 512, false, id, newName, faction);
			Print("No name being provided!		|		MyScriptedMapUI.c", LogLevel.ERROR);
			return;
		}

		//Widget w = GetGame().GetWorkspace().CreateWidgets(m_sScoreboardRow, m_wTable);
		Widget w = entry.m_wRow;
		if (!w)
		{
			Print("Missing widget!		|		MyScriptedMapUI.c", LogLevel.ERROR);
			return;
		}
		/*
		SCR_BotPlayerListEntry entry = new SCR_BotPlayerListEntry();
		entry.m_iID = id;
		entry.m_wRow = w; */

		//--- 'rid the voting
		
		/*
		SCR_ButtonBaseComponent handler = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		if (handler)
		{
			handler.m_OnFocus.Insert(OnEntryFocused);
			handler.m_OnFocusLost.Insert(OnEntryFocusLost);
		}

		if (m_aAllPlayersInfo)
		{
			foreach (int playerId, SCR_ScoreInfo info : m_aAllPlayersInfo) 	**** Maybe bring back for Character skill ****
			{
				if (!info || playerId != id)
					continue;

				entry.m_Info = info;
				break;
			}
		} */

		/*
		// Find faction
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!faction)
		{
			faction = factionManager.GetPlayerFaction(entry.m_iID);
		}
		entry.m_Faction = faction;

		Widget factionImage = w.FindAnyWidget("FactionImage");

		if (factionImage)
		{
			if (entry.m_Faction)
				factionImage.SetColor(entry.m_Faction.GetFactionColor());
			else
				factionImage.SetVisible(false);
		}*/
		
		m_mBotNames.Set(id, newName);
		entry.m_wName.SetText(newName);
		if (entry.m_iID == m_PlayerController.GetPlayerId())
		{
			entry.m_wName.SetColor(m_PlayerNameSelfColor);
		}

	/*	entry.m_wFreq = TextWidget.Cast(w.FindAnyWidget("Freq"));
		// entry.m_wKills = TextWidget.Cast(w.FindAnyWidget("Kills"));
		// entry.m_wDeaths = TextWidget.Cast(w.FindAnyWidget("Deaths"));
		// entry.m_wScore = TextWidget.Cast(w.FindAnyWidget("Score"));
		if (entry.m_Info)
		{
			if (entry.m_wKills)
				entry.m_wKills.SetText(entry.m_Info.m_iKills.ToString());
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText(entry.m_Info.m_iDeaths.ToString());
			if (entry.m_wScore)
			{
				// Use modifiers from scoring system where applicable!!!
				int score;
				if (m_ScoringSystem)
					score = m_ScoringSystem.GetPlayerScore(id);

				entry.m_wScore.SetText(score.ToString());
			}
		}
		else
		{

			if (entry.m_wKills)
				entry.m_wKills.SetText("");
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText("");
			if (entry.m_wScore)
				entry.m_wScore.SetText("");
			// Unfortunately the parent that must be hidden is two parents above the text widgets
			
			if (entry.m_wKills)
				entry.m_wKills.GetParent().GetParent().SetVisible(false);
			if (entry.m_wDeaths)
				entry.m_wDeaths.GetParent().GetParent().SetVisible(false);
			if (entry.m_wScore)
				entry.m_wScore.GetParent().GetParent().SetVisible(false);
			
		}
		ImageWidget background = ImageWidget.Cast(w.FindAnyWidget("Background")); 
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup group = groupManager.GetPlayerGroup(m_PlayerController.GetPlayerId());

		if (group && group.HasRequesterID(id))
			background.SetColor(m_PlayerNameSelfColor);		// TODO: Idk I stole SCR_PlayerListMenu

		if (entry.m_wTaskIcon && GetTaskManager())
		{
			SCR_BaseTaskExecutor taskExecutor = SCR_BaseTaskExecutor.FindTaskExecutorByID(entry.m_iID);
			if (taskExecutor.GetAssignedTask())
			{
				entry.m_wTaskIcon.SetColor(entry.m_Faction.GetFactionColor());
			}
			else
			{
				entry.m_wTaskIcon.SetOpacity(0);
			}
		}

		Faction playerFaction;		
		Faction entryPlayerFaction;
		if (!faction)
		{
			// CLINTON_VirtualPlayerManager pm = CLINTON_VirtualPlayerManager.getInstance();
			playerFaction = factionManager.GetFactionByKey(p.GetFactionKey());
			entry.m_Faction = playerFaction;
		}

		badgeTop = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeTop"));
		badgeMiddle = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeMiddle"));
		badgeBottom = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeBottom"));
		Color factionColor;

		if (badgeTop && badgeMiddle && badgeBottom && entry.m_Faction)
		{
			factionColor = entry.m_Faction.GetFactionColor();
			badgeTop.SetColor(factionColor);
			badgeMiddle.SetColor(factionColor);
			badgeBottom.SetColor(factionColor);
		}
		
		m_aEntries.Insert(entry); */
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupPlayerActionList(notnull SCR_ComboBoxComponent combo)
	{
		//if (!m_VotingManager || !m_VoterComponent)
		//	return;

		combo.ClearAll();
		
		combo.AddItem("Loadout Preference");
		combo.AddItem("Choose Loadout");
		combo.AddItem("Respawn");
		combo.AddItem("Delete",true);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void SetupLoadoutPreferenceList(notnull SCR_ComboBoxComponent combo)
	{
		//if (!m_VotingManager || !m_VoterComponent)
		//	return;

		combo.ClearAll();
		
		combo.AddItem("Even loadouts");
		combo.AddItem("Random Loadout each Spawn");
		combo.AddItem("Random Loadout Chosen Once",true);
	}
	
	// Written by me
	void DeleteEntry(notnull SCR_PlayerListEntry entry, int id)
	{
		if (entry.m_wRow)
			entry.m_wRow.RemoveFromHierarchy();
		
		m_aEntries.RemoveOrdered(id);  // Is this enough?
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnComboBoxConfirm(notnull SCR_ComboBoxComponent combo, int index)
	{
		PrintFormat("%1",index);
		// Widget w = GetRootWidget().FindAnyWidget("Table");
		int botId = GetVotingPlayerID(combo);
		Widget w = m_aEntries[botId].m_wRow;
		PrintFormat("Player listed %1", botId);
		SCR_ComboBoxComponent marlon_brando;
		switch (index)
		{
			// Loadout Preference
			case 0:
			{
				marlon_brando = SCR_ComboBoxComponent.GetComboBoxComponent("PreferenceCombo", w);
				Print("We are here");
				//combo.m_OnOpened.Insert(SetupLoadoutPreferenceList);
				
				marlon_brando.ClearAll();
				
				marlon_brando.AddItem("Even loadouts");
				marlon_brando.AddItem("Random Loadout each Spawn");
				marlon_brando.AddItem("Random Loadout Chosen Once",true);
				
				marlon_brando.m_OnChanged.Clear();
				marlon_brando.m_OnChanged.Insert(OnComboBoxLoadoutPreferenceConfirm);
				marlon_brando.SetEnabled(true);
				marlon_brando.SetVisible(true);
				
				marlon_brando.OpenList();
				break;
			}
			
			// Specific Loadout
			case 1:
			{
				marlon_brando = SCR_ComboBoxComponent.GetComboBoxComponent("SpecificCombo", w);
				string fKey = m_aEntries[botId].m_Faction.GetFactionKey();
				if (!fKey)
				{
					Print("Factions are playing-up!", LogLevel.ERROR);
					break;	
				}
				ref array<SCR_BasePlayerLoadout> loadouts_in_faction;  // idk this doesn't have to be seperate
				loadouts_in_faction = CLINTON_VirtualPlayerManager.GetLoadouts().Get(fKey);
				
				int index_of_last_loadout = loadouts_in_faction.Count() -1;
				int i = 0;
				marlon_brando.ClearAll();
				foreach( SCR_BasePlayerLoadout loadout : loadouts_in_faction)
				{
					if(i == index_of_last_loadout)
					{
						marlon_brando.AddItem(loadout.GetLoadoutName(), true);  // Is knowing the last entry helping anything?
					} else {
						marlon_brando.AddItem(loadout.GetLoadoutName());
					}
					i = i + 1;
				}
				
				marlon_brando.m_OnChanged.Clear();
				marlon_brando.m_OnChanged.Insert(OnComboBoxSpecificLoadoutConfirm);
				marlon_brando.SetEnabled(true);
				marlon_brando.SetVisible(true);
				
				marlon_brando.OpenList();
					
				break;
			}
			
			// Respawn
			case 2:
			{
				break;
			}
			
			// Delete
			case 3:
			{
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnComboBoxLoadoutPreferenceConfirm(notnull SCR_ComboBoxComponent combo, int index)
	{
		PrintFormat("Loadout Preference with index %1",index);
		Widget w = GetRootWidget().FindAnyWidget("Table");
		switch (index)
		{
			// Loadout Preference
			case 0:
			{
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnComboBoxSpecificLoadoutConfirm(notnull SCR_ComboBoxComponent combo, int index)
	{
		PrintFormat("Specific loadout with index %1",index);
		Widget w = GetRootWidget().FindAnyWidget("Table");
		switch (index)
		{
			// Loadout Preference
			case 0:
			{
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------ Do this but for the m_loadout etc
	protected int GetVotingPlayerID(SCR_ComboBoxComponent combo)
	{
		for (int i, count = m_aEntries.Count(); i < count; i++)
		{
			if (m_aEntries[i].m_PlayerActionList == combo)
				return m_aEntries[i].m_iID;
		}
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveEntry(notnull SCR_BotPlayerListEntry entry)
	{
		if (entry.m_wRow)
			entry.m_wRow.RemoveFromHierarchy();
		m_aEntries.RemoveItemOrdered(entry);
	}
	/*
	void UpdatePlayerList()
	{
		// With the Replication Update, delete the table and repopulate on the client's side instead of mutating the list (w/ changes)
		
		// Create table
		if (!m_wTable || m_sScoreboardRow == string.Empty)
			return;

		//Get editor Delegate manager to check if has editor rights
		SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));

		CLINTON_VirtualPlayerManager pm = GetBotsManager();
		array<ref CLINTON_Virtual_Player> players = pm.GetPlayers();
		
		m_aEntries.Clear();
		for (int i = 0; i < players.Count(); i++) // starts from 1 to not have 0-based index miscalculation
		{
				CreateEntry(i, editorDelegateManager);
		}
		
		InitSorting();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerList( int id, Faction faction)
	{
			SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
			CreateEntry(id, editorDelegateManager, faction);

			// Get current sort method and re-apply sorting
			if (!m_Header)
				return;

			OnHeaderChanged(m_Header);
	}*/
	
	//------------------------------------------------------------------------------------------------
	void OnEntryFocused(Widget w)
	{		
		if (!m_PlayerController)
			return;
		
		foreach (SCR_BotPlayerListEntry entry : m_aEntries)
		{
			if (!entry)
				continue;

			Widget row = entry.m_wRow;
			if (row != w)
				continue;

			m_SelectedEntry = entry;
			break;
		}
		
		if (m_SelectedEntry)
		{
			UpdateViewProfileButton(m_SelectedEntry.m_iID);  // isn't it the other way-around? Once you have entered a name, loosing focus will confirm the change. (maybe)
			 /* // If EditTextBox is changed
			if (m_SelectedEntry.m_wName.GetText() == "")
				m_SelectedEntry.m_wName.SetText(m_mBotNames.Get(m_SelectedEntry.m_iID));
			if (m_SelectedEntry.m_wName != m_mBotNames.Get(m_SelectedEntry.m_iID))
			{
				//use RPC, on the controller call RPC to server to get the data, send it back to teh owner's controller then call CreateEntry on here
				BotsWorldController controller = GetBotsController();
				controller.RequestNameChange(m_SelectedEntry.m_iID, m_SelectedEntry.m_wName.GetText());
				controller.RequestGetPlayerManagerForMenu();
			} */
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnEntryFocusLost(Widget w)
	{
		UpdateViewProfileButton(0, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnNameFocusLost(Widget w)
	{
		if (!m_PlayerController)
			return;
		
		foreach (SCR_BotPlayerListEntry entry : m_aEntries)
		{
			if (!entry)
				continue;

			Widget row = entry.m_wName;
			if (row != w)
				continue;

			m_SelectedEntry = entry;
			break;
		}
		if (m_SelectedEntry)
		{
			string selectedEntryText = m_SelectedEntry.m_wName.GetText();
			if (selectedEntryText == "")
				m_SelectedEntry.m_wName.SetText(m_mBotNames.Get(m_SelectedEntry.m_iID));
			if (selectedEntryText != m_mBotNames.Get(m_SelectedEntry.m_iID))
			{
				//use RPC, on the controller call RPC to server to get the data, send it back to teh owner's controller then call CreateEntry on here
				BotsWorldController controller = GetBotsController();
				
				if (!controller)
				{
					Print("Can't get Controller!	|	MyScriptedMapUI.c", LogLevel.ERROR);
					return;
				}
				controller.RequestNameChange(m_SelectedEntry.m_iID, m_SelectedEntry.m_wName.GetText());
				// controller.RequestGetPlayerManagerForMenu(); Instead the server will call an update function on all/some clients
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// IsLocalPlayer would be better naming
	protected bool IsLocalPlayer(int id)
	{
		if (id <= 0)
			return false;

		return SCR_PlayerController.GetLocalPlayerId() == id;
	}

	
	protected bool CanOpenPlayerActionList(SCR_PlayerListEntry entry)
	{
		if (!entry)
			return false;
		
		if (m_VotingManager)
		{
			//~ Check if can vote, if yes return true
			array<EVotingType> votingTypes = {};
			m_VotingManager.GetVotingsAboutPlayer(entry.m_iID, votingTypes, true, true);
			
			//~ Check if UI info can be found
			foreach(EVotingType votingType : votingTypes)
			{
				if (m_VotingManager.GetVotingInfo(votingType))
					return true;
			}
		}
		
		//~ Can invite the player so return true
		if (m_PlayerGroupController.CanInvitePlayer(entry.m_iID))
			return true;
		
		//~ Check if Player actions have group dropdown
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (groupManager)
		{
			//~ No group so no drop down
			SCR_AIGroup group = groupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
			if (group)
			{
				array<int> requesters = {};
				group.GetRequesterIDs(requesters);
		
				if (requesters.Contains(entry.m_iID))
					return true;
			}
		}
		
		//Check if reporting is available
		if (entry.m_iID != SCR_PlayerController.GetLocalPlayerId())
			return true;
		
		//~ None of the conditions met
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateViewProfileButton(int playerId, bool forceHidden = false)
	{
		if (!m_ViewProfile)
			return;

		m_ViewProfile.SetVisible(!forceHidden && GetGame().GetPlayerManager().IsUserProfileAvailable(playerId), false);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateBlockElements()
	{
		if (!m_SelectedEntry || !m_SocialComponent)
			return;
		
		//no self block
		if (m_SelectedEntry.m_iID == SCR_PlayerController.GetLocalPlayerId())
		{
			if (m_Block)
			{
				m_Block.SetVisible(false, false);
				m_Block.SetEnabled(false);
			}
			
			if (m_Unblock)
			{
				m_Unblock.SetVisible(false, false);
				m_Unblock.SetEnabled(false);
			}
			
			m_SelectedEntry.m_wBlockedIcon.SetOpacity(0);
			return;
		}
		
		bool isBlocked = m_SocialComponent.IsBlocked(m_SelectedEntry.m_iID);
		
		m_SelectedEntry.m_wBlockedIcon.SetOpacity(isBlocked);
		
		if (m_Block)
		{
			m_Block.SetVisible(!isBlocked, false);
			m_Block.SetEnabled(!isBlocked);
		}
		
		if (m_Unblock)
		{
			m_Unblock.SetVisible(isBlocked, false);
			m_Unblock.SetEnabled(isBlocked);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTabChanged(SCR_TabViewComponent comp, Widget w, int selectedTab)
	{
		if (selectedTab < 0)
			return;

		Faction faction = null;
		foreach (Faction playableFaction : m_aFactions)
		{
			if (comp.GetShownTabComponent().m_sTabButtonContent == playableFaction.GetFactionName())
				faction = playableFaction;
		}

		int lowestZOrder = int.MAX;
		foreach (SCR_BotPlayerListEntry entry : m_aEntries)
		{
			if (!entry.m_wRow)
				continue;

			//if the tab is the first one, it's the All tab for now
			if (comp.GetShownTab() == 0)
				entry.m_wRow.SetVisible(true);
			else if (faction == entry.m_Faction)
				entry.m_wRow.SetVisible(true);
			else
				entry.m_wRow.SetVisible(false);
		}

		if (m_Header)
			m_Header.SetCurrentSortElement(DEFAULT_SORT_INDEX, ESortOrder.ASCENDING, useDefaultSortOrder: true);
	}
	
	//------------------------------------------------------------------------------------------------
	/*void UpdateFrequencies()
	{
		return;
		SCR_GadgetManagerComponent gadgetManager;
		IEntity localPlayer = SCR_PlayerController.GetLocalMainEntity();
		Faction localFaction;
		set<int> localFrequencies = new set<int>();
		if (localPlayer)
		{
			gadgetManager = SCR_GadgetManagerComponent.Cast(localPlayer.FindComponent(SCR_GadgetManagerComponent));
			SCR_Global.GetFrequencies(gadgetManager, localFrequencies);

			FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(localPlayer.FindComponent(FactionAffiliationComponent));
			if (factionAffiliation)
				localFaction = factionAffiliation.GetAffiliatedFaction();
		}
		CLINTON_VirtualPlayerManager pm = GetBotsManager();
		if (!pm )
			return;
		foreach (SCR_BotPlayerListEntry entry : m_aEntries)
		{
			if (entry.m_Faction == localFaction)
			{
				IEntity playerEntity = IEntity.Cast(pm.GetPlayers()[entry.m_iID].GetCurrentCharacter());
				
				if (playerEntity)
				{
					//--- ToDo: Don't extract frequencies locally; do it on server and distribute values to all clients
					gadgetManager = SCR_GadgetManagerComponent.Cast(playerEntity.FindComponent(SCR_GadgetManagerComponent));
					set<int> frequencies = new set<int>();
					SCR_Global.GetFrequencies(gadgetManager, frequencies);
					if (!frequencies.IsEmpty())
					{
						entry.m_iSortFrequency = frequencies[0];
						entry.m_wFreq.SetText(SCR_FormatHelper.FormatFrequencies(frequencies, localFrequencies));
						continue;
					}
				}
			}
			entry.m_iSortFrequency = int.MAX;
			entry.m_wFreq.SetText("-");
		}
		GetGame().GetCallqueue().CallLater(UpdateFrequencies, 1000, false);
	}*/
	
	//------------------------------------------------------------------------------------------------
	void AddFactionPlayerCounter(Faction faction)
	{
		SCR_Faction scriptedFaction = SCR_Faction.Cast(faction);
		if (!scriptedFaction)
			return;
		
		Widget contentLayout = GetRootWidget().FindAnyWidget("FactionPlayerNumbersLayout");
		if (!contentLayout)
			return;
		
		Widget factionTile = GetGame().GetWorkspace().CreateWidgets(FACTION_COUNTER_LAYOUT, contentLayout);
		if (!factionTile)
			return;
		
		RichTextWidget playerCount = RichTextWidget.Cast(factionTile.FindAnyWidget("PlayerCount"));
		if (!playerCount)
			return;
		
		ImageWidget factionFlag = ImageWidget.Cast(factionTile.FindAnyWidget("FactionFlag"));
		if (!factionFlag)
			return;

		int x, y;
		factionFlag.LoadImageTexture(0, scriptedFaction.GetFactionFlag());	
		factionFlag.GetImageSize(0, x, y);
		factionFlag.SetSize(x, y);
		
		playerCount.SetText(scriptedFaction.GetPlayerCount().ToString());
	}
	
	//------------------------------------------------------------------------------------------------
	void OnAddClick(SCR_ButtonBaseComponent comp)
	{		
		EditBoxWidget entry_field = EditBoxWidget.Cast(	GetRootWidget().FindAnyWidget("CLINTON_number_field"));
		
		string input = entry_field.GetText();
		// Using parser. Maybe modify to fail instead of consume
		int converted = input.ToInt();
		if(!converted)
		{
			entry_field.SetColor(Color.FromRGBA(188,0,0,255));
			return;
		}
		entry_field.SetColor(Color.Black);
		
		// Get Combo Box selections
		XComboBoxWidget factionBox = XComboBoxWidget.Cast(GetRootWidget().FindAnyWidget("CLINTON_ComboBox"));
		int faction_setting = factionBox.GetCurrentItem() -1;
		
		Widget w;
		SCR_ComboBoxComponent scr;
		
		w = GetRootWidget().FindAnyWidget("CLINTON_ComboRoot");
		scr = SCR_ComboBoxComponent.Cast(w.GetHandler(0));
		if(!scr) Print("Wrong Handler / Controller", LogLevel.ERROR);
		int groups_setting = scr.GetCurrentIndex();
		
		w = GetRootWidget().FindAnyWidget("CLINTON_ComboLoadoutMode");
		scr = SCR_ComboBoxComponent.Cast(w.GetHandler(0));
		if(!scr) Print("Wrong Handler / Controller", LogLevel.ERROR);
		int loadouts_setting = scr.GetCurrentIndex();
		
		// TODO: Bot naming convension stuff
		// Bots are named when they first spawn a character.
		// The empty string signifies no name.
		bool useCustomNames = false;
		
		//RplComponent ent = RplComponent.Cast(Replication.FindItem(botsController));  // Can't get it registered on Clients
		//IEntity enty = IEntity.Cast(BaseRplComponent.Cast(ent).GetEntity());
		//RplBotsManagerController yourController = RplBotsManagerController.Cast(enty);
		
		//BotsWorldSystem system = BotsWorldSystem.Cast(
		//	GetGame().GetWorld().FindSystem(BotsWorldSystem));
		
		BotsWorldController controller = GetBotsController();
		
		controller.RequestGetNumber();
		int x = controller.numberCopy;
		controller.RequestSetNumber(x+1);
		
		
		if (!controller)
			Print("No Controller Found! | MyScriptedMapUI.c", LogLevel.ERROR);
		
		if(faction_setting < 0)
		{
			int j = 0;
			foreach(Faction faction : m_aFactions)
			{
				for(int i = 0; i < converted; i++)
				{
					controller.RequestAddBots( faction.GetFactionKey(), groups_setting, useCustomNames, loadouts_setting);  // They actually are being added, I just broke the spawning
					j = j + 1;
				}
			}
		} else {
			string factionKey = m_aFactions[faction_setting].GetFactionKey();
			for(int i = 0; i < converted; i++)
			{
				controller.RequestAddBots(factionKey, groups_setting, useCustomNames, loadouts_setting);
			}
		}
		// Wait some then display changes
		GetGame().GetCallqueue().CallLater(controller.RequestGetAddMenu, 512, false);
		//RequestGetPlayerManagerForMenu()
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRemoveClick(SCR_ButtonBaseComponent comp)
	{		
		EditBoxWidget entry_field = EditBoxWidget.Cast(	GetRootWidget().FindAnyWidget("CLINTON_number_field"));
		
		string input = entry_field.GetText();
		// Using parser. Maybe modify to fail instead of consume
		int converted = input.ToInt();
		if(!converted)
		{
			entry_field.SetColor(Color.FromRGBA(188,0,0,255));
			return;
		}
		entry_field.SetColor(Color.Black);
		
		// Get Combo Box selections
		XComboBoxWidget factionBox = XComboBoxWidget.Cast(GetRootWidget().FindAnyWidget("CLINTON_ComboBox"));
		int faction_setting = factionBox.GetCurrentItem() -1;
		
		// ****
		BotsWorldController controller = GetBotsController();
		
		controller.RequestGetNumber();
		int x = controller.numberCopy;
		controller.RequestSetNumber(x+1);
		
		if (!controller)
			Print("No Controller Found! | MyScriptedMapUI.c", LogLevel.ERROR);
		
		if(faction_setting < 0)
		{
			int j = 0;
			foreach(Faction faction : m_aFactions)
			{
				for(int i = 0; i < converted; i++)
				{
					controller.RequestRemoveBots( faction.GetFactionKey() );  // They actually are being added, I just broke the spawning
					j = j + 1;
				}
			}
		} else {
			string factionKey = m_aFactions[faction_setting].GetFactionKey();
			for(int i = 0; i < converted; i++)
			{
				controller.RequestRemoveBots(factionKey);
			}
		}
		// Wait some then display changes
		GetGame().GetCallqueue().CallLater(controller.RequestGetRemoveMenu, 512, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBack()
	{
		//if (m_bFiltering)
		//	OnFilter();
		//else
			Close();
	}
	/*
	//------------------------------------------------------------------------------------------------
	void SetupComboBoxDropdown(array<Faction> m_aFactions)
	{
		FrameWidget fw = FrameWidget.Cast(GetRootWidget().FindAnyWidget("Attribute_Dropdown0"));
		SCR_DropdownEditorAttributeUIComponent dropDownComp = fw.FindHandler(SCR_DropdownEditorAttributeUIComponent);
		// See SCR_DropdownEditorAttributeUIComponent.GetEntries()
	}*/
	
	protected ref array<ref string> myInfoList = {};
    protected int currentSelectedItem = 0;
    
    // -----------------------------------------------------------------------------------------------------
    array<ref string> GetInfo()
    {                
        return myInfoList;
    }
    
    // -----------------------------------------------------------------------------------------------------
    int GetCurrentSelectedItem()
    {
        return currentSelectedItem;
    }
    
    // -----------------------------------------------------------------------------------------------------
    void SetCurrentSelectedItem(int index)
    {
        currentSelectedItem = index;
        Print(currentSelectedItem);
    }    
    
    // -----------------------------------------------------------------------------------------------------
    void setupFactionPicker()
    {
        // This will initialize myInfoList with 10 strings
        myInfoList.Clear();
        int count = 10;
        for(int i = 0; i < count; i++)
        {                
            myInfoList.Insert("Info_"+i);
        }    
        
        // The selected item will be the second.
        currentSelectedItem = 1    
		// -ripped from discord
    }  
}